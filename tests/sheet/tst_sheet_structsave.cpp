// Tests of the file as rows, columns, and cells are inserted and deleted: the sheet's cells are renumbered, and every part of the file that names cells follows them (merged ranges,
// conditional formats, validations, hyperlinks, page breaks, filters, names, charts, drawings, notes). The workbooks are built in memory.
#include "richbook.h"
#include "sheethelpers.h"

#include "sheet/structsave.h"
#include "sheet/xlsxsave.h"

namespace {

struct Saved {
    QMap<QString, QByteArray> parts;
    QString error;
    SaveReport report;
};

// The file with the edits made.
Saved saveWith(const QByteArray &xlsx, const std::vector<StructEdit> &edits, const SaveChanges *more = nullptr)
{
    Saved out;
    Package package;
    QString why;
    if (!package.readLazy(xlsx, &why)) {
        out.error = why;
        return out;
    }
    Workbook book;
    if (!book.open(package, &why)) {
        out.error = why;
        return out;
    }
    SaveChanges changes;
    if (more) changes = *more;
    changes.edits = edits;
    QByteArray bytes;
    if (!saveXlsx(book, changes, &bytes, &out.error, &out.report)) return out;
    jp::readZip(bytes, out.parts);
    return out;
}

QString text(const Saved &s, const QString &part) { return QString::fromUtf8(s.parts.value(part)); }

} // namespace

class SheetStructSaveTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A row inserted at row 3: the cells below move down, a shared formula is written out, the array's range follows, and what names rows follows.
    void aRowInsertedMovesTheCellsAndWhatNamesThem()
    {
        const Saved s = saveWith(richBook(), {StructEdit::insertRows(0, 2, 1)});
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.contains(QStringLiteral("<row r=\"1\"")));
        QVERIFY(!sheet.contains(QStringLiteral("<row r=\"3\"")));
        QVERIFY2(sheet.contains(QStringLiteral("<c r=\"A4\"><v>3</v></c>")), qPrintable(sheet));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"A9\" t=\"inlineStr\">")));
        // the shared formula is written out (the formulas that read the cells that moved are written again by the workbook, as cells that changed)
        QVERIFY2(sheet.contains(QStringLiteral("<c r=\"B1\"><f>A1*2</f>")), qPrintable(sheet));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"B2\"><f>A2*2</f>")));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"B4\"><f>A3*2</f>")));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"B6\"><f>A5*2</f>")));
        QVERIFY(!sheet.contains(QStringLiteral("shared")));
        QVERIFY(sheet.contains(QStringLiteral("<f t=\"array\" ref=\"C4:C5\">A3:A4*2</f>")));
        QVERIFY(sheet.contains(QStringLiteral("<dimension ref=\"A1:F9\"/>")));
        QVERIFY(sheet.contains(QStringLiteral("<mergeCell ref=\"A8:B8\"/>")));
        QVERIFY(sheet.contains(QStringLiteral("<mergeCell ref=\"E1:F2\"/>")));
        QVERIFY2(sheet.contains(QStringLiteral("<autoFilter ref=\"A1:D6\">")), qPrintable(sheet));
        QVERIFY(sheet.contains(QStringLiteral("sqref=\"A1:A6\"")));
        QVERIFY(sheet.contains(QStringLiteral("sqref=\"C7\"")));
        QVERIFY2(sheet.contains(QStringLiteral("<formula1>$A$1:$A$6</formula1>")), qPrintable(sheet));
        QVERIFY(sheet.contains(QStringLiteral("<hyperlink ref=\"A9\"")));
        QVERIFY(sheet.contains(QStringLiteral("location=\"Sheet2!A1\"")));
        QVERIFY(sheet.contains(QStringLiteral("<brk id=\"5\"")));
        QVERIFY(sheet.contains(QStringLiteral("A1&gt;$D$1")));   // (the rule reads cells above the row)
        // the cells of the other sheet that refer to this one
        const QString sheet2 = text(s, QStringLiteral("xl/worksheets/sheet2.xml"));
        QVERIFY2(sheet2.contains(QStringLiteral("<formula1>Sheet1!$A$1:$A$6</formula1>")), qPrintable(sheet2));
        // names, chart, drawing, notes
        QVERIFY(text(s, QStringLiteral("xl/workbook.xml")).contains(QStringLiteral(">Sheet1!$A$1:$A$6</definedName>")));
        QVERIFY(text(s, QStringLiteral("xl/workbook.xml")).contains(QStringLiteral(">Sheet1!$A$1:$D$9</definedName>")));
        QVERIFY(text(s, QStringLiteral("xl/charts/chart1.xml")).contains(QStringLiteral("<c:f>Sheet1!$A$1:$A$6</c:f>")));
        const QString drawing = text(s, QStringLiteral("xl/drawings/drawing1.xml"));
        QVERIFY2(drawing.contains(QStringLiteral("<xdr:row>3</xdr:row>")) && drawing.contains(QStringLiteral("<xdr:row>7</xdr:row>")), qPrintable(drawing));
        QVERIFY(drawing.contains(QStringLiteral("<xdr:col>1</xdr:col>")));
        const QString comments = text(s, QStringLiteral("xl/comments1.xml"));
        QVERIFY2(comments.contains(QStringLiteral("ref=\"A4\"")) && comments.contains(QStringLiteral("ref=\"A5\"")) && !comments.contains(QStringLiteral("ref=\"A3\"")), qPrintable(comments));
        const QString vml = text(s, QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QVERIFY2(vml.contains(QStringLiteral("<x:Row>3</x:Row>")) && vml.contains(QStringLiteral("<x:Row>4</x:Row>")), qPrintable(vml));
        QVERIFY(vml.contains(QStringLiteral("<x:Anchor>1, 15, 3, 2, 3, 15, 5, 16</x:Anchor>")));
        QVERIFY(s.report.calcChainDropped || true);
    }

    // The formulas of a rule (a conditional format, a validation) are written for the first cell of its range, in the file too: the first row of the range deleted, or rows inserted above
    // it, and what it reads follows its first cell, not the cells it names.
    void aRulesRelativeFormulasAreWrittenForTheirNewFirstCell()
    {
        QString rows;
        for (int r = 1; r <= 10; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        const QString after = QStringLiteral("<conditionalFormatting sqref=\"B2:B10\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A2&gt;5</formula></cfRule></conditionalFormatting>"
                                             "<dataValidations count=\"1\"><dataValidation type=\"custom\" allowBlank=\"1\" showErrorMessage=\"1\" sqref=\"B2:B10\"><formula1>ISNUMBER($A2)</formula1></dataValidation></dataValidations>");
        const QByteArray book = buildXlsx({worksheet(rows, after)});
        {
            const Saved s = saveWith(book, {StructEdit::deleteRows(0, 1, 1)});
            QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
            const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
            QVERIFY2(sheet.contains(QStringLiteral("sqref=\"B2:B9\"><cfRule")), qPrintable(sheet));
            QVERIFY2(sheet.contains(QStringLiteral("<formula>$A2&gt;5</formula>")), qPrintable(sheet));
            QVERIFY2(sheet.contains(QStringLiteral("<formula1>ISNUMBER($A2)</formula1>")), qPrintable(sheet));
            QVERIFY(!sheet.contains(QStringLiteral("#REF!")));
        }
        {
            const Saved s = saveWith(book, {StructEdit::insertRows(0, 1, 2)});
            const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
            QVERIFY2(sheet.contains(QStringLiteral("sqref=\"B4:B12\"><cfRule")), qPrintable(sheet));
            QVERIFY2(sheet.contains(QStringLiteral("<formula>$A4&gt;5</formula>")), qPrintable(sheet));
            QVERIFY2(sheet.contains(QStringLiteral("<formula1>ISNUMBER($A4)</formula1>")), qPrintable(sheet));
        }
        {
            // two edits in turn: the first row deleted twice
            const Saved s = saveWith(book, {StructEdit::deleteRows(0, 1, 1), StructEdit::deleteRows(0, 1, 1)});
            const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
            QVERIFY2(sheet.contains(QStringLiteral("sqref=\"B2:B8\"><cfRule")), qPrintable(sheet));
            QVERIFY2(sheet.contains(QStringLiteral("<formula1>ISNUMBER($A2)</formula1>")), qPrintable(sheet));
        }
        {
            // a row the formula reads, above the range: it follows the cell
            const Saved s = saveWith(book, {StructEdit::deleteRows(0, 0, 1)});
            const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
            QVERIFY2(sheet.contains(QStringLiteral("sqref=\"B1:B9\"><cfRule")), qPrintable(sheet));
            QVERIFY2(sheet.contains(QStringLiteral("<formula1>ISNUMBER($A1)</formula1>")), qPrintable(sheet));
        }
    }

    // Threaded comments (the part a thread's cell is named in), the links of form controls (a check box that sets a cell, a list that reads a range), and the anchors of drawings follow the
    // rows; an anchor that does not move with cells (editAs "absolute") stays where it is, and one that moves and does not size ("oneCell") keeps its size.
    void threadedCommentsControlsAndAnchorsFollowTheRows()
    {
        const QByteArray threads = "<?xml version=\"1.0\"?><ThreadedComments xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\">"
                                   "<threadedComment ref=\"A1\" dT=\"2024-01-01T00:00:00.00\" personId=\"{P}\" id=\"{1}\"><text>first</text></threadedComment>"
                                   "<threadedComment ref=\"A3\" dT=\"2024-01-01T00:00:00.00\" personId=\"{P}\" id=\"{2}\"><text>on three</text></threadedComment>"
                                   "<threadedComment ref=\"A3\" dT=\"2024-01-01T00:00:00.00\" personId=\"{P}\" id=\"{3}\" parentId=\"{2}\"><text>reply</text></threadedComment></ThreadedComments>";
        const QByteArray control = "<?xml version=\"1.0\"?><formControlPr xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" objectType=\"Drop\" fmlaLink=\"$A$5\" fmlaRange=\"Sheet1!$A$1:$A$5\"/>";
        const QByteArray drawing = "<?xml version=\"1.0\"?><xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\">"
                                   "<xdr:twoCellAnchor><xdr:from><xdr:col>1</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>4</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>6</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to></xdr:twoCellAnchor>"
                                   "<xdr:twoCellAnchor editAs=\"oneCell\"><xdr:from><xdr:col>1</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>4</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>6</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to></xdr:twoCellAnchor>"
                                   "<xdr:twoCellAnchor editAs=\"absolute\"><xdr:from><xdr:col>1</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>4</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>6</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to></xdr:twoCellAnchor></xdr:wsDr>";
        QVector<ExtraPart> parts;
        parts.append({QStringLiteral("xl/threadedComments/threadedComment1.xml"), threads, QStringLiteral("application/vnd.ms-excel.threadedcomments+xml"), 0, QStringLiteral("rIdT"), QStringLiteral("threadedComment"), QStringLiteral("../threadedComments/threadedComment1.xml")});
        parts.append({QStringLiteral("xl/ctrlProps/ctrlProp1.xml"), control, QStringLiteral("application/vnd.ms-excel.controlproperties+xml"), 0, QStringLiteral("rIdK"), QStringLiteral("ctrlProp"), QStringLiteral("../ctrlProps/ctrlProp1.xml")});
        parts.append({QStringLiteral("xl/drawings/drawing2.xml"), drawing, QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), 1, QStringLiteral("rIdD2"), QStringLiteral("drawing"), QStringLiteral("../drawings/drawing2.xml")});
        const QByteArray book = withParts(richBook(), parts);
        // (the drawing of the second sheet is made the sheet's by its own element)
        QMap<QString, QByteArray> zipParts;
        jp::readZip(book, zipParts);
        zipParts[QStringLiteral("xl/worksheets/sheet2.xml")].replace("</worksheet>", "<drawing r:id=\"rIdD2\"/></worksheet>");
        jp::ZipWriter z;
        for (auto it = zipParts.begin(); it != zipParts.end(); ++it) z.add(it.key(), it.value(), true);
        const QByteArray withDrawing = z.finish();
        {
            const Saved s = saveWith(withDrawing, {StructEdit::insertRows(0, 0, 2)});   // two rows above everything, on Sheet1
            QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
            const QString t = text(s, QStringLiteral("xl/threadedComments/threadedComment1.xml"));
            QVERIFY2(t.contains(QStringLiteral("ref=\"A3\"")) && t.contains(QStringLiteral("ref=\"A5\"")) && !t.contains(QStringLiteral("ref=\"A1\"")), qPrintable(t));
            const QString c = text(s, QStringLiteral("xl/ctrlProps/ctrlProp1.xml"));
            QVERIFY2(c.contains(QStringLiteral("fmlaLink=\"$A$7\"")) && c.contains(QStringLiteral("fmlaRange=\"Sheet1!$A$3:$A$7\"")), qPrintable(c));
        }
        {
            const Saved s = saveWith(withDrawing, {StructEdit::deleteRows(0, 2, 1)});   // A3's row: its threads go
            const QString t = text(s, QStringLiteral("xl/threadedComments/threadedComment1.xml"));
            QVERIFY2(!t.contains(QStringLiteral("on three")) && !t.contains(QStringLiteral("reply")) && t.contains(QStringLiteral("first")) && t.contains(QStringLiteral("ref=\"A1\"")), qPrintable(t));
            QVERIFY(t.contains(QStringLiteral("</ThreadedComments>")));
            const QString c = text(s, QStringLiteral("xl/ctrlProps/ctrlProp1.xml"));
            QVERIFY2(c.contains(QStringLiteral("fmlaLink=\"$A$4\"")) && c.contains(QStringLiteral("fmlaRange=\"Sheet1!$A$1:$A$4\"")), qPrintable(c));
        }
        {
            // the anchors of a drawing on Sheet2, for rows inserted on Sheet2 (the sheet the drawing is on): above them, and inside the span of them
            const Saved above = saveWith(withDrawing, {StructEdit::insertRows(1, 0, 2)});
            const QString d = text(above, QStringLiteral("xl/drawings/drawing2.xml"));
            const QStringList anchors = d.split(QStringLiteral("</xdr:twoCellAnchor>"));
            QVERIFY2(anchors.size() == 4, qPrintable(d));
            QVERIFY2(anchors[0].contains(QStringLiteral("<xdr:row>4</xdr:row>")) && anchors[0].contains(QStringLiteral("<xdr:row>8</xdr:row>")), qPrintable(anchors[0]));   // moves and sizes with cells
            QVERIFY2(anchors[1].contains(QStringLiteral("<xdr:row>4</xdr:row>")) && anchors[1].contains(QStringLiteral("<xdr:row>8</xdr:row>")), qPrintable(anchors[1]));   // moves, keeps its size
            QVERIFY2(anchors[2].contains(QStringLiteral("<xdr:row>2</xdr:row>")) && anchors[2].contains(QStringLiteral("<xdr:row>6</xdr:row>")), qPrintable(anchors[2]));   // does not move
            const Saved inside = saveWith(withDrawing, {StructEdit::insertRows(1, 4, 3)});
            const QStringList in = text(inside, QStringLiteral("xl/drawings/drawing2.xml")).split(QStringLiteral("</xdr:twoCellAnchor>"));
            QVERIFY2(in[0].contains(QStringLiteral("<xdr:row>2</xdr:row>")) && in[0].contains(QStringLiteral("<xdr:row>9</xdr:row>")), qPrintable(in[0]));   // longer
            QVERIFY2(in[1].contains(QStringLiteral("<xdr:row>2</xdr:row>")) && in[1].contains(QStringLiteral("<xdr:row>6</xdr:row>")), qPrintable(in[1]));   // the same size
            QVERIFY2(in[2].contains(QStringLiteral("<xdr:row>2</xdr:row>")) && in[2].contains(QStringLiteral("<xdr:row>6</xdr:row>")), qPrintable(in[2]));
        }
    }

    // A defined name with references that are not fixed (relative to the cell it is used in) keeps the offsets it has when rows or columns are inserted; the parts that are fixed with "$"
    // follow the cells.
    void relativeDefinedNamesKeepTheirOffsets()
    {
        QString rows;
        for (int r = 1; r <= 5; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        const QByteArray book = buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(),
                                          QStringLiteral("<definedName name=\"Rel\">Sheet1!A2</definedName><definedName name=\"Abs\">Sheet1!$A$2</definedName><definedName name=\"ColFixed\">Sheet1!$A2</definedName><definedName name=\"RowFixed\">Sheet1!A$2</definedName>"));
        {
            const QString w = text(saveWith(book, {StructEdit::insertRows(0, 0, 1)}), QStringLiteral("xl/workbook.xml"));
            QVERIFY2(w.contains(QStringLiteral(">Sheet1!A2</definedName>")), qPrintable(w));
            QVERIFY(w.contains(QStringLiteral(">Sheet1!$A$3</definedName>")));
            QVERIFY(w.contains(QStringLiteral(">Sheet1!$A2</definedName>")));
            QVERIFY(w.contains(QStringLiteral(">Sheet1!A$3</definedName>")));
        }
        {
            const QString w = text(saveWith(book, {StructEdit::insertColumns(0, 0, 1)}), QStringLiteral("xl/workbook.xml"));
            QVERIFY2(w.contains(QStringLiteral(">Sheet1!A2</definedName>")), qPrintable(w));
            QVERIFY(w.contains(QStringLiteral(">Sheet1!$B$2</definedName>")));
            QVERIFY(w.contains(QStringLiteral(">Sheet1!$B2</definedName>")));
            QVERIFY(w.contains(QStringLiteral(">Sheet1!A$2</definedName>")));
        }
    }

    // Rows deleted: their cells go, merges and ranges shrink or go, and a note on a deleted cell goes with it.
    void rowsDeletedTakeWhatIsInThemAway()
    {
        const Saved s = saveWith(richBook(), {StructEdit::deleteRows(0, 2, 2)});   // rows 3 and 4
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(!sheet.contains(QStringLiteral("r=\"A8\"")));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"A3\"><v>5</v></c>")));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"B3\"><f>A5*2</f>")));
        QVERIFY(!sheet.contains(QStringLiteral("array")));
        QVERIFY(sheet.contains(QStringLiteral("<dimension ref=\"A1:F6\"/>")));
        QVERIFY(sheet.contains(QStringLiteral("<mergeCell ref=\"A5:B5\"/>")));
        QVERIFY(sheet.contains(QStringLiteral("<autoFilter ref=\"A1:D3\">")));
        QVERIFY(sheet.contains(QStringLiteral("<formula1>$A$1:$A$3</formula1>")));
        QVERIFY(sheet.contains(QStringLiteral("<hyperlink ref=\"A6\"")));
        QVERIFY2(sheet.contains(QStringLiteral("<brk id=\"2\"")), qPrintable(sheet));
        const QString comments = text(s, QStringLiteral("xl/comments1.xml"));
        QVERIFY(!comments.contains(QStringLiteral("<comment ")));
        QVERIFY(!text(s, QStringLiteral("xl/drawings/vmlDrawing1.vml")).contains(QStringLiteral("<v:shape")));
        QVERIFY(text(s, QStringLiteral("xl/workbook.xml")).contains(QStringLiteral(">Sheet1!$A$1:$A$3</definedName>")));
        const QString drawing = text(s, QStringLiteral("xl/drawings/drawing1.xml"));
        QVERIFY2(drawing.contains(QStringLiteral("<xdr:row>2</xdr:row>")) && drawing.contains(QStringLiteral("<xdr:row>4</xdr:row>")), qPrintable(drawing));
    }

    // Columns inserted and deleted: cells are renumbered, the widths follow, the filter's conditions follow their columns.
    void columnsMoveTheirCellsWidthsAndFilters()
    {
        const Saved s = saveWith(richBook(), {StructEdit::insertColumns(0, 1, 2)});   // two columns before B
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"A1\"><v>1</v></c><c r=\"D1\"><f>A1*2</f>")));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"E1\" t=\"inlineStr\">")));
        QVERIFY(!sheet.contains(QStringLiteral("spans=")));
        QVERIFY2(sheet.contains(QStringLiteral("<col min=\"1\" max=\"1\" width=\"12\"")), qPrintable(sheet));
        QVERIFY(sheet.contains(QStringLiteral("<col min=\"5\" max=\"6\" width=\"20\"")));
        QVERIFY(sheet.contains(QStringLiteral("<dimension ref=\"A1:H8\"/>")));
        QVERIFY(sheet.contains(QStringLiteral("<mergeCell ref=\"G1:H2\"/>")));
        QVERIFY(sheet.contains(QStringLiteral("<mergeCell ref=\"A7:D7\"/>")));   // (the merge grows over the new columns)
        QVERIFY(sheet.contains(QStringLiteral("<autoFilter ref=\"A1:F5\">")));
        QVERIFY(sheet.contains(QStringLiteral("colId=\"4\"")));   // (the filter's column C is now E, the fifth of the six columns A to F)
        QVERIFY(sheet.contains(QStringLiteral("<f t=\"array\" ref=\"E3:E4\">")));
        // deleting
        const Saved d = saveWith(richBook(), {StructEdit::deleteColumns(0, 1, 2)});   // columns B and C
        sheet = text(d, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(!sheet.contains(QStringLiteral("array")));
        QVERIFY2(sheet.contains(QStringLiteral("<c r=\"B1\"><v>7</v></c>")), qPrintable(sheet));
        QVERIFY(sheet.contains(QStringLiteral("<col min=\"2\" max=\"2\" width=\"20\"")));
        QVERIFY(sheet.contains(QStringLiteral("<mergeCell ref=\"C1:D2\"/>")));
        QVERIFY(sheet.contains(QStringLiteral("<mergeCell ref=\"A7\"/>")) == false);   // (a merge left with one cell is no merge)
        QVERIFY(!sheet.contains(QStringLiteral("filterColumn")));                      // (the filter's column was deleted)
        QVERIFY(sheet.contains(QStringLiteral("<autoFilter ref=\"A1:B5\">")));
    }

    // Cells shifted: the cells in the block's columns move down, and are written by the workbook; those that were there are left out of the sheet.
    void cellsShiftedAreLeftOutForTheWorkbookToWrite()
    {
        CellRange block;
        block.valid = true;
        block.row1 = 1;
        block.row2 = 1;
        block.col1 = 0;
        block.col2 = 0;
        const Saved s = saveWith(richBook(), {StructEdit::insertCellsDown(0, block)});
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"A1\"><v>1</v></c>")));
        QVERIFY(!sheet.contains(QStringLiteral("r=\"A2\"")));   // (A2 and below in column A are the workbook's to write)
        QVERIFY(!sheet.contains(QStringLiteral("r=\"A5\"")));
        QVERIFY(sheet.contains(QStringLiteral("<c r=\"B2\">")));   // (B is not in the block's columns)
    }

    // The sheets and parts the edits do not touch are copied as they were, and a sheet the edits do not move cells on has only what names cells followed.
    void whatTheEditsDoNotTouchIsCopied()
    {
        const Saved s = saveWith(richBook(), {StructEdit::insertRows(0, 2, 1)});
        QVERIFY(s.report.partsCopied > 0);
        const Saved other = saveWith(richBook(), {StructEdit::insertRows(1, 0, 1)});   // rows on the second sheet
        const QString sheet1 = text(other, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(sheet1.contains(QStringLiteral("Sheet2!A1")), qPrintable(sheet1));
        QVERIFY(sheet1.contains(QStringLiteral("<c r=\"A8\" t=\"inlineStr\">")));   // (the cells did not move)
        QVERIFY(sheet1.contains(QStringLiteral("t=\"shared\"")));                    // (nor were the formulas written out)
        QVERIFY(text(other, QStringLiteral("xl/worksheets/sheet2.xml")).contains(QStringLiteral("<c r=\"A2\">")));
        // an edit that changes nothing in a sheet leaves its bytes as they were
        QMap<QString, QByteArray> original;
        jp::readZip(richBook(), original);
        const Saved unrelated = saveWith(richBook(), {StructEdit::insertRows(0, 20, 1)});   // below everything the second sheet and the first's rules name
        QCOMPARE(unrelated.parts.value(QStringLiteral("xl/worksheets/sheet2.xml")), original.value(QStringLiteral("xl/worksheets/sheet2.xml")));
        QCOMPARE(unrelated.parts.value(QStringLiteral("xl/charts/chart1.xml")), original.value(QStringLiteral("xl/charts/chart1.xml")));
        QCOMPARE(unrelated.parts.value(QStringLiteral("xl/workbook.xml")), original.value(QStringLiteral("xl/workbook.xml")));
    }

    // Cells that the workbook changed (an array's formula, whose reading moved) are written whole over the renumbered ones.
    void changedCellsAreWrittenOverTheRenumberedOnes()
    {
        SaveChanges more;
        SavedCell cell;
        cell.row = 3;   // C4, where the array begins after the insert
        cell.col = 2;
        cell.isFormula = true;
        cell.formula = QStringLiteral("A4:A5*2");
        cell.arrayRef = QStringLiteral("C4:C5");
        cell.value.kind = SavedValue::Kind::Number;
        cell.value.number = 6;
        more.sheets[0].cells.push_back(cell);
        const Saved s = saveWith(richBook(), {StructEdit::insertRows(0, 2, 1)}, &more);
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(sheet.contains(QStringLiteral("<c r=\"C4\"><f t=\"array\" ref=\"C4:C5\">A4:A5*2</f><v>6</v></c>")), qPrintable(sheet));
        QVERIFY(!sheet.contains(QStringLiteral("A3:A4*2")));
    }

    // What the file says about conditional formats, hyperlinks, and chart series after rows and columns are inserted and deleted above, inside, and below them, with every expected text written
    // out by hand (the yardstick of the roundtrip works the expected text out with the program's own adjustment of formulas, so a fault in it would not be seen there).
    void aConditionalFormatFollowsRowsAndColumnsAsWrittenByHand()
    {
        QString rows;
        for (int r = 1; r <= 12; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        const QString after = QStringLiteral("<conditionalFormatting sqref=\"B3:B6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;$D$8</formula></cfRule></conditionalFormatting>"
                                             "<conditionalFormatting sqref=\"C3:C6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A3&gt;$D$4</formula></cfRule></conditionalFormatting>");
        const QByteArray book = buildXlsx({worksheet(rows, after)});
        struct Case {
            const char *what;
            StructEdit edit;
            QStringList expected;   // each must be in the sheet
            QStringList absent;     // and none of these
        };
        const QVector<Case> cases = {
            {"two rows inserted above both", StructEdit::insertRows(0, 0, 2),
             {"sqref=\"B5:B8\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A5&gt;$D$10</formula>", "sqref=\"C5:C8\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A5&gt;$D$6</formula>"}, {"#REF!"}},
            {"two rows inserted inside them", StructEdit::insertRows(0, 4, 2),
             {"sqref=\"B3:B8\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;$D$10</formula>", "sqref=\"C3:C8\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A3&gt;$D$4</formula>"}, {"#REF!"}},
            {"two rows inserted below them", StructEdit::insertRows(0, 9, 2),
             {"sqref=\"B3:B6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;$D$8</formula>", "sqref=\"C3:C6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A3&gt;$D$4</formula>"}, {"#REF!"}},
            {"two rows deleted above both", StructEdit::deleteRows(0, 0, 2),
             {"sqref=\"B1:B4\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A1&gt;$D$6</formula>", "sqref=\"C1:C4\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A1&gt;$D$2</formula>"}, {"#REF!"}},
            {"a row deleted inside them: the cell the second reads is gone", StructEdit::deleteRows(0, 3, 1),
             {"sqref=\"B3:B5\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;$D$7</formula>", "sqref=\"C3:C5\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A3&gt;#REF!</formula>"}, {}},
            {"the first row of them deleted: the rule is written for the first row left", StructEdit::deleteRows(0, 2, 1),
             {"sqref=\"B3:B5\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;$D$7</formula>", "sqref=\"C3:C5\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A3&gt;$D$3</formula>"}, {"#REF!"}},
            {"all their rows deleted", StructEdit::deleteRows(0, 2, 4), {}, {"conditionalFormatting", "cfRule", "#REF!"}},
            {"a row deleted below them", StructEdit::deleteRows(0, 10, 1),
             {"sqref=\"B3:B6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;$D$8</formula>", "sqref=\"C3:C6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A3&gt;$D$4</formula>"}, {"#REF!"}},
            {"a column inserted before them", StructEdit::insertColumns(0, 0, 1),
             {"sqref=\"C3:C6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$B3&gt;$E$8</formula>", "sqref=\"D3:D6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$B3&gt;$E$4</formula>"}, {"#REF!"}},
            {"the column the rules read deleted", StructEdit::deleteColumns(0, 3, 1),
             {"sqref=\"B3:B6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;#REF!</formula>", "sqref=\"C3:C6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"2\"><formula>$A3&gt;#REF!</formula>"}, {}},
        };
        for (const Case &c : cases) {
            const Saved s = saveWith(book, {c.edit});
            QVERIFY2(s.error.isEmpty(), c.what);
            const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
            for (const QString &e : c.expected) QVERIFY2(sheet.contains(e), qPrintable(QStringLiteral("%1: expected %2 in %3").arg(QLatin1String(c.what), e, sheet)));
            for (const QString &a : c.absent) QVERIFY2(!sheet.contains(a), qPrintable(QStringLiteral("%1: %2 should not be in %3").arg(QLatin1String(c.what), a, sheet)));
        }
    }

    void aHyperlinkFollowsRowsAndColumnsAsWrittenByHand()
    {
        QString rows;
        for (int r = 1; r <= 12; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        const QString after = QStringLiteral("<hyperlinks><hyperlink ref=\"A5\" location=\"Sheet1!B7\" display=\"one\"/><hyperlink ref=\"A9:A10\" location=\"Sheet1!B2\" display=\"two\"/></hyperlinks>");
        const QByteArray book = buildXlsx({worksheet(rows, after)});
        struct Case {
            const char *what;
            StructEdit edit;
            QStringList expected;
            QStringList absent;
        };
        const QVector<Case> cases = {
            {"two rows inserted above", StructEdit::insertRows(0, 0, 2),
             {"<hyperlink ref=\"A7\" location=\"Sheet1!B9\" display=\"one\"/>", "<hyperlink ref=\"A11:A12\" location=\"Sheet1!B4\" display=\"two\"/>"}, {"#REF!"}},
            {"a row inserted between the first link and its target", StructEdit::insertRows(0, 5, 1),
             {"<hyperlink ref=\"A5\" location=\"Sheet1!B8\" display=\"one\"/>", "<hyperlink ref=\"A10:A11\" location=\"Sheet1!B2\" display=\"two\"/>"}, {"#REF!"}},
            {"a row inserted inside the second link's cells", StructEdit::insertRows(0, 9, 1),
             {"<hyperlink ref=\"A5\" location=\"Sheet1!B7\" display=\"one\"/>", "<hyperlink ref=\"A9:A11\" location=\"Sheet1!B2\" display=\"two\"/>"}, {"#REF!"}},
            {"the row of the first link deleted", StructEdit::deleteRows(0, 4, 1),
             {"<hyperlink ref=\"A8:A9\" location=\"Sheet1!B2\" display=\"two\"/>"}, {"display=\"one\"", "#REF!"}},
            {"the row the first link goes to deleted", StructEdit::deleteRows(0, 6, 1),
             {"<hyperlink ref=\"A5\" location=\"Sheet1!#REF!\" display=\"one\"/>", "<hyperlink ref=\"A8:A9\" location=\"Sheet1!B2\" display=\"two\"/>"}, {}},
            {"both rows of the second link deleted", StructEdit::deleteRows(0, 8, 2),
             {"<hyperlink ref=\"A5\" location=\"Sheet1!B7\" display=\"one\"/>"}, {"display=\"two\"", "#REF!"}},
            {"a column inserted before them", StructEdit::insertColumns(0, 0, 1),
             {"<hyperlink ref=\"B5\" location=\"Sheet1!C7\" display=\"one\"/>", "<hyperlink ref=\"B9:B10\" location=\"Sheet1!C2\" display=\"two\"/>"}, {"#REF!"}},
        };
        for (const Case &c : cases) {
            const Saved s = saveWith(book, {c.edit});
            QVERIFY2(s.error.isEmpty(), c.what);
            const QString sheet = text(s, QStringLiteral("xl/worksheets/sheet1.xml"));
            for (const QString &e : c.expected) QVERIFY2(sheet.contains(e), qPrintable(QStringLiteral("%1: expected %2 in %3").arg(QLatin1String(c.what), e, sheet)));
            for (const QString &a : c.absent) QVERIFY2(!sheet.contains(a), qPrintable(QStringLiteral("%1: %2 should not be in %3").arg(QLatin1String(c.what), a, sheet)));
        }
    }

    void aChartSeriesFollowsRowsAndColumnsAsWrittenByHand()
    {
        QString rows;
        for (int r = 1; r <= 12; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%1</v></c></row>").arg(r);
        const QByteArray chart = "<?xml version=\"1.0\"?><c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart><c:plotArea><c:barChart><c:ser>"
                                 "<c:tx><c:strRef><c:f>Sheet1!$B$1</c:f></c:strRef></c:tx><c:cat><c:numRef><c:f>Sheet1!$A$2:$A$9</c:f></c:numRef></c:cat><c:val><c:numRef><c:f>Sheet1!$B$2:$B$9</c:f></c:numRef></c:val></c:ser>"
                                 "<c:ser><c:val><c:numRef><c:f>Sheet2!$A$1:$A$3</c:f></c:numRef></c:val></c:ser></c:barChart></c:plotArea></c:chart></c:chartSpace>";
        QVector<ExtraPart> parts;
        parts.append({QStringLiteral("xl/charts/chart1.xml"), chart, QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"), -1, QString(), QString(), QString()});
        const QByteArray book = withParts(buildXlsx({worksheet(rows), worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}), parts);
        struct Case {
            const char *what;
            StructEdit edit;
            QString name, categories, values, other;   // the four formulas, as they are to be
        };
        const QString s2 = QStringLiteral("Sheet2!$A$1:$A$3");
        const QVector<Case> cases = {
            {"a row inserted above", StructEdit::insertRows(0, 0, 1), "Sheet1!$B$2", "Sheet1!$A$3:$A$10", "Sheet1!$B$3:$B$10", s2},
            {"two rows inserted inside the series", StructEdit::insertRows(0, 4, 2), "Sheet1!$B$1", "Sheet1!$A$2:$A$11", "Sheet1!$B$2:$B$11", s2},
            {"a row inserted below the series", StructEdit::insertRows(0, 9, 1), "Sheet1!$B$1", "Sheet1!$A$2:$A$9", "Sheet1!$B$2:$B$9", s2},
            {"the first row deleted: the name's cell is gone", StructEdit::deleteRows(0, 0, 1), "Sheet1!#REF!", "Sheet1!$A$1:$A$8", "Sheet1!$B$1:$B$8", s2},
            {"a row of the series deleted", StructEdit::deleteRows(0, 4, 1), "Sheet1!$B$1", "Sheet1!$A$2:$A$8", "Sheet1!$B$2:$B$8", s2},
            {"all the rows of the series deleted", StructEdit::deleteRows(0, 1, 8), "Sheet1!$B$1", "Sheet1!#REF!", "Sheet1!#REF!", s2},
            {"a row deleted below the series", StructEdit::deleteRows(0, 10, 1), "Sheet1!$B$1", "Sheet1!$A$2:$A$9", "Sheet1!$B$2:$B$9", s2},
            {"a column inserted before them", StructEdit::insertColumns(0, 0, 1), "Sheet1!$C$1", "Sheet1!$B$2:$B$9", "Sheet1!$C$2:$C$9", s2},
            {"the column of the categories deleted", StructEdit::deleteColumns(0, 0, 1), "Sheet1!$A$1", "Sheet1!#REF!", "Sheet1!$A$2:$A$9", s2},
            {"a row inserted in the other sheet", StructEdit::insertRows(1, 0, 1), "Sheet1!$B$1", "Sheet1!$A$2:$A$9", "Sheet1!$B$2:$B$9", "Sheet2!$A$2:$A$4"},
        };
        for (const Case &c : cases) {
            const Saved s = saveWith(book, {c.edit});
            QVERIFY2(s.error.isEmpty(), c.what);
            const QString xml = text(s, QStringLiteral("xl/charts/chart1.xml"));
            const QString want = QStringLiteral("<c:tx><c:strRef><c:f>%1</c:f></c:strRef></c:tx><c:cat><c:numRef><c:f>%2</c:f></c:numRef></c:cat><c:val><c:numRef><c:f>%3</c:f></c:numRef></c:val></c:ser>"
                                                "<c:ser><c:val><c:numRef><c:f>%4</c:f>").arg(c.name, c.categories, c.values, c.other);
            QVERIFY2(xml.contains(want), qPrintable(QStringLiteral("%1: expected %2 in %3").arg(QLatin1String(c.what), want, xml)));
        }
    }
};

JP_TEST_CLASS(SheetStructSaveTests)
#include "tst_sheet_structsave.moc"
