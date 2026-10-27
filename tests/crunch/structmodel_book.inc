// The workbook every run of the model check begins with (included by structmodel.cpp): built here, with the package writer, in memory.
//
//   Data (A1:L40)                                                          Calc (A1:F40)                    Extra (A1:D20)
//     A   numbers, 1 to 30            D   labels             H2:K12 a table "Tbl" (Item, Qty,            A   numbers          A   numbers
//     B   =A1*2 (a shared family)     E   one formula of each   Price, Amount; Amount is a              B   =Data!A1+A1      B   =A1+1 (shared)
//     C   =$A1+B$1 (shared, mixed)        kind of reference (30)  calculated column)                    C   =SUM(A1:B1)      C   =Data!A1*2 (shared)
//     F   =SUM(A1:C1) (shared)        G, L   numbers          E1:E4 formulas that read Data
//   rows 31 to 40: numbers; merged C34:D35 and F38:G38; a filter on A1:D30; a chart over G15:K26.
//   Names: Total, ColB, RowSet, Dyn, TblQty, Loc (Calc's), the print area and titles. Notes on Data A3, A4 and Calc B2; links on Data A8 and D5:D6.
// The formulas in column E are the zoo: relative, absolute, mixed, range, whole column and row, other sheet, 3-D, name, table, OFFSET, INDIRECT, ROW, COLUMN, ADDRESS, and the like.

namespace {

const QString kMainNs = QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main");
const QString kRelNs = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");
const QString kRelBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");

struct GridXml {
    std::map<int, std::map<int, QString>> rows;
    std::map<int, QString> rowAttributes;
    void set(int row, int col, const QString &xml) { rows[row][col] = xml; }
    void number(int row, int col, double v) { set(row, col, QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(cellName(quint32(row), quint32(col))).arg(v, 0, 'g', 15)); }
    void text(int row, int col, const QString &s) { set(row, col, QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(cellName(quint32(row), quint32(col)), s.toHtmlEscaped())); }
    void formula(int row, int col, const QString &f) { set(row, col, QStringLiteral("<c r=\"%1\"><f>%2</f><v>0</v></c>").arg(cellName(quint32(row), quint32(col)), f.toHtmlEscaped())); }
    // A family: the formula is written for the first row, and each cell of the column down to `lastRow` is a member.
    void family(int firstRow, int lastRow, int col, int index, const QString &f)
    {
        for (int r = firstRow; r <= lastRow; ++r) {
            const QString ref = cellName(quint32(r), quint32(col));
            if (r == firstRow)
                set(r, col, QStringLiteral("<c r=\"%1\"><f t=\"shared\" ref=\"%2:%3\" si=\"%4\">%5</f><v>0</v></c>").arg(ref, ref, cellName(quint32(lastRow), quint32(col))).arg(index).arg(f.toHtmlEscaped()));
            else
                set(r, col, QStringLiteral("<c r=\"%1\"><f t=\"shared\" si=\"%2\"/><v>0</v></c>").arg(ref).arg(index));
        }
    }
    QString xml() const
    {
        QString out;
        std::map<int, bool> seen;
        for (const auto &row : rows) {
            out += QStringLiteral("<row r=\"%1\"%2>").arg(row.first + 1).arg(rowAttributes.count(row.first) ? rowAttributes.at(row.first) : QString());
            for (const auto &cell : row.second) out += cell.second;
            out += QStringLiteral("</row>");
        }
        return out;
    }
};

QString dataZoo(int i)
{
    static const char *const zoo[30] = {
        "SUM($A$1:$A$30)",                       // 1 absolute range
        "SUM(A1:A30)",                           // 2 relative range
        "SUM(A:A)",                              // 3 a whole column
        "SUM(2:2)",                              // 4 a whole row
        "Calc!A1+Calc!B2",                       // 5 another sheet, cells
        "SUM(Calc!A1:A10)",                      // 6 another sheet, a range
        "SUM(Data:Extra!B2:B5)",                 // 7 a block of sheets
        "SUM(Total)",                            // 8 a name for a block
        "SUM(ColB)",                             // 9 a name for a column
        "SUM(RowSet)",                             // 10 a name for a row
        "SUM(Dyn)",                              // 11 a name that is OFFSET
        "SUM(Tbl[Qty])",                         // 12 a table's column
        "SUMPRODUCT(Tbl[Qty],Tbl[Price])",       // 13 two of them
        "SUM(TblQty)",                           // 14 a name for a table's column
        "OFFSET(A1,2,1)",                        // 15 OFFSET
        "SUM(OFFSET(A1,0,0,3,2))",               // 16 OFFSET, a block
        "INDIRECT(\"A5\")",                      // 17 INDIRECT
        "SUM(INDIRECT(\"A1:A\"&3))",             // 18 INDIRECT, a block made of text
        "ROW(A7)",                               // 19 the row of a reference
        "COLUMN(D1)",                            // 20 the column of a reference
        "ADDRESS(ROW(B3),COLUMN(B3))",           // 21 the address of one
        "SUMPRODUCT(ROW(A1:A5))",                // 22 the rows of a block
        "ROW()+COLUMN()",                        // 23 its own place
        "INDEX(A1:C30,3,2)",                     // 24 INDEX
        "MATCH(7,A1:A30,0)",                     // 25 MATCH
        "VLOOKUP(5,A1:C30,2,FALSE)",             // 26 VLOOKUP
        "COUNTIF(A1:A30,\">5\")",                // 27 COUNTIF
        "IF(A1>5,B1,C1)",                        // 28 relative cells
        "MAX(Calc!B:B)",                         // 29 another sheet, a whole column
        "Extra!A1+Extra!A2+SUM(Extra!B1:B3)",    // 30 another sheet again
    };
    return QString::fromLatin1(zoo[i]);
}

QString sheetHead(const QString &cols = QString()) { return QStringLiteral("<sheetFormatPr defaultRowHeight=\"15\"/>") + cols; }

QString sheetPart(const GridXml &grid, const QString &head, const QString &tail)
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<worksheet xmlns=\"%1\" xmlns:r=\"%2\">%3<sheetData>%4</sheetData>%5</worksheet>").arg(kMainNs, kRelNs, head, grid.xml(), tail);
}

} // namespace

QByteArray baseWorkbook()
{
    // ---- Data
    GridXml data;
    for (int r = 0; r < 30; ++r) {
        data.number(r, 0, ((r * 7 + 3) % 23) + 1);
        data.text(r, 3, QStringLiteral("t%1").arg(r % 5));
        data.formula(r, 4, dataZoo(r));
        data.number(r, 6, (r + 1) * 2);
        data.number(r, 11, 100 + r);
    }
    data.family(0, 29, 1, 0, QStringLiteral("A1*2"));
    data.family(0, 29, 2, 1, QStringLiteral("$A1+B$1"));
    data.family(0, 29, 5, 2, QStringLiteral("SUM(A1:C1)"));
    // the table H2:K12 (the header row is row 2)
    data.text(1, 7, QStringLiteral("Item"));
    data.text(1, 8, QStringLiteral("Qty"));
    data.text(1, 9, QStringLiteral("Price"));
    data.text(1, 10, QStringLiteral("Amount"));
    for (int k = 0; k < 10; ++k) {
        const int r = 2 + k;
        data.text(r, 7, QStringLiteral("i%1").arg(k));
        data.number(r, 8, k + 1);
        data.number(r, 9, ((k * 3) % 7) + 2);
        data.formula(r, 10, QStringLiteral("Tbl[[#This Row],[Qty]]*Tbl[[#This Row],[Price]]"));
    }
    for (int r = 30; r < 40; ++r)
        for (int c : {0, 1, 3, 4, 5, 6, 7, 11})
            if (!((r == 33 || r == 34) && c == 3) && !(r == 37 && c == 6)) data.number(r, c, r * 10 + c);   // (the merged cells but the first are empty)
    data.number(33, 2, 77);   // (the first cell of the merged C34:D35)
    data.number(37, 5, 55);   // (and of F38:G38)
    data.rowAttributes[4] = QStringLiteral(" ht=\"30\" customHeight=\"1\"");
    data.rowAttributes[8] = QStringLiteral(" hidden=\"1\"");

    // ---- Calc
    GridXml calc;
    for (int r = 0; r < 30; ++r) {
        calc.number(r, 0, ((r * 5) % 11) + 1);
        if (r < 10) calc.number(r, 3, r * 2);
    }
    calc.family(0, 29, 1, 0, QStringLiteral("Data!A1+A1"));
    calc.family(0, 29, 2, 1, QStringLiteral("SUM(A1:B1)"));
    calc.formula(0, 4, QStringLiteral("SUM(Loc)"));
    calc.formula(1, 4, QStringLiteral("Data!E1+1"));
    calc.formula(2, 4, QStringLiteral("SUM(Data!$A$1:$A$10)+SUM(Tbl[Amount])"));
    calc.formula(3, 4, QStringLiteral("INDEX(Data!A1:C30,2,2)"));
    for (int r = 30; r < 40; ++r) {
        calc.number(r, 0, r);
        calc.number(r, 3, r * 2);
    }

    // ---- Extra
    GridXml extra;
    for (int r = 0; r < 20; ++r) extra.number(r, 0, ((r * 3) % 13) + 1);
    extra.family(0, 9, 1, 0, QStringLiteral("A1+1"));
    extra.family(0, 19, 2, 1, QStringLiteral("Data!A1*2"));
    extra.number(0, 3, 1);

    PackageWriter w;
    const QString wbPart = QStringLiteral("/xl/workbook.xml");
    w.addPart(wbPart, QByteArray(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRelBase + "officeDocument", wbPart);
    const QString stylesPart = QStringLiteral("/xl/styles.xml");
    w.addPart(stylesPart,
              QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"%1\"><fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
                             "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                             "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                             "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                             "<cellXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/></cellXfs>"
                             "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles><dxfs count=\"1\"><dxf><font><b/></font></dxf></dxfs></styleSheet>")
                  .arg(kMainNs)
                  .toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"));
    w.addRelationship(wbPart, kRelBase + "styles", stylesPart);
    w.addDefault(QStringLiteral("vml"), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"));

    const QString dataPart = QStringLiteral("/xl/worksheets/sheet1.xml"), calcPart = QStringLiteral("/xl/worksheets/sheet2.xml"), extraPart = QStringLiteral("/xl/worksheets/sheet3.xml");
    auto notes = [&](const QString &sheetPart, int n, const QVector<QPair<QString, QString>> &list) {
        const QString commentsName = QStringLiteral("/xl/comments%1.xml").arg(n), vmlName = QStringLiteral("/xl/drawings/vmlDrawing%1.vml").arg(n);
        QString comments = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"%1\"><authors><author>a</author></authors><commentList>").arg(kMainNs), vml = QStringLiteral("<xml>");
        for (const auto &note : list) {
            quint32 row = 0, col = 0;
            parseCellRef(note.first, &row, &col);
            comments += QStringLiteral("<comment ref=\"%1\" authorId=\"0\"><text><r><t>%2</t></r></text></comment>").arg(note.first, note.second.toHtmlEscaped());
            vml += QStringLiteral("<v:shape id=\"s%1\"><x:ClientData ObjectType=\"Note\"><x:Anchor>%2, 15, %3, 2, %4, 15, %5, 16</x:Anchor><x:Row>%3</x:Row><x:Column>%2</x:Column></x:ClientData></v:shape>")
                       .arg(note.first).arg(col + 1).arg(row).arg(col + 3).arg(row + 2);
        }
        comments += QStringLiteral("</commentList></comments>");
        vml += QStringLiteral("</xml>");
        w.addPart(commentsName, comments.toUtf8(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"));
        w.addPart(vmlName, vml.toUtf8(), QString());
        return QPair<QString, QString>(w.addRelationship(sheetPart, kRelBase + "comments", commentsName), w.addRelationship(sheetPart, kRelBase + "vmlDrawing", vmlName));
    };
    // Data's rels: comments, vml, the drawing, and the table
    const auto dataNotes = notes(dataPart, 1, {{QStringLiteral("A3"), QStringLiteral("first note")}, {QStringLiteral("A4"), QStringLiteral("second note")}});
    const QString drawingName = QStringLiteral("/xl/drawings/drawing1.xml"), chartName = QStringLiteral("/xl/charts/chart1.xml"), tableName = QStringLiteral("/xl/tables/table1.xml");
    const QString drawingRel = w.addRelationship(dataPart, kRelBase + "drawing", drawingName);
    const QString tableRel = w.addRelationship(dataPart, kRelBase + "table", tableName);
    const QString chartRel = w.addRelationship(drawingName, kRelBase + "chart", chartName);
    const QString xdr = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"), dml = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main"),
                  chartNs = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/chart");
    w.addPart(drawingName,
              QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<xdr:wsDr xmlns:xdr=\"%1\" xmlns:a=\"%2\" xmlns:r=\"%3\"><xdr:twoCellAnchor><xdr:from><xdr:col>6</xdr:col><xdr:colOff>0</xdr:colOff>"
                             "<xdr:row>14</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>10</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>25</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>"
                             "<xdr:graphicFrame macro=\"\"><xdr:nvGraphicFramePr><xdr:cNvPr id=\"2\" name=\"Chart 1\"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr><xdr:xfrm><a:off x=\"0\" y=\"0\"/>"
                             "<a:ext cx=\"0\" cy=\"0\"/></xdr:xfrm><a:graphic><a:graphicData uri=\"%4\"><c:chart xmlns:c=\"%4\" r:id=\"%5\"/></a:graphicData></a:graphic></xdr:graphicFrame><xdr:clientData/></xdr:twoCellAnchor></xdr:wsDr>")
                  .arg(xdr, dml, kRelNs, chartNs, chartRel)
                  .toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"));
    auto series = [&](int i, const QString &name, const QString &cat, const QString &val) {
        return QStringLiteral("<c:ser><c:idx val=\"%1\"/><c:order val=\"%1\"/><c:tx><c:strRef><c:f>%2</c:f></c:strRef></c:tx><c:cat><c:strRef><c:f>%3</c:f></c:strRef></c:cat><c:val><c:numRef><c:f>%4</c:f></c:numRef></c:val></c:ser>")
            .arg(i).arg(name, cat, val);
    };
    w.addPart(chartName,
              QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<c:chartSpace xmlns:c=\"%1\" xmlns:a=\"%2\" xmlns:r=\"%3\"><c:chart><c:autoTitleDeleted val=\"1\"/><c:plotArea><c:layout/>"
                             "<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:varyColors val=\"0\"/>%4%5<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>"
                             "<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:crossAx val=\"222\"/></c:catAx>"
                             "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:crossAx val=\"111\"/></c:valAx></c:plotArea>"
                             "<c:plotVisOnly val=\"1\"/><c:dispBlanksAs val=\"gap\"/></c:chart></c:chartSpace>")
                  .arg(chartNs, dml, kRelNs, series(0, QStringLiteral("Data!$H$2"), QStringLiteral("Data!$D$1:$D$10"), QStringLiteral("Data!$A$1:$A$10")),
                       series(1, QStringLiteral("Calc!$A$1"), QStringLiteral("Data!$D$1:$D$10"), QStringLiteral("Calc!$B$1:$B$10")))
                  .toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
    w.addPart(tableName,
              QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" id=\"1\" name=\"Tbl\" displayName=\"Tbl\" ref=\"H2:K12\"><autoFilter ref=\"H2:K12\"/><tableColumns count=\"4\">"
                             "<tableColumn id=\"1\" name=\"Item\"/><tableColumn id=\"2\" name=\"Qty\"/><tableColumn id=\"3\" name=\"Price\"/><tableColumn id=\"4\" name=\"Amount\"><calculatedColumnFormula>Tbl[[#This Row],[Qty]]*Tbl[[#This Row],[Price]]"
                             "</calculatedColumnFormula></tableColumn></tableColumns><tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/></table>")
                  .arg(kMainNs)
                  .toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.table+xml"));
    const auto calcNotes = notes(calcPart, 2, {{QStringLiteral("B2"), QStringLiteral("calc note")}});

    const QString dataTail =
        QStringLiteral("<autoFilter ref=\"A1:D30\"/>"
                       "<mergeCells count=\"2\"><mergeCell ref=\"C34:D35\"/><mergeCell ref=\"F38:G38\"/></mergeCells>"
                       "<conditionalFormatting sqref=\"A1:A30\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>A1&gt;$A$15</formula></cfRule></conditionalFormatting>"
                       "<conditionalFormatting sqref=\"B1:C30\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"2\" operator=\"greaterThan\"><formula>20</formula></cfRule></conditionalFormatting>"
                       "<dataValidations count=\"1\"><dataValidation type=\"list\" allowBlank=\"1\" sqref=\"D1:D30\"><formula1>$A$1:$A$10</formula1></dataValidation></dataValidations>"
                       "<hyperlinks><hyperlink ref=\"A8\" location=\"Calc!A1\" display=\"x\"/><hyperlink ref=\"D5:D6\" location=\"Extra!B2\"/></hyperlinks>"
                       "<rowBreaks count=\"2\" manualBreakCount=\"2\"><brk id=\"10\" max=\"16383\" man=\"1\"/><brk id=\"25\" max=\"16383\" man=\"1\"/></rowBreaks>"
                       "<colBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"6\" max=\"1048575\" man=\"1\"/></colBreaks>"
                       "<drawing r:id=\"%1\"/><legacyDrawing r:id=\"%2\"/><tableParts count=\"1\"><tablePart r:id=\"%3\"/></tableParts>")
            .arg(drawingRel, dataNotes.second, tableRel);
    w.addPart(dataPart,
              sheetPart(data, sheetHead(QStringLiteral("<cols><col min=\"3\" max=\"3\" width=\"20\" customWidth=\"1\"/><col min=\"10\" max=\"10\" width=\"9\" hidden=\"1\" customWidth=\"1\"/></cols>")), dataTail).toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString calcTail =
        QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A32:B32\"/></mergeCells>"
                       "<conditionalFormatting sqref=\"A1:A10\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>Data!$A1&gt;5</formula></cfRule></conditionalFormatting>"
                       "<dataValidations count=\"1\"><dataValidation type=\"list\" sqref=\"C1:C10\"><formula1>Data!$A$1:$A$5</formula1></dataValidation></dataValidations>"
                       "<legacyDrawing r:id=\"%1\"/>")
            .arg(calcNotes.second);
    w.addPart(calcPart, sheetPart(calc, sheetHead(), calcTail).toUtf8(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString extraTail = QStringLiteral("<dataValidations count=\"1\"><dataValidation type=\"whole\" operator=\"between\" sqref=\"A1:A10\"><formula1>0</formula1><formula2>Data!$A$1</formula2></dataValidation></dataValidations>");
    w.addPart(extraPart, sheetPart(extra, sheetHead(), extraTail).toUtf8(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString id1 = w.addRelationship(wbPart, kRelBase + "worksheet", dataPart), id2 = w.addRelationship(wbPart, kRelBase + "worksheet", calcPart), id3 = w.addRelationship(wbPart, kRelBase + "worksheet", extraPart);
    const QString workbook =
        QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<workbook xmlns=\"%1\" xmlns:r=\"%2\"><sheets><sheet name=\"Data\" sheetId=\"1\" r:id=\"%3\"/><sheet name=\"Calc\" sheetId=\"2\" r:id=\"%4\"/>"
                       "<sheet name=\"Extra\" sheetId=\"3\" r:id=\"%5\"/></sheets><definedNames>"
                       "<definedName name=\"_xlnm._FilterDatabase\" localSheetId=\"0\" hidden=\"1\">Data!$A$1:$D$30</definedName>"
                       "<definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">Data!$A$1:$J$30</definedName>"
                       "<definedName name=\"_xlnm.Print_Titles\" localSheetId=\"0\">Data!$1:$2</definedName>"
                       "<definedName name=\"ColB\">Data!$B:$B</definedName><definedName name=\"Dyn\">OFFSET(Data!$A$1,0,0,5,1)</definedName><definedName name=\"Loc\" localSheetId=\"1\">Calc!$A$1:$B$3</definedName>"
                       "<definedName name=\"RowSet\">Data!$3:$3</definedName><definedName name=\"TblQty\">Tbl[Qty]</definedName><definedName name=\"Total\">Data!$A$1:$A$10</definedName></definedNames>"
                       "<calcPr calcId=\"191029\"/></workbook>")
            .arg(kMainNs, kRelNs, id1, id2, id3);
    w.addPart(wbPart, workbook.toUtf8(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    return w.finish();
}
