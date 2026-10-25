// Tests of what a sheet says about printing: the page setup, margins, print options, header and footer, and breaks read from a sheet part, the print area and titles read from the
// names, the paper sizes, and the bounds on all of it. Every file is built here.
#include "sheethelpers.h"

#include "sheet/pagesetup.h"

namespace {

QByteArray sheetWith(const QString &head, const QString &after) { return worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), after, head); }

bool readPage(const QString &head, const QString &after, PageSetup *out)
{
    Sheet sheet;
    if (!readSheetXml(sheetWith(head, after), &sheet)) return false;
    *out = sheet.page;
    return true;
}

} // namespace

class SheetPageSetupTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Every attribute of the page setup, the margins, the print options, and the header and footer is read.
    void thePageSettingsOfASheetAreRead()
    {
        PageSetup p;
        QVERIFY(readPage(QStringLiteral("<sheetPr><pageSetUpPr fitToPage=\"1\"/></sheetPr>"),
                         QStringLiteral("<printOptions horizontalCentered=\"1\" verticalCentered=\"true\" headings=\"1\" gridLines=\"1\" gridLinesSet=\"1\"/>"
                                        "<pageMargins left=\"0.5\" right=\"0.6\" top=\"0.7\" bottom=\"0.8\" header=\"0.2\" footer=\"0.25\"/>"
                                        "<pageSetup paperSize=\"9\" scale=\"85\" firstPageNumber=\"3\" fitToWidth=\"2\" fitToHeight=\"0\" pageOrder=\"overThenDown\" orientation=\"landscape\" "
                                        "usePrinterDefaults=\"0\" blackAndWhite=\"1\" draft=\"1\" cellComments=\"atEnd\" useFirstPageNumber=\"1\" errors=\"blank\" horizontalDpi=\"600\"/>"
                                        "<headerFooter differentOddEven=\"1\" differentFirst=\"1\" scaleWithDoc=\"0\" alignWithMargins=\"0\"><oddHeader>&amp;CReport &amp;A</oddHeader>"
                                        "<oddFooter>&amp;LPage &amp;P of &amp;N</oddFooter><evenHeader>even h</evenHeader><evenFooter>even f</evenFooter><firstHeader>first h</firstHeader>"
                                        "<firstFooter>first f</firstFooter></headerFooter>"),
                         &p));
        QVERIFY(p.fitToPage);
        QVERIFY(p.centerHorizontally && p.centerVertically && p.headings && p.gridLines);
        QCOMPARE(p.margins.left, 0.5);
        QCOMPARE(p.margins.right, 0.6);
        QCOMPARE(p.margins.top, 0.7);
        QCOMPARE(p.margins.bottom, 0.8);
        QCOMPARE(p.margins.header, 0.2);
        QCOMPARE(p.margins.footer, 0.25);
        QCOMPARE(p.paperSize, 9);
        QCOMPARE(p.scale, 85);
        QCOMPARE(p.firstPageNumber, 3);
        QVERIFY(p.useFirstPageNumber);
        QCOMPARE(p.fitToWidth, 2);
        QCOMPARE(p.fitToHeight, 0);
        QVERIFY(p.pageOrder == PageOrder::OverThenDown);
        QVERIFY(p.orientation == PageOrientation::Landscape);
        QVERIFY(p.blackAndWhite && p.draft);
        QVERIFY(p.comments == PrintComments::AtEnd);
        QVERIFY(p.errors == PrintErrors::Blank);
        QVERIFY(p.headerFooter.differentOddEven && p.headerFooter.differentFirst);
        QVERIFY(!p.headerFooter.scaleWithDoc);
        QVERIFY(!p.headerFooter.alignWithMargins);
        QCOMPARE(p.headerFooter.oddHeader, QStringLiteral("&CReport &A"));
        QCOMPARE(p.headerFooter.oddFooter, QStringLiteral("&LPage &P of &N"));
        QCOMPARE(p.headerFooter.evenHeader, QStringLiteral("even h"));
        QCOMPARE(p.headerFooter.evenFooter, QStringLiteral("even f"));
        QCOMPARE(p.headerFooter.firstHeader, QStringLiteral("first h"));
        QCOMPARE(p.headerFooter.firstFooter, QStringLiteral("first f"));
    }

    // A sheet that says nothing prints with the format's defaults.
    void aSheetWithNoPageSettingsHasTheDefaults()
    {
        PageSetup p;
        QVERIFY(readPage(QString(), QString(), &p));
        QCOMPARE(p, PageSetup());
        QCOMPARE(p.paperSize, 1);
        QCOMPARE(p.scale, 100);
        QVERIFY(!p.fitToPage);
        QCOMPARE(p.margins.left, 0.7);
        QCOMPARE(p.margins.top, 0.75);
        QCOMPARE(p.margins.header, 0.3);
        QVERIFY(p.headerFooter.scaleWithDoc && p.headerFooter.alignWithMargins);
        QVERIFY(p.orientation == PageOrientation::Default);
        QVERIFY(p.rowBreaks.isEmpty() && p.colBreaks.isEmpty());
        QVERIFY(!p.hasPrintArea());
    }

    // The breaks, in the order of the file; the page after a break begins at the row or column of the number the file gives, counted from 0.
    void theBreaksAreRead()
    {
        PageSetup p;
        QVERIFY(readPage(QString(), QStringLiteral("<rowBreaks count=\"3\" manualBreakCount=\"2\"><brk id=\"20\" max=\"16383\" man=\"1\"/><brk id=\"40\" max=\"16383\"/><brk id=\"60\" man=\"0\"/></rowBreaks>"
                                                   "<colBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"4\" max=\"1048575\" man=\"1\"/></colBreaks>"),
                         &p));
        QCOMPARE(p.rowBreaks.size(), 3);
        QCOMPARE(p.rowBreaks[0].before, 20u);
        QCOMPARE(p.rowBreaks[0].max, 16383u);
        QVERIFY(p.rowBreaks[0].manual);
        QVERIFY(!p.rowBreaks[2].manual);
        QCOMPARE(p.colBreaks.size(), 1);
        QCOMPARE(p.colBreaks[0].before, 4u);
        QCOMPARE(p.colBreaks[0].max, 1048575u);
    }

    // A break the file lists twice is one; a break at 0, or past the sheet, is not one; and no sheet keeps more breaks than the bound.
    void breaksAreBounded()
    {
        PageSetup p;
        QString list;
        for (int i = 0; i < kMaxPageBreaks + 500; ++i) list += QStringLiteral("<brk id=\"%1\" max=\"16383\" man=\"1\"/>").arg(i + 1);
        QVERIFY(readPage(QString(), QStringLiteral("<rowBreaks>") + list + QStringLiteral("</rowBreaks><colBreaks><brk id=\"0\"/><brk id=\"5\"/><brk id=\"5\"/><brk id=\"99999\"/><brk id=\"x\"/></colBreaks>"), &p));
        QCOMPARE(p.rowBreaks.size(), kMaxPageBreaks);
        QCOMPARE(p.colBreaks.size(), 1);
        QCOMPARE(p.colBreaks[0].before, 5u);
        // (a break past the last row is not one)
        PageSetup q;
        QVERIFY(readPage(QString(), QStringLiteral("<rowBreaks><brk id=\"1048576\" max=\"16383\" man=\"1\"/><brk id=\"1048575\" max=\"16383\" man=\"1\"/></rowBreaks>"), &q));
        QCOMPARE(q.rowBreaks.size(), 1);
        QCOMPARE(q.rowBreaks[0].before, 1048575u);
    }

    // A file that repeats one break a million times is not read a million times: the elements looked at are counted, and past the bound they are passed by.
    void repeatedBreaksAreNotAllLookedAt()
    {
        PageSetup p;
        QString list = QStringLiteral("<brk id=\"7\" max=\"16383\" man=\"1\"/>");
        QString many;
        const int n = 1000000;
        many.reserve(n * 36);
        for (int i = 0; i < n; ++i) many += QStringLiteral("<brk id=\"7\" max=\"16383\" man=\"1\"/>");
        QVERIFY(readPage(QString(), QStringLiteral("<rowBreaks>") + list + many + QStringLiteral("<brk id=\"9\"/></rowBreaks>"), &p));
        QCOMPARE(p.rowBreaks.size(), 1);   // (the one at 9 comes after the bound: it is not read)
        QCOMPARE(p.rowBreakElements, kMaxBreakElements);
        QCOMPARE(p.colBreakElements, 0);
    }

    // What a file gives is held to what the format allows: the scale 10 to 400, the pages to fit 0 to 32,767, a margin 0 to 100 inches, and the texts to their bound.
    void numbersAndTextsAreBounded()
    {
        PageSetup p;
        QVERIFY(readPage(QString(), QStringLiteral("<pageMargins left=\"-3\" right=\"1e9\" top=\"abc\" bottom=\"nan\" header=\"\" footer=\"inf\"/><pageSetup scale=\"5000\" fitToWidth=\"99999\" fitToHeight=\"-4\" firstPageNumber=\"-9\" paperSize=\"-1\"/>"), &p));
        QCOMPARE(p.margins.left, 0.0);
        QCOMPARE(p.margins.right, kMaxMarginInches);
        QCOMPARE(p.margins.top, 0.75);      // (not a number: as it was)
        QCOMPARE(p.margins.bottom, 0.75);
        QCOMPARE(p.margins.header, 0.3);
        QCOMPARE(p.margins.footer, 0.3);
        QCOMPARE(p.scale, kMaxPageScale);
        QCOMPARE(p.fitToWidth, kMaxFitPages);
        QCOMPARE(p.fitToHeight, 0);
        QCOMPARE(p.firstPageNumber, 1);
        QCOMPARE(p.paperSize, 1);
        PageSetup low;
        QVERIFY(readPage(QString(), QStringLiteral("<pageSetup scale=\"1\"/>"), &low));
        QCOMPARE(low.scale, kMinPageScale);
        const QString huge(kMaxHeaderFooterChars * 3, QLatin1Char('x'));
        PageSetup h;
        QVERIFY(readPage(QString(), QStringLiteral("<headerFooter><oddHeader>") + huge + QStringLiteral("</oddHeader><oddFooter>") + huge + QStringLiteral("</oddFooter></headerFooter>"), &h));
        QCOMPARE(h.headerFooter.oddHeader.size(), kMaxHeaderFooterChars);
        QCOMPARE(h.headerFooter.oddFooter.size(), kMaxHeaderFooterChars);
    }

    // A paper size given in measures (a paper the format has no number for) is kept in points.
    void aPaperOfItsOwnSizeIsRead()
    {
        PageSetup p;
        QVERIFY(readPage(QString(), QStringLiteral("<pageSetup paperSize=\"0\" paperWidth=\"200mm\" paperHeight=\"10in\"/>"), &p));
        QVERIFY(qAbs(p.paperWidth - 566.929) < 0.01);
        QCOMPARE(p.paperHeight, 720.0);
        PageSetup bad;
        QVERIFY(readPage(QString(), QStringLiteral("<pageSetup paperWidth=\"2mm\" paperHeight=\"90000in\"/>"), &bad));
        QCOMPARE(bad.paperWidth, 0.0);
        QCOMPARE(bad.paperHeight, 0.0);
    }

    // The print area and the titles come from the names of the workbook that belong to the sheet; a name of another sheet, or one that is not cells, is passed by.
    void thePrintAreaAndTitlesAreReadFromTheNames()
    {
        const QByteArray s = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"));
        const QByteArray book = buildXlsx({s, s}, QByteArray(), QString(), QString(), QByteArray(), QStringList({"Data", "My Sheet"}),
                                          QStringLiteral("<definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">Data!$A$1:$D$20,Data!$F$3:$G$9</definedName>"
                                                         "<definedName name=\"_xlnm.Print_Titles\" localSheetId=\"0\">Data!$A:$B,Data!$1:$2</definedName>"
                                                         "<definedName name=\"_xlnm.Print_Area\" localSheetId=\"1\">'My Sheet'!$B$2:$C$3</definedName>"
                                                         "<definedName name=\"_xlnm.Print_Titles\" localSheetId=\"1\">Data!$1:$1</definedName>"));
        Package package;
        QVERIFY(package.readLazy(book));
        Workbook wb;
        QVERIFY(wb.open(package));
        Sheet first, second;
        QVERIFY(wb.readSheet(0, &first));
        QVERIFY(wb.readSheet(1, &second));
        QCOMPARE(first.page.printArea.size(), 2);
        QCOMPARE(rangeName(first.page.printArea[0]), QStringLiteral("A1:D20"));
        QCOMPARE(rangeName(first.page.printArea[1]), QStringLiteral("F3:G9"));
        QVERIFY(first.page.titles.hasRows && first.page.titles.hasCols);
        QCOMPARE(first.page.titles.row1, 0u);
        QCOMPARE(first.page.titles.row2, 1u);
        QCOMPARE(first.page.titles.col1, 0u);
        QCOMPARE(first.page.titles.col2, 1u);
        QCOMPARE(second.page.printArea.size(), 1);
        QCOMPARE(rangeName(second.page.printArea[0]), QStringLiteral("B2:C3"));
        QVERIFY(!second.page.titles.hasRows && !second.page.titles.hasCols);   // (its titles name the other sheet)
    }

    // The text of a name: whole rows and columns, a single cell, a quoted sheet name, and what is not cells.
    void theTextOfTheNamesIsReadAndWritten()
    {
        PageSetup p;
        readPrintAreaText(QStringLiteral("'It''s'!$C$4,'It''s'!A:B,'It''s'!$5:$6,Other!A1,#REF!"), QStringLiteral("It's"), &p);
        QCOMPARE(p.printArea.size(), 3);
        QCOMPARE(rangeName(p.printArea[0]), QStringLiteral("C4"));
        QCOMPARE(p.printArea[1].row1, 0u);
        QCOMPARE(p.printArea[1].row2, kMaxRows - 1);
        QCOMPARE(p.printArea[1].col2, 1u);
        QCOMPARE(p.printArea[2].col2, kMaxColumns - 1);
        QCOMPARE(p.printArea[2].row1, 4u);
        QCOMPARE(printAreaText(p, QStringLiteral("It's")), QStringLiteral("'It''s'!$C$4,'It''s'!$A:$B,'It''s'!$5:$6"));
        PageSetup none;
        readPrintAreaText(QStringLiteral("#REF!"), QStringLiteral("Data"), &none);
        QVERIFY(!none.hasPrintArea());
        QCOMPARE(printAreaText(none, QStringLiteral("Data")), QString());
        PageSetup t;
        readPrintTitlesText(QStringLiteral("Data!$3:$4"), QStringLiteral("Data"), &t);
        QVERIFY(t.titles.hasRows && !t.titles.hasCols);
        QCOMPARE(printTitlesText(t, QStringLiteral("Data")), QStringLiteral("Data!$3:$4"));
        t.titles.hasCols = true;
        t.titles.col1 = 0;
        t.titles.col2 = 2;
        QCOMPARE(printTitlesText(t, QStringLiteral("Data")), QStringLiteral("Data!$A:$C,Data!$3:$4"));
        // more areas than the bound are not kept
        QString many;
        for (int i = 0; i < kMaxPrintAreas + 50; ++i) many += QStringLiteral("Data!$A$%1,").arg(i + 1);
        PageSetup m;
        readPrintAreaText(many, QStringLiteral("Data"), &m);
        QCOMPARE(m.printArea.size(), kMaxPrintAreas);
    }

    // The paper: the sizes of the numbers the format has, in points; a number it has not is letter; and landscape turns the page.
    void thePaperSizesAreInPoints()
    {
        double w = 0, h = 0;
        paperSizePoints(1, &w, &h);
        QCOMPARE(w, 612.0);
        QCOMPARE(h, 792.0);
        paperSizePoints(9, &w, &h);
        QVERIFY(qAbs(w - 595.276) < 0.01);
        QVERIFY(qAbs(h - 841.890) < 0.01);
        paperSizePoints(5, &w, &h);
        QCOMPARE(h, 1008.0);
        paperSizePoints(8, &w, &h);
        QVERIFY(qAbs(w - 841.890) < 0.01);
        paperSizePoints(3, &w, &h);
        QCOMPARE(w, 792.0);
        QCOMPARE(h, 1224.0);
        QVERIFY(isKnownPaperSize(9));
        QVERIFY(!isKnownPaperSize(9999));
        paperSizePoints(9999, &w, &h);
        QCOMPARE(w, 612.0);
        QCOMPARE(paperSizeName(9), QStringLiteral("A4"));
        QCOMPARE(paperSizeName(1), QStringLiteral("Letter"));
        QVERIFY(paperSizeName(9999).isEmpty());
        QVERIFY(commonPaperSizes().contains(1) && commonPaperSizes().contains(9) && commonPaperSizes().contains(5));
        PageSetup p;
        p.paperSize = 9;
        p.orientation = PageOrientation::Landscape;
        pageSizePoints(p, &w, &h);
        QVERIFY(w > h);
        QVERIFY(qAbs(w - 841.890) < 0.01);
        QVERIFY(isLandscape(p));
        p.orientation = PageOrientation::Default;
        QVERIFY(!isLandscape(p));
        // a paper of its own size, when the file gave one
        p.paperSize = 0;
        p.paperWidth = 300;
        p.paperHeight = 500;
        pageSizePoints(p, &w, &h);
        QCOMPARE(w, 300.0);
        QCOMPARE(h, 500.0);
    }
};

JP_TEST_CLASS(SheetPageSetupTests)
#include "tst_sheet_pagesetup.moc"
