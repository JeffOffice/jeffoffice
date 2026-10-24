// Tests of the printing records of the old binary workbook format: the page setup, header and footer, margins, print options, breaks, and the print area and titles
// (built-in names). Every workbook is built in memory by xlshelpers.
#include "xlshelpers.h"

#include "sheet/xlsxsave.h"

namespace {

QByteArray setup(int paper, int scale, int pageStart, int fitW, int fitH, int grbit, double header = 0.3, double footer = 0.3)
{
    return rec(0x00A1, W().u16(paper).u16(scale).u16(pageStart).u16(fitW).u16(fitH).u16(grbit).u16(600).u16(600).f64(header).f64(footer).u16(1));
}

QByteArray text8(quint16 id, const QString &s) { return rec(id, W().str8(s, 2, true)); }

QByteArray margin(quint16 id, double v) { return rec(id, W().f64(v)); }

QByteArray hfExtra(int flags, const QString &headerEven, const QString &footerEven, const QString &headerFirst, const QString &footerFirst)
{
    W w;
    w.u16(0x089C).u16(0).raw(QByteArray(8, 0)).raw(QByteArray(16, 0)).u16(flags).u16(headerEven.size()).u16(footerEven.size()).u16(headerFirst.size()).u16(footerFirst.size());
    for (const QString &s : {headerEven, footerEven, headerFirst, footerFirst})
        if (!s.isEmpty()) w.u8(0).raw(s.toLatin1());
    return rec(0x089C, w);
}

Sheet readFirst(const Book &b, QString *why = nullptr)
{
    Workbook wb;
    Sheet s;
    if (!openBook(b, &wb, why) || !wb.readSheet(0, &s)) return Sheet();
    return s;
}

} // namespace

class SheetXlsPrintTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The page setup, the header and footer, the margins, the options, the fit-to-page flag, and the breaks of a sheet.
    void thePrintingRecordsOfASheetAreRead()
    {
        Book b;
        QByteArray s = number(0, 0, 1);
        s += rec(0x0081, W().u16(0x0100 | 0x0001));   // fit to pages
        // left to right, landscape (bit 1 clear), black and white, draft, notes, first page number used, notes at the end, errors as dashes
        s += setup(9, 80, 5, 2, 3, 0x0001 | 0x0008 | 0x0010 | 0x0020 | 0x0080 | 0x0200 | (2 << 10), 0.4, 0.5);
        s += text8(0x0014, QStringLiteral("&CHead &A"));
        s += text8(0x0015, QStringLiteral("&LPage &P"));
        s += margin(0x0026, 0.5) + margin(0x0027, 0.6) + margin(0x0028, 0.7) + margin(0x0029, 0.8);
        s += rec(0x002A, W().u16(1)) + rec(0x002B, W().u16(1)) + rec(0x0083, W().u16(1)) + rec(0x0084, W().u16(0));
        s += rec(0x001B, W().u16(2).u16(20).u16(0).u16(255).u16(40).u16(0).u16(255));
        s += rec(0x001A, W().u16(1).u16(4).u16(0).u16(65535));
        s += hfExtra(0x0001 | 0x0002 | 0x0004, QStringLiteral("even h"), QStringLiteral("even f"), QStringLiteral("first h"), QString());
        b.sheet(QStringLiteral("S"), s);
        const Sheet sheet = readFirst(b);
        const PageSetup &p = sheet.page;
        QVERIFY(p.fitToPage);
        QCOMPARE(p.paperSize, 9);
        QCOMPARE(p.scale, 80);
        QCOMPARE(p.firstPageNumber, 5);
        QVERIFY(p.useFirstPageNumber);
        QCOMPARE(p.fitToWidth, 2);
        QCOMPARE(p.fitToHeight, 3);
        QVERIFY(p.pageOrder == PageOrder::OverThenDown);
        QVERIFY(p.orientation == PageOrientation::Landscape);
        QVERIFY(p.blackAndWhite && p.draft);
        QVERIFY(p.comments == PrintComments::AtEnd);
        QVERIFY(p.errors == PrintErrors::Dash);
        QCOMPARE(p.margins.header, 0.4);
        QCOMPARE(p.margins.footer, 0.5);
        QCOMPARE(p.margins.left, 0.5);
        QCOMPARE(p.margins.right, 0.6);
        QCOMPARE(p.margins.top, 0.7);
        QCOMPARE(p.margins.bottom, 0.8);
        QCOMPARE(p.headerFooter.oddHeader, QStringLiteral("&CHead &A"));
        QCOMPARE(p.headerFooter.oddFooter, QStringLiteral("&LPage &P"));
        QVERIFY(p.gridLines && p.headings && p.centerHorizontally && !p.centerVertically);
        QCOMPARE(p.rowBreaks.size(), 2);
        QCOMPARE(p.rowBreaks[0].before, 20u);
        QCOMPARE(p.rowBreaks[1].before, 40u);
        QVERIFY(p.rowBreaks[0].manual);
        QCOMPARE(p.colBreaks.size(), 1);
        QCOMPARE(p.colBreaks[0].before, 4u);
        QVERIFY(p.headerFooter.differentOddEven && p.headerFooter.differentFirst && p.headerFooter.scaleWithDoc && !p.headerFooter.alignWithMargins);
        QCOMPARE(p.headerFooter.evenHeader, QStringLiteral("even h"));
        QCOMPARE(p.headerFooter.evenFooter, QStringLiteral("even f"));
        QCOMPARE(p.headerFooter.firstHeader, QStringLiteral("first h"));
        QVERIFY(p.headerFooter.firstFooter.isEmpty());
    }

    // The orientation bit means portrait when it is set; a sheet whose printer settings are not valid has the paper and scale of no printer; notes as displayed.
    void orientationAndInvalidPrinterSettings()
    {
        Book b;
        b.sheet(QStringLiteral("Portrait"), number(0, 0, 1) + setup(5, 70, 1, 1, 1, 0x0002 | 0x0020));
        b.sheet(QStringLiteral("NoPrinter"), number(0, 0, 1) + setup(9, 70, 1, 1, 1, 0x0004));
        b.sheet(QStringLiteral("NoOrientation"), number(0, 0, 1) + setup(1, 100, 1, 1, 1, 0x0040));
        Workbook wb;
        QVERIFY(openBook(b, &wb));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY(s.page.orientation == PageOrientation::Portrait);
        QCOMPARE(s.page.paperSize, 5);
        QVERIFY(s.page.comments == PrintComments::AsDisplayed);
        QVERIFY(wb.readSheet(1, &s));
        QCOMPARE(s.page.paperSize, 1);
        QCOMPARE(s.page.scale, 100);
        QVERIFY(wb.readSheet(2, &s));
        QVERIFY(s.page.orientation == PageOrientation::Default);
    }

    // The print area and the titles are built-in names of the sheet (the numbers 6 and 7).
    void thePrintAreaAndTitlesAreNames()
    {
        Book b;
        const QByteArray after = supbookInternal(1) + externSheet({{0, 0, 0}})
                                 + name(QString(), tArea3d(0, 0, 0, 9, 4, false, false), 1, 6)
                                 + name(QString(), tArea3d(0, 0, 1, 65535, 1, false, false) + tArea3d(0, 0, 0, 1, 255, false, false) + tOp(0x10), 1, 7);
        b.after(after);
        b.sheet(QStringLiteral("Sheet1"), number(0, 0, 1));
        const Sheet s = readFirst(b);
        QCOMPARE(s.page.printArea.size(), 1);
        QCOMPARE(rangeName(s.page.printArea[0]), QStringLiteral("A1:E10"));
        QVERIFY(s.page.titles.hasRows && s.page.titles.hasCols);
        QCOMPARE(s.page.titles.col1, 1u);
        QCOMPARE(s.page.titles.col2, 1u);
        QCOMPARE(s.page.titles.row2, 1u);
    }

    // A record that claims more than it holds, and counts past the bounds, read what is there.
    void damagedPrintingRecordsAreBounded()
    {
        Book b;
        QByteArray s = number(0, 0, 1);
        s += rec(0x001B, W().u16(60000).u16(7).u16(0).u16(255));                  // a count with one break in it
        s += rec(0x0014, W().u16(5000).u8(0).raw(QByteArray(10, 'x')));           // a string longer than the record
        s += rec(0x00A1, W().u16(9));                                              // a record cut short
        s += rec(0x0026, W().u8(1));                                               // a margin cut short
        s += rec(0x001A, QByteArray());
        QByteArray many = W().u16(30000);
        for (int i = 1; i <= 6000; ++i) many += W().u16(i).u16(0).u16(255).bytes();
        s += rec(0x001B, many.left(8000));
        b.sheet(QStringLiteral("S"), s);
        const Sheet sheet = readFirst(b);
        QVERIFY(sheet.page.rowBreaks.size() >= 1);
        QVERIFY(sheet.page.rowBreaks.size() <= kMaxPageBreaks);
        QVERIFY(sheet.page.headerFooter.oddHeader.size() <= kMaxHeaderFooterChars);
        QCOMPARE(sheet.page.paperSize, 1);   // (the cut-short record changed nothing)
        QCOMPARE(sheet.page.margins.left, 0.7);
    }

    // A workbook read from the old form is saved as the XML form with its sheets' page setup and print area, and reads back the same.
    void aConvertedWorkbookKeepsItsPageSetup()
    {
        Book b;
        QByteArray s = number(0, 0, 1) + rec(0x0081, W().u16(0x0100)) + setup(9, 80, 1, 2, 0, 0x0001 | 0x0008) + text8(0x0014, QStringLiteral("&CHead & more"));
        s += margin(0x0026, 0.5) + rec(0x002B, W().u16(1)) + rec(0x001B, W().u16(1).u16(20).u16(0).u16(255)) + rec(0x001A, W().u16(1).u16(4).u16(0).u16(65535));
        b.after(supbookInternal(1) + externSheet({{0, 0, 0}}) + name(QString(), tArea3d(0, 0, 0, 9, 4, false, false), 1, 6));
        b.sheet(QStringLiteral("Data"), s);
        Workbook wb;
        QVERIFY(openBook(b, &wb));
        Sheet first;
        QVERIFY(wb.readSheet(0, &first));
        QByteArray out;
        QString why;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &out, &why), qPrintable(why));
        Package package;
        QVERIFY2(package.readLazy(out, &why), qPrintable(why));
        Workbook again;
        QVERIFY2(again.open(package, &why), qPrintable(why));
        Sheet back;
        QVERIFY(again.readSheet(0, &back));
        QCOMPARE(back.page, first.page);
        QVERIFY(back.page.fitToPage);
        QCOMPARE(back.page.rowBreaks.size(), 1);
        QCOMPARE(back.page.printArea.size(), 1);
        QCOMPARE(back.page.headerFooter.oddHeader, QStringLiteral("&CHead & more"));
    }

    // BIFF5 writes the header and footer as bytes.
    void aBiff5SheetHasItsHeaderAndFooter()
    {
        Book b(5);
        b.sheet(QStringLiteral("S"), number(0, 0, 1) + rec(0x0014, W().u8(7).raw("&CTitle")) + rec(0x0015, W().u8(5).raw("&RP&P")) + setup(1, 90, 1, 1, 1, 0x0002));
        const Sheet s = readFirst(b);
        QCOMPARE(s.page.headerFooter.oddHeader, QStringLiteral("&CTitle"));
        QCOMPARE(s.page.headerFooter.oddFooter, QStringLiteral("&RP&P"));
        QCOMPARE(s.page.scale, 90);
    }
};

JP_TEST_CLASS(SheetXlsPrintTests)
#include "tst_sheet_xlsprint.moc"
