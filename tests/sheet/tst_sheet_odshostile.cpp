// Tests of the reader of OpenDocument spreadsheets on files made to do harm: counts that repeat past the sheet, text of any length, nesting without end, entities, styles and sheets
// without number, and a part that inflates to more than a document may. Every one ends, in the memory its cells take, with a refusal in words or what could be read.
#include "odshelpers.h"

class SheetOdsHostileTests : public QObject {
    Q_OBJECT

    static bool openIt(const QByteArray &bytes, Workbook *wb, QString *why, const Limits &limits = Limits()) { return openBytes(bytes, QStringLiteral("hostile.ods"), wb, why, limits); }

private Q_SLOTS:
    // A row repeated a million times with a cell repeated across the sheet is a million times 16,384 cells: the read stops at the budget of the sheet, saying so, and keeps the cells before it.
    void repeatsPastTheBudgetStop()
    {
        const QString r = row(floatCell(1, QStringLiteral("table:number-columns-repeated=\"16384\"")), QStringLiteral("table:number-rows-repeated=\"1048576\""));
        Workbook wb;
        QString why;
        Limits limits;
        limits.cellsPerSheet = 100000;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(16384) + r))), &wb, &why, limits), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("more cells")), qPrintable(s.error));
        QVERIFY(s.cells.cellCount() <= 100001);
        QVERIFY(s.cells.cellCount() > 0);
    }

    // A count too large for any number is the sheet's size, not a wrap-around; a column past the last one is not read.
    void absurdCountsAreClipped()
    {
        const QString r = row(floatCell(1, QStringLiteral("table:number-columns-repeated=\"99999999999999999\"")) + floatCell(2));
        Workbook wb;
        QString why;
        Limits limits;
        limits.cellsPerSheet = 1000000;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), QStringLiteral("<table:table-column table:number-columns-repeated=\"99999999999999999999\"/>") + r))), &wb, &why, limits), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(s.cells.cellCount(), qint64(16384));   // (the second cell is past the last column)
        QCOMPARE(s.extentCols, 16384u);
        QCOMPARE(s.columns.size() <= 1, true);
    }

    // Rows past the last row of a sheet are not read, and a million empty repeated rows that carry a height make no cell and no more row sizes than rows.
    void rowsPastTheSheetAreNotMade()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"0.5in\"/></style:style>");
        const QString r = row(floatCell(1)) + row(emptyCell(), QStringLiteral("table:style-name=\"ro1\" table:number-rows-repeated=\"2000000\"")) + row(floatCell(2));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(1) + r), styles)), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(s.cells.cellCount(), qint64(1));   // (the last row is past the sheet)
        QVERIFY(s.rowSpecs.size() <= 1048576);
    }

    // Nesting without end: the spans of a text a hundred thousand deep, and notes inside notes. A sheet is read when it is asked for, so a sheet nested past the bound is refused then, in
    // words, with the cells before it kept; the document opens, and its other sheets can be read.
    void nestingIsBounded()
    {
        QString deep;
        for (int i = 0; i < 100000; ++i) deep += QStringLiteral("<text:span>");
        deep += QStringLiteral("x");
        for (int i = 0; i < 100000; ++i) deep += QStringLiteral("</text:span>");
        const QString spans = table(QStringLiteral("S"), columns(1) + row(floatCell(1)) + row(cell(QStringLiteral("office:value-type=\"string\""), QStringLiteral("<text:p>") + deep + QStringLiteral("</text:p>"))))
                              + table(QStringLiteral("T"), row(floatCell(2)));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(spans)), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("nested")), qPrintable(s.error));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1"));
        QVERIFY(wb.readSheet(1, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:2"));
        QString layers;
        for (int i = 0; i < 100000; ++i) layers += QStringLiteral("<office:annotation>");
        const QString notes = table(QStringLiteral("S"), columns(1) + row(cell(QString(), layers)));
        Workbook wb2;
        QVERIFY2(openIt(buildOds(contentXml(notes)), &wb2, &why), qPrintable(why));
        QVERIFY(!wb2.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("nested")) || s.error.contains(QLatin1String("damaged")), qPrintable(s.error));
        // the same, in the flat form
        Workbook flat;
        QVERIFY2(openBytes(buildFods(spans), QStringLiteral("deep.fods"), &flat, &why), qPrintable(why));
        QVERIFY(!flat.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("nested")), qPrintable(s.error));
        // and a document nested without end before its first sheet is not opened
        QString before;
        for (int i = 0; i < 100000; ++i) before += QStringLiteral("<table:calculation-settings>");
        Workbook head;
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1))), QString(), before)), &head, &why));
        QVERIFY2(why.contains(QLatin1String("nested")) || why.contains(QLatin1String("damaged")), qPrintable(why));
    }

    // A DOCTYPE with entities (a billion laughs) is refused before any entity is read.
    void entitiesAreRefused()
    {
        const QByteArray content = "<?xml version=\"1.0\"?><!DOCTYPE lolz [<!ENTITY a \"aaaaaaaaaa\"><!ENTITY b \"&a;&a;&a;&a;&a;&a;&a;&a;&a;&a;\"><!ENTITY c \"&b;&b;&b;&b;&b;&b;&b;&b;&b;&b;\">]>"
                                   "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" "
                                   "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"><office:body><office:spreadsheet><table:table table:name=\"S\"><table:table-row><table:table-cell "
                                   "office:value-type=\"string\"><text:p>&c;</text:p></table:table-cell></table:table-row></table:table></office:spreadsheet></office:body></office:document-content>";
        Workbook wb;
        QString why;
        QVERIFY(!openIt(buildOds(content), &wb, &why));
        QVERIFY2(why.contains(QLatin1String("DOCTYPE")), qPrintable(why));
        Workbook flat;
        QVERIFY(!openBytes(content, QStringLiteral("flat.fods"), &flat, &why));
    }

    // A text of any length: spaces that stand for a billion, a cell of millions of characters, and an attribute larger than any piece of XML may be.
    void textsAreBounded()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(1) + row(cell(QStringLiteral("office:value-type=\"string\""), QStringLiteral("<text:p><text:s text:c=\"1000000000\"/>x</text:p>")))))), &wb, &why),
                 qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        const Cell *c = at(s, QStringLiteral("A1"));
        QVERIFY(c);
        QVERIFY(s.text(*c, wb.sharedStrings()).size() <= 10001);
        // a text of more characters than the limit says is refused with what came before it kept
        Limits limits;
        limits.stringChars = 1000;
        Workbook big;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(1) + row(floatCell(1)) + row(cell(QStringLiteral("office:value-type=\"string\""), QStringLiteral("<text:p>") + QString(5000, QLatin1Char('a')) + QStringLiteral("</text:p>")))))),
                                 &big, &why, limits), qPrintable(why));
        QVERIFY(!big.readSheet(0, &s));
        QVERIFY(s.error.contains(QLatin1String("longer")));
        QCOMPARE(shown(big, s, QStringLiteral("A1")), QStringLiteral("n:1"));
        // an attribute of 70 MB, larger than any piece of XML may be: the sheet that has it is refused when it is read
        Workbook attr;
        const QString huge = table(QStringLiteral("S"), columns(1) + row(floatCell(1)) + row(cell(QStringLiteral("office:value-type=\"string\" office:string-value=\"") + QString(70 << 20, QLatin1Char('b')) + QStringLiteral("\""))));
        QVERIFY2(openIt(buildOds(contentXml(huge)), &attr, &why), qPrintable(why));
        QVERIFY(!attr.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("longer")), qPrintable(s.error));
        QCOMPARE(shown(attr, s, QStringLiteral("A1")), QStringLiteral("n:1"));
    }

    // Styles and sheets without end: more of them than the limits say are refused in words.
    void countsOfStylesAndSheetsAreBounded()
    {
        QString styles;
        for (int i = 0; i < 200; ++i) styles += QStringLiteral("<style:style style:name=\"s%1\" style:family=\"table-cell\"><style:text-properties fo:font-size=\"%2pt\"/></style:style>").arg(i).arg(8 + i % 50 + i / 50);
        Limits limits;
        limits.cellXfs = 20;
        Workbook wb;
        QString why;
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1))), styles)), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("formats")), qPrintable(why));
        limits = Limits();
        limits.sheets = 3;
        QString tables;
        for (int i = 0; i < 5; ++i) tables += table(QStringLiteral("S%1").arg(i), row(floatCell(1)));
        QVERIFY(!openIt(buildOds(contentXml(tables)), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("sheets")), qPrintable(why));
        limits = Limits();
        limits.definedNames = 2;
        QString names = QStringLiteral("<table:named-expressions>");
        for (int i = 0; i < 5; ++i) names += QStringLiteral("<table:named-range table:name=\"n%1\" table:base-cell-address=\"$S.$A$1\" table:cell-range-address=\"$S.$A$1\"/>").arg(i);
        names += QStringLiteral("</table:named-expressions>");
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1))) + names)), &wb, &why, limits));
    }

    // A style that follows itself, or a chain of parents without end, ends.
    void styleChainsEnd()
    {
        QString styles = QStringLiteral("<style:style style:name=\"loop\" style:family=\"table-cell\" style:parent-style-name=\"loop\"/>"
                                        "<style:style style:name=\"a\" style:family=\"table-cell\" style:parent-style-name=\"b\"/><style:style style:name=\"b\" style:family=\"table-cell\" style:parent-style-name=\"a\"/>");
        for (int i = 0; i < 100; ++i) styles += QStringLiteral("<style:style style:name=\"c%1\" style:family=\"table-cell\" style:parent-style-name=\"c%2\"><style:text-properties fo:font-weight=\"bold\"/></style:style>").arg(i).arg(i + 1);
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(1) + row(floatCell(1, QStringLiteral("table:style-name=\"loop\"")))), styles)), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1"));
    }

    // A document that is cut off, or has an element that never closes, is an error in words with the cells before it kept.
    void damagedDocumentsAreErrors()
    {
        QByteArray content = contentXml(table(QStringLiteral("S"), columns(1) + row(floatCell(1)) + row(floatCell(2))));
        const QByteArray cut = content.left(content.indexOf("<table:table-row", content.indexOf("<table:table-row") + 5) + 20);
        Workbook wb;
        QString why;
        QVERIFY(!openIt(buildOds(cut), &wb, &why));
        QVERIFY(!why.isEmpty());
        QByteArray swapped = content;
        swapped.replace("</table:table-row>", "</table:table-cell>");
        Workbook wb2;
        if (openIt(buildOds(swapped), &wb2, &why)) {
            Sheet s;
            QVERIFY(!wb2.readSheet(0, &s));
            QVERIFY(!s.error.isEmpty());
        }
        // a part that is not XML at all
        QVERIFY(!openIt(buildOds(QByteArray("\x00\x01 not xml", 11)), &wb, &why));
        QVERIFY(!openIt(buildOds(QByteArray()), &wb, &why));
    }

    // A document whose content is larger than the program holds is refused before it is inflated (a stand-in for a bomb: the limit is lowered for the test).
    void aContentLargerThanTheLimitIsRefused()
    {
        QString rows;
        for (int i = 0; i < 2000; ++i) rows += row(floatCell(i));
        Limits limits;
        limits.odsContentBytes = 10000;
        Workbook wb;
        QString why;
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(1) + rows))), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("larger")), qPrintable(why));
    }

    // Formulas of any shape are converted, refused, or kept as values, and none takes more time than its length.
    void hostileFormulasInCells()
    {
        QString deep = QStringLiteral("of:=");
        for (int i = 0; i < 50000; ++i) deep += QLatin1Char('(');
        const QString r = row(cell(QStringLiteral("table:formula=\"") + deep + QStringLiteral("\" office:value-type=\"float\" office:value=\"1\"")) + cell(QStringLiteral("table:formula=\"of:=[.A\" office:value-type=\"float\" office:value=\"2\"")));
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), columns(2) + r))), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(shown(wb, s, QStringLiteral("B1")), QStringLiteral("n:2"));
        QCOMPARE(s.formulaCells, qint64(0));
        QCOMPARE(s.stats.unreadFormulas, qint64(2));
    }

    // Cells whose repeats add up past what a number holds: the count stops at the edge of the sheet, and the cell after them is not wrapped round to the first column.
    void repeatsThatAddUpPastANumberDoNotWrapRound()
    {
        QString cells;
        for (int i = 0; i < 4200; ++i) cells += floatCell(1, QStringLiteral("table:number-columns-repeated=\"1048576\"")).replace(QLatin1String("office:value=\"1\""), QLatin1String("office:value=\"0\""));
        cells += floatCell(7);
        Workbook wb;
        QString why;
        Limits limits;
        limits.cellsPerSheet = 1000000;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), row(cells)))), &wb, &why, limits), qPrintable(why));
        Sheet s;
        wb.readSheet(0, &s);
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:0"));   // (the first cell's, not the last one's)
        QVERIFY(s.cells.cellCount() <= 16384);
    }

    // Many cells in one row, and a long row of covered cells: only the cells that are there are made.
    void wideRowsAreCheap()
    {
        QString cells;
        for (int i = 0; i < 20000; ++i) cells += QStringLiteral("<table:table-cell/>");
        cells += floatCell(1);
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), row(cells)))), &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(s.cells.cellCount(), qint64(0));   // (the cell is in column 20,001, past the last column of a sheet)
    }
};

JP_TEST_CLASS(SheetOdsHostileTests)
#include "tst_sheet_odshostile.moc"
