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
};

JP_TEST_CLASS(SheetStructSaveTests)
#include "tst_sheet_structsave.moc"
