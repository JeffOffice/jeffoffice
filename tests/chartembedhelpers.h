#pragma once
// A workbook of our own making for the tests of a chart that keeps its numbers in a workbook beside it (sheet/chartembed.h): one sheet of cells given as rows of XML, as a program that
// writes a chart with an embedded workbook would.

#include "office/package.h"

#include <QByteArray>
#include <QLocale>
#include <QString>

namespace jp::testing {

// A cell for the rows: a number, or words (an inline string).
inline QString bookNumber(const QString &at, double v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(at, QLocale::c().toString(v, 'g', QLocale::FloatingPointShortest)); }
inline QString bookText(const QString &at, const QString &v) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(at, v); }
inline QString bookRow(int row, const QStringList &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(row).arg(cells.join(QString())); }

// The package of a workbook with one sheet (`rows` are the XML of its rows) and an extra part, so that a test can see what a change leaves alone.
inline QByteArray bookOf(const QString &rows, const QString &sheetName = QStringLiteral("Sheet1"), const QByteArray &extraPart = QByteArray())
{
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QString ns = QStringLiteral("xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"");
    office::PackageWriter w;
    w.addPart(QStringLiteral("/xl/workbook.xml"),
              QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><workbook %1><sheets><sheet name=\"%2\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>").arg(ns, sheetName).toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><worksheet %1><sheetData>%2</sheetData></worksheet>").arg(ns, rows).toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    w.addPart(QStringLiteral("/docProps/custom-note.xml"), QByteArray("<note>kept as it is</note>"), QStringLiteral("application/xml"));
    if (!extraPart.isEmpty()) w.addPart(QStringLiteral("/docProps/padding.bin"), extraPart, QStringLiteral("application/octet-stream"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/xl/workbook.xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("worksheet"), QStringLiteral("worksheets/sheet1.xml"));
    return w.finish();
}

// A cell with a cell format of the styles part of richBook (1: a date, 2: 0.00, 3: General again) and a number, or none.
inline QString bookStyled(const QString &at, int style, double v) { return QStringLiteral("<c r=\"%1\" s=\"%2\"><v>%3</v></c>").arg(at).arg(style).arg(QLocale::c().toString(v, 'g', QLocale::FloatingPointShortest)); }
inline QString bookShared(const QString &at, int index) { return QStringLiteral("<c r=\"%1\" t=\"s\"><v>%2</v></c>").arg(at).arg(index); }

// A workbook as a program with more in it would write: two sheets, shared strings, a styles part (a date format and 0.00), a formula that reads the chart's cells, and a defined name. `rows` are the
// rows of the first sheet, "Sheet1" (the shared strings are 0 Sales, 1 Costs, 2 Q1, 3 Q2, 4 Q3).
inline QByteArray richBook(const QString &rows)
{
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QString ns = QStringLiteral("xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"");
    const QString head = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
    office::PackageWriter w;
    w.addPart(QStringLiteral("/xl/workbook.xml"),
              (head + QStringLiteral("<workbook %1><sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/><sheet name=\"Data2\" sheetId=\"2\" r:id=\"rId2\"/></sheets>"
                                    "<definedNames><definedName name=\"Total\">Sheet1!$B$2:$B$4</definedName></definedNames></workbook>").arg(ns)).toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), (head + QStringLiteral("<worksheet %1><sheetData>%2</sheetData></worksheet>").arg(ns, rows)).toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet2.xml"),
              (head + QStringLiteral("<worksheet %1><sheetData><row r=\"1\"><c r=\"A1\"><f>SUM(Sheet1!B2:B4)</f><v>6</v></c><c r=\"B1\"><f>Total*2</f><v>12</v></c></row></sheetData></worksheet>").arg(ns)).toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    w.addPart(QStringLiteral("/xl/sharedStrings.xml"),
              (head + QStringLiteral("<sst %1 count=\"5\" uniqueCount=\"5\"><si><t>Sales</t></si><si><t>Costs</t></si><si><t>Q1</t></si><si><t>Q2</t></si><si><t>Q3</t></si></sst>").arg(ns)).toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"));
    w.addPart(QStringLiteral("/xl/styles.xml"),
              (head + QStringLiteral("<styleSheet %1><numFmts count=\"1\"><numFmt numFmtId=\"164\" formatCode=\"0.00\"/></numFmts><fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Arial\"/></font></fonts>"
                                    "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills><borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                    "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                                    "<cellXfs count=\"4\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"14\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyNumberFormat=\"1\"/>"
                                    "<xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyNumberFormat=\"1\"/><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/></cellXfs></styleSheet>").arg(ns)).toUtf8(),
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/xl/workbook.xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("worksheet"), QStringLiteral("worksheets/sheet1.xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("worksheet"), QStringLiteral("worksheets/sheet2.xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("sharedStrings"), QStringLiteral("sharedStrings.xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("styles"), QStringLiteral("styles.xml"));
    return w.finish();
}

// Q1 to Q3 down A2:A4, the names Sales and Costs in B1 and C1, and numbers under them (1, 2, 3 and 4, 5, 6).
inline QByteArray salesBook()
{
    return bookOf(bookRow(1, {bookText(QStringLiteral("B1"), QStringLiteral("Sales")), bookText(QStringLiteral("C1"), QStringLiteral("Costs"))}) +
                  bookRow(2, {bookText(QStringLiteral("A2"), QStringLiteral("Q1")), bookNumber(QStringLiteral("B2"), 1), bookNumber(QStringLiteral("C2"), 4)}) +
                  bookRow(3, {bookText(QStringLiteral("A3"), QStringLiteral("Q2")), bookNumber(QStringLiteral("B3"), 2), bookNumber(QStringLiteral("C3"), 5)}) +
                  bookRow(4, {bookText(QStringLiteral("A4"), QStringLiteral("Q3")), bookNumber(QStringLiteral("B4"), 3), bookNumber(QStringLiteral("C4"), 6)}));
}

} // namespace jp::testing
