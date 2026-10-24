// A workbook with something of everything in it, for the tests of rows, columns, and cells inserted and deleted (built in memory).
#pragma once

#include "sheethelpers.h"

namespace sheettest {

// A workbook of two sheets with something of everything:
//          A    B (shared)   C         D          E            F
//   1      1    =A1*2        text      7          [merged E1:F2]
//   2      2    =A2*2
//   3      3    =A3*2        {=A3:A4*2}
//   4      4    =A4*2        (array)
//   5      5    =A5*2                   =SUM(A1:A5) =Sheet2!A1+1
//   6                        x
//   7      [merged A7:B7]
//   8      link
// Sheet2: A1 =Sheet1!A2, A2 =SUM(Sheet1!A1:A5), with a validation that lists Sheet1!$A$1:$A$5.
inline QByteArray richBook()
{
    const QString rows1 = QStringLiteral(
        "<row r=\"1\" spans=\"1:6\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"shared\" ref=\"B1:B5\" si=\"0\">A1*2</f><v>2</v></c><c r=\"C1\" t=\"inlineStr\"><is><t>text</t></is></c><c r=\"D1\"><v>7</v></c></row>"
        "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"B2\"><f t=\"shared\" si=\"0\"/><v>4</v></c></row>"
        "<row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"B3\"><f t=\"shared\" si=\"0\"/><v>6</v></c><c r=\"C3\"><f t=\"array\" ref=\"C3:C4\">A3:A4*2</f><v>6</v></c></row>"
        "<row r=\"4\"><c r=\"A4\"><v>4</v></c><c r=\"B4\"><f t=\"shared\" si=\"0\"/><v>8</v></c><c r=\"C4\"><v>8</v></c></row>"
        "<row r=\"5\"><c r=\"A5\"><v>5</v></c><c r=\"B5\"><f t=\"shared\" si=\"0\"/><v>10</v></c><c r=\"D5\"><f>SUM(A1:A5)</f><v>15</v></c><c r=\"E5\"><f>Sheet2!A1+1</f><v>3</v></c></row>"
        "<row r=\"6\"><c r=\"C6\" t=\"inlineStr\"><is><t>x</t></is></c></row>"
        "<row r=\"8\"><c r=\"A8\" t=\"inlineStr\"><is><t>link</t></is></c></row>");
    const QString after1 = QStringLiteral(
        "<autoFilter ref=\"A1:D5\"><filterColumn colId=\"2\"><customFilters><customFilter operator=\"equal\" val=\"x\"/></customFilters></filterColumn></autoFilter>"
        "<mergeCells count=\"2\"><mergeCell ref=\"A7:B7\"/><mergeCell ref=\"E1:F2\"/></mergeCells>"
        "<conditionalFormatting sqref=\"A1:A5\"><cfRule type=\"expression\" priority=\"1\"><formula>A1&gt;$D$1</formula></cfRule></conditionalFormatting>"
        "<dataValidations count=\"1\"><dataValidation type=\"list\" sqref=\"C6\"><formula1>$A$1:$A$5</formula1></dataValidation></dataValidations>"
        "<hyperlinks><hyperlink ref=\"A8\" location=\"Sheet2!A1\" display=\"x\"/></hyperlinks>"
        "<rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"4\" max=\"16383\" man=\"1\"/></rowBreaks>"
        "<drawing r:id=\"rIdD\"/><legacyDrawing r:id=\"rIdV\"/>");
    const QString head1 = QStringLiteral("<dimension ref=\"A1:F8\"/><cols><col min=\"1\" max=\"1\" width=\"12\" customWidth=\"1\"/><col min=\"3\" max=\"4\" width=\"20\" customWidth=\"1\"/></cols>");
    const QByteArray sheet1 = worksheet(rows1, after1, head1);
    const QByteArray sheet2 = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>Sheet1!A2</f><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><f>SUM(Sheet1!A1:A5)</f><v>15</v></c></row>"),
                                        QStringLiteral("<dataValidations count=\"1\"><dataValidation type=\"list\" sqref=\"B1\"><formula1>Sheet1!$A$1:$A$5</formula1></dataValidation></dataValidations>"));
    const QByteArray book = buildXlsx({sheet1, sheet2}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QStringLiteral("<definedName name=\"Total\">Sheet1!$A$1:$A$5</definedName><definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">Sheet1!$A$1:$D$8</definedName>"));
    const QByteArray chart = "<?xml version=\"1.0\"?><c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart><c:plotArea><c:barChart><c:ser><c:val><c:numRef><c:f>Sheet1!$A$1:$A$5</c:f></c:numRef></c:val></c:ser></c:barChart></c:plotArea></c:chart></c:chartSpace>";
    const QByteArray drawing = "<?xml version=\"1.0\"?><xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\"><xdr:twoCellAnchor><xdr:from><xdr:col>1</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>"
                               "<xdr:to><xdr:col>4</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>6</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to></xdr:twoCellAnchor></xdr:wsDr>";
    const QByteArray comments = "<?xml version=\"1.0\"?><comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>a</author></authors><commentList><comment ref=\"A3\" authorId=\"0\"><text><t>note</t></text></comment><comment ref=\"A4\" authorId=\"0\"><text><t>other</t></text></comment></commentList></comments>";
    const QByteArray vml = "<xml><v:shape id=\"s1\"><x:ClientData ObjectType=\"Note\"><x:Anchor>1, 15, 2, 2, 3, 15, 4, 16</x:Anchor><x:Row>2</x:Row><x:Column>0</x:Column></x:ClientData></v:shape>"
                           "<v:shape id=\"s2\"><x:ClientData ObjectType=\"Note\"><x:Anchor>1, 15, 3, 2, 3, 15, 5, 16</x:Anchor><x:Row>3</x:Row><x:Column>0</x:Column></x:ClientData></v:shape></xml>";
    QVector<ExtraPart> parts;
    parts.append({QStringLiteral("xl/charts/chart1.xml"), chart, QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"), -1, QString(), QString(), QString()});
    parts.append({QStringLiteral("xl/drawings/drawing1.xml"), drawing, QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), 0, QStringLiteral("rIdD"), QStringLiteral("drawing"), QStringLiteral("../drawings/drawing1.xml")});
    parts.append({QStringLiteral("xl/comments1.xml"), comments, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"), 0, QStringLiteral("rIdC"), QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
    parts.append({QStringLiteral("xl/drawings/vmlDrawing1.vml"), vml, QString(), 0, QStringLiteral("rIdV"), QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml")});
    return withParts(book, parts);
}


} // namespace sheettest
