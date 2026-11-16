#pragma once
// A workbook of our own making for the tests of a chart that keeps its numbers in a workbook beside it (sheet/chartembed.h): one sheet of cells given as rows of XML, as a program that
// writes a chart with an embedded workbook would.

#include "office/package.h"

#include <QByteArray>
#include <QString>

namespace jp::testing {

// A cell for the rows: a number, or words (an inline string).
inline QString bookNumber(const QString &at, double v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(at).arg(v, 0, 'g', 15); }
inline QString bookText(const QString &at, const QString &v) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(at, v); }
inline QString bookRow(int row, const QStringList &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(row).arg(cells.join(QString())); }

// The package of a workbook with one sheet (`rows` are the XML of its rows) and an extra part, so that a test can see what a change leaves alone.
inline QByteArray bookOf(const QString &rows, const QString &sheetName = QStringLiteral("Sheet1"))
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
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/xl/workbook.xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("worksheet"), QStringLiteral("worksheets/sheet1.xml"));
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
