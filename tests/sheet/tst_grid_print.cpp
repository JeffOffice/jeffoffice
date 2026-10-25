// Tests of cutting a sheet into pages (libs/shell/grid/printlayout.h): the sizes in points, the paper and margins, the part of the sheet that prints, the scale and the fit to pages,
// the manual and the automatic breaks, hidden rows and columns, the titles, the order of the pages, and the bounds. Every sheet is built here; the counts are work done, never a time.
#include "sheethelpers.h"

#include "grid/gridsheet.h"
#include "grid/printlayout.h"

using namespace jp::grid;

namespace {

Cell numberAt(quint32 row, quint32 col, quint32 style = 0)
{
    Cell c;
    c.row = row;
    c.col = quint16(col);
    c.type = CellType::Number;
    c.number = 1;
    c.style = style;
    return c;
}

// A sheet with a number in the cell at each corner of the block A1:(rows, cols).
struct Fixture {
    Sheet sheet;
    Styles styles;
    Axis rows{1048576, 20}, cols{16384, 64};   // 20 px (15 points) by 64 px (48 points): the usual cell
    PrintRequest request;

    explicit Fixture(quint32 lastRow = 99, quint32 lastCol = 9)
    {
        sheet.cells.add(numberAt(lastRow, lastCol));
        sheet.cells.finish();
        styles.fonts = {FontSpec()};
        styles.fills = {FillSpec(), FillSpec()};
        styles.borders = {BorderSpec()};
        styles.formats = {CellFormat()};
        styles.cellXfs = {0};
        request.sheet = &sheet;
        request.styles = &styles;
        request.rows = &rows;
        request.cols = &cols;
    }
    SheetPrint run() { return paginate(request); }
};

// Letter paper upright with the format's usual margins holds 681.6 by 912 pixels of sheet at 100%: 10 columns of 64 and 45 rows of 20.

QString rowsOf(const SheetPrint &p)
{
    QStringList l;
    for (const PageCut &c : p.pages) l << QStringLiteral("%1-%2").arg(c.row1).arg(c.row2);
    return l.join(QLatin1Char(' '));
}

QString colsOf(const SheetPrint &p)
{
    QStringList l;
    for (const PageCut &c : p.pages) l << QStringLiteral("%1-%2").arg(c.col1).arg(c.col2);
    return l.join(QLatin1Char(' '));
}

} // namespace

class GridPrintTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A width in characters is a number of pixels (cut, as the format says) and a pixel is three quarters of a point: values that are known.
    void sizesBecomePointsAsTheGridMakesThem()
    {
        // the usual column (8.43 characters, 9.140625 in the file) is 64 pixels, 48 points; a width of 13 characters is 96 pixels, 72 points; the digit being 7 pixels wide
        QCOMPARE(columnPoints(9.140625, 7), 48.0);
        QCOMPARE(columnPoints(13.7109375, 7), 72.0);
        QCOMPARE(columnPoints(0, 7), 0.0);
        // with a wider digit (a larger default font): 11 pixels wide, a width of 9.7109375 is 107 pixels (80.25 points)
        QCOMPARE(columnPoints(9.7109375, 11), 80.25);
        // rows: 15 points is 20 pixels; 12.75 is 17 exactly; 14.4 becomes 19 pixels, 14.25 points
        QCOMPARE(rowPoints(15), 15.0);
        QCOMPARE(rowPoints(12.75), 12.75);
        QCOMPARE(rowPoints(14.4), 14.25);
        QCOMPARE(rowPoints(0), 0.0);
        QCOMPARE(rowPoints(409.5), 409.5);
        // the same numbers as the grid's own conversions
        QCOMPARE(columnPoints(9.140625, 7), GridSheet::pointsForPixels(GridSheet::columnPixels(9.140625, 7)));
        QCOMPARE(rowPoints(14.4), GridSheet::pointsForPixels(GridSheet::rowPixels(14.4)));
    }

    // On paper a column's width in characters is counted in the widest digit in printer pixels (600 to the inch), which is a few percent wider than the screen's 7 pixels.
    void theDigitAndTheColumnsAreWiderOnPaperThanOnTheScreen()
    {
        Styles calibri;
        calibri.fonts = {FontSpec()};
        calibri.fonts[0].name = QStringLiteral("Calibri");
        calibri.fonts[0].size = 11;
        QCOMPARE(printDigitPixels(calibri), 46);
        Styles arial = calibri;
        arial.fonts[0].name = QStringLiteral("Arial");
        arial.fonts[0].size = 10;
        QCOMPARE(printDigitPixels(arial), 46);
        // the same digit at 96 to the inch is the 7 pixels of the screen
        QCOMPARE(printDigitPixels(calibri, 96), 7);
        // the usual column (9.140625 in the file) is 420 printer pixels, 50.4 points (the screen has 48), and 13.7109375 is 631, 75.72 points (the screen 72)
        QCOMPARE(printColumnPoints(9.140625, 46), 420 * 72.0 / 600);
        QCOMPARE(printColumnPoints(13.7109375, 46), 631 * 72.0 / 600);
        QVERIFY(qFuzzyCompare(printColumnPoints(9.140625, 46), 50.4));
        QVERIFY(qFuzzyCompare(printColumnPoints(13.7109375, 46), 75.72));
        // at the screen's resolution the same call gives the screen's 48 and 72
        QCOMPARE(printColumnPoints(9.140625, 7, 96), columnPoints(9.140625, 7));
        QCOMPARE(printColumnPoints(13.7109375, 7, 96), columnPoints(13.7109375, 7));
        // nothing for no width, and nothing for a digit that is not there
        QCOMPARE(printColumnPoints(0, 46), 0.0);
        QCOMPARE(printColumnPoints(-3, 46), 0.0);
        QCOMPARE(printColumnPoints(9.140625, 0), 0.0);
    }

    // The columns of a sheet on paper: the default, the runs the file gives, the hidden ones, in the grid's pixels (three quarters of a point).
    void aSheetsColumnsAreSizedForPaper()
    {
        Sheet sheet;
        Styles styles;
        styles.fonts = {FontSpec()};
        styles.fonts[0].name = QStringLiteral("Calibri");
        styles.fonts[0].size = 11;
        sheet.format.defaultColWidth = 9.140625;
        ColumnSpan wide;
        wide.first = 2;
        wide.last = 3;
        wide.width = 13.7109375;
        ColumnSpan hidden;
        hidden.first = 5;
        hidden.last = 5;
        hidden.width = 9.140625;
        hidden.hidden = true;
        ColumnSpan plain;   // a run that says nothing of its width keeps the default
        plain.first = 7;
        plain.last = 7;
        sheet.columns = {hidden, wide, plain};   // (out of order, as a file may have them)
        const Axis cols = printColumnAxis(sheet, styles);
        QCOMPARE(cols.size(0), 67);      // 50.4 points
        QCOMPARE(cols.size(1), 67);
        QCOMPARE(cols.size(2), 101);     // 75.72 points
        QCOMPARE(cols.size(3), 101);
        QCOMPARE(cols.size(4), 67);
        QCOMPARE(cols.size(5), 0);
        QCOMPARE(cols.size(7), 67);
        QCOMPARE(cols.position(4), 67 + 67 + 101 + 101);
        // the screen's own columns for the same sheet are narrower
        QVERIFY(cols.size(0) > 64);
        // a width the file does not give is made from the base width in characters, in whole eighths of the screen's pixels
        Sheet bare;
        const Axis base = printColumnAxis(bare, styles);
        QVERIFY(base.size(0) > 0);
        QCOMPARE(base.size(0), base.size(100));
        QCOMPARE(base.count(), qint64(jp::office::sheet::kMaxColumns));
    }

    // The rows on paper are as tall as the file says, to a small part of a pixel (the screen's rows are held to whole pixels: 14.4 points is 19 of them, 14.25 points).
    void theRowsOnPaperKeepTheirHeights()
    {
        Sheet sheet;
        sheet.format.defaultRowHeight = 15;
        RowSpec tall;
        tall.row = 1;
        tall.height = 14.4;
        RowSpec hidden;
        hidden.row = 3;
        hidden.height = 20;
        hidden.hidden = true;
        RowSpec again;   // the last of two for a row is the one that counts
        again.row = 1;
        again.height = 30;
        sheet.rowSpecs = {hidden, tall, again};
        const Axis whole = printRowAxis(sheet);
        QCOMPARE(whole.size(0), 20);
        QCOMPARE(whole.size(1), 40);
        QCOMPARE(whole.size(3), 0);
        QCOMPARE(whole.size(2), 20);
        // eighths of a pixel: 15 points is 160, 14.4 is 153.6 (154)
        sheet.rowSpecs = {tall};
        const Axis fine = printRowAxis(sheet, 8);
        QCOMPARE(fine.size(0), 160);
        QCOMPARE(fine.size(1), 154);
        QCOMPARE(fine.position(2), 160 + 154);
        QCOMPARE(printRowAxis(sheet).size(1), 19);   // (what the grid has)
        QCOMPARE(printRowAxis(sheet).size(1), GridSheet::rowPixels(14.4));
        // a sheet that hides the rows it does not list ends at the last it lists
        Sheet zero;
        zero.format.zeroHeight = true;
        RowSpec shown;
        shown.row = 2;
        shown.height = 15;
        zero.rowSpecs = {shown};
        const Axis z = printRowAxis(zero);
        QCOMPARE(z.count(), qint64(3));
        QCOMPARE(z.size(0), 0);
        QCOMPARE(z.size(2), 20);
    }

    // A finer axis cuts the page where the width really ends: ten columns of 50.4 points are 504, and a page with 503 points to print on holds nine, not the ten that columns held to whole pixels
    // (67 of them, 50.25 points each) would say.
    void aFinerAxisCutsToThePoint()
    {
        Fixture whole(99, 29);
        whole.cols = Axis(16384, 67);
        whole.request.cols = &whole.cols;
        whole.request.setup.margins.left = 0.75;
        whole.request.setup.margins.right = 55.0 / 72;   // 612 - 54 - 55 = 503 points
        QCOMPARE(colsOf(whole.run()).section(QLatin1Char(' '), 0, 0), QStringLiteral("0-9"));
        Fixture fine(99, 29);
        fine.cols = Axis(16384, 538);   // 50.4 points in eighths of a pixel
        fine.request.cols = &fine.cols;
        fine.request.colFine = 8;
        fine.request.setup.margins = whole.request.setup.margins;
        QCOMPARE(colsOf(fine.run()).section(QLatin1Char(' '), 0, 0), QStringLiteral("0-8"));
        // the headings (a number of pixels) take their room the same from a fine axis, and a margin that is not a close call gives the same cut either way
        Fixture wide(99, 29), wideFine(99, 29);
        wide.cols = Axis(16384, 64);
        wide.request.cols = &wide.cols;
        wide.request.setup.headings = true;
        wideFine.cols = Axis(16384, 64 * 8);
        wideFine.request.cols = &wideFine.cols;
        wideFine.request.colFine = 8;
        wideFine.request.setup.headings = true;
        QCOMPARE(colsOf(wideFine.run()), colsOf(wide.run()));
        // a sheet with an object past its last cell takes the object's box in pixels whatever the axis is
        Fixture box(0, 0), boxFine(0, 0);
        boxFine.cols = Axis(16384, 64 * 8);
        boxFine.request.cols = &boxFine.cols;
        boxFine.request.colFine = 8;
        ObjectBox o;
        o.x = 64 * 3 + 5;
        o.y = 3;
        o.w = 10;
        o.h = 10;
        box.request.objects = {o};
        boxFine.request.objects = {o};
        QCOMPARE(box.run().areas[0].col2, boxFine.run().areas[0].col2);
        QCOMPARE(box.run().areas[0].col2, quint32(3));
    }

    // The last cell of each row of use is found at the cost of the rows, not of every cell (a sheet of 100,000 rows by 20 columns is asked about at every edit).
    void theEndOfEachRowIsFoundWithoutWalkingTheRow()
    {
        Sheet sheet;
        for (quint32 r = 0; r < 3000; ++r)
            for (quint32 c = 0; c < 20; ++c) sheet.cells.add(numberAt(r, c));
        // (one row is short, one is long: the guess of where a row begins is wrong there, and the answer is still right)
        sheet.cells.add(numberAt(5000, 0));
        for (quint32 c = 0; c < 200; ++c) sheet.cells.add(numberAt(5001, c));
        sheet.cells.finish();
        qint64 calls = 0;
        quint32 lastRow = 0, lastCol = 0;
        qint64 rowsSeen = 0;
        quint32 previousRow = 0xFFFFFFFF;
        sheet.cells.forEachRowTail([&](const Cell &cell) {
            ++calls;
            if (cell.row != previousRow) {
                ++rowsSeen;
                previousRow = cell.row;
                lastRow = std::max(lastRow, cell.row);
            }
            lastCol = std::max<quint32>(lastCol, cell.col);
            return true;
        });
        QCOMPARE(calls, qint64(3002));
        QCOMPARE(rowsSeen, qint64(3002));
        QCOMPARE(lastRow, 5001u);
        QCOMPARE(lastCol, 199u);
        // asked to go back over a row until a cell is wanted: it does, and only that row
        qint64 steps = 0;
        sheet.cells.forEachRowTail([&](const Cell &cell) {
            ++steps;
            return cell.row != 5001 || cell.col == 150;
        });
        QCOMPARE(steps, qint64(3001 + 50));
    }

    // Rows hidden between two manual breaks still make a page each (blank, with the titles): a list filtered down to a few rows keeps a page for each of its breaks.
    void hiddenRowsBetweenBreaksMakeBlankPages()
    {
        Fixture f(99, 3);
        for (qint64 r = 10; r < 80; ++r) f.rows.setSize(r, 0);
        for (quint32 b : {20u, 30u, 40u}) {
            PageBreak br;
            br.before = b;
            f.request.setup.rowBreaks.append(br);
        }
        SheetPrint p = f.run();
        QCOMPARE(rowsOf(p), QStringLiteral("0-19 20-29 30-39 80-99"));
        // the rows 10 to 79 are all hidden: 40 is a break whose page is the rows after the hidden ones
        QCOMPARE(p.pages[0].rowBands, 4);
        // titles are on the blank pages too (they are after the titles)
        f.request.setup.titles.hasRows = true;
        f.request.setup.titles.row1 = 0;
        f.request.setup.titles.row2 = 0;
        p = f.run();
        QCOMPARE(p.pages.size(), size_t(4));
        QCOMPARE(p.pages[1].titleRow1, qint64(0));
        QCOMPARE(p.pages[3].titleRow1, qint64(0));
        // a break in the hidden rows with no other between the visible rows before and after it makes no extra page
        Fixture g(99, 3);
        for (qint64 r = 10; r < 80; ++r) g.rows.setSize(r, 0);
        PageBreak one;
        one.before = 30;
        g.request.setup.rowBreaks.append(one);
        QCOMPARE(rowsOf(g.run()), QStringLiteral("0-29 80-99"));
        // hidden rows at the start with breaks among them: a blank page for each segment before the first row that shows, as for the ones in the middle
        Fixture h(99, 3);
        for (qint64 r = 0; r < 20; ++r) h.rows.setSize(r, 0);
        for (quint32 b : {5u, 12u}) {
            PageBreak br;
            br.before = b;
            h.request.setup.rowBreaks.append(br);
        }
        QCOMPARE(rowsOf(h.run()), QStringLiteral("0-4 5-11 20-64 65-99"));
    }

    // The paper: letter and A4 upright and turned, a custom size, margins that are negative or leave no room.
    void thePaperAndItsMarginsAreHeldToWhatThePageCanHave()
    {
        PageSetup s;
        PaperBox b = paperBox(s);
        QCOMPARE(b.width, 612.0);
        QCOMPARE(b.height, 792.0);
        QVERIFY(qAbs(b.left - 50.4) < 1e-9);
        QVERIFY(qAbs(b.content().width() - 511.2) < 1e-9);
        QCOMPARE(b.content().height(), 684.0);
        s.paperSize = 9;
        s.orientation = jp::office::sheet::PageOrientation::Landscape;
        b = paperBox(s);
        QVERIFY(qAbs(b.width - 841.89) < 0.01);
        QVERIFY(qAbs(b.height - 595.276) < 0.01);
        s.margins.left = -5;
        s.margins.top = -1;
        b = paperBox(s);
        QCOMPARE(b.left, 0.0);
        QCOMPARE(b.top, 0.0);
        // margins larger than the paper leave a page to print on
        s.margins.left = 100;
        s.margins.right = 100;
        s.margins.top = 100;
        s.margins.bottom = 100;
        b = paperBox(s);
        QVERIFY(b.content().width() >= 36);
        QVERIFY(b.content().height() >= 36);
        QVERIFY(b.left >= 0 && b.right >= 0 && b.top >= 0 && b.bottom >= 0);
    }

    // Whole rows and columns: 45 rows and 10 columns fit a page at 100%; the rest go on further pages, down and then over.
    void aSheetIsCutIntoWholeRowsAndColumns()
    {
        Fixture f(99, 9);
        SheetPrint p = f.run();
        QCOMPARE(p.scalePercent, 100);
        QCOMPARE(rowsOf(p), QStringLiteral("0-44 45-89 90-99"));
        QCOMPARE(colsOf(p), QStringLiteral("0-9 0-9 0-9"));
        QVERIFY(!p.truncated);
        Fixture wide(99, 11);   // 12 columns: 10 on the first band
        p = wide.run();
        QCOMPARE(p.pages.size(), size_t(6));
        QCOMPARE(rowsOf(p), QStringLiteral("0-44 45-89 90-99 0-44 45-89 90-99"));
        QCOMPARE(colsOf(p), QStringLiteral("0-9 0-9 0-9 10-11 10-11 10-11"));
        QCOMPARE(p.pages[3].colBand, 1);
        QCOMPARE(p.pages[3].rowBand, 0);
        QCOMPARE(p.pages[0].rowBands, 3);
        QCOMPARE(p.pages[0].colBands, 2);
        // over and then down
        wide.request.setup.pageOrder = jp::office::sheet::PageOrder::OverThenDown;
        p = wide.run();
        QCOMPARE(rowsOf(p), QStringLiteral("0-44 0-44 45-89 45-89 90-99 90-99"));
        QCOMPARE(colsOf(p), QStringLiteral("0-9 10-11 0-9 10-11 0-9 10-11"));
    }

    // The scale makes a page hold more or less of the sheet; the paper's turn and size do too.
    void theScaleAndThePaperChangeWhatAPageHolds()
    {
        Fixture f(199, 24);
        f.request.setup.scale = 50;   // 1363 by 1824 pixels: 21 columns and 91 rows
        SheetPrint p = f.run();
        QCOMPARE(p.scalePercent, 50);
        QCOMPARE(rowsOf(p), QStringLiteral("0-90 91-181 182-199 0-90 91-181 182-199"));
        QCOMPARE(colsOf(p), QStringLiteral("0-20 0-20 0-20 21-24 21-24 21-24"));
        Fixture l(99, 9);
        l.request.setup.orientation = jp::office::sheet::PageOrientation::Landscape;   // 691.2 by 504 points: 921.6 by 672 pixels: 14 columns and 33 rows
        p = l.run();
        QCOMPARE(rowsOf(p), QStringLiteral("0-32 33-65 66-98 99-99"));
        QCOMPARE(colsOf(p), QStringLiteral("0-9 0-9 0-9 0-9"));
        Fixture big(99, 9);
        big.request.setup.scale = 400;   // 170.4 by 228 pixels: 2 columns and 11 rows
        p = big.run();
        QCOMPARE(p.scalePercent, 400);
        QCOMPARE(p.pages.size(), size_t(10 * 5));
        QCOMPARE(p.pages[0].row2, qint64(10));
        QCOMPARE(p.pages[0].col2, qint64(1));
    }

    // A scale a file gives that is not one is held to 10 to 400.
    void aScaleOutsideTheLimitsIsHeldToThem()
    {
        Fixture f;
        f.request.setup.scale = 5;
        QCOMPARE(f.run().scalePercent, 10);
        f.request.setup.scale = 9000;
        QCOMPARE(f.run().scalePercent, 400);
        f.request.setup.scale = -3;
        QCOMPARE(f.run().scalePercent, 10);
    }

    // Fit to pages: the largest percent at which the sheet is no more than that many pages wide and tall; 0 is as many as it takes.
    void aSheetFitsTheNumberOfPagesAskedFor()
    {
        Fixture f(99, 29);   // 30 columns: 1,920 pixels across
        f.request.setup.fitToPage = true;
        f.request.setup.fitToWidth = 1;
        f.request.setup.fitToHeight = 0;
        SheetPrint p = f.run();
        QVERIFY(p.fitted);
        QCOMPARE(p.scalePercent, 35);   // 511.2 / (1,920 * 0.75) = 35.5
        QCOMPARE(p.pages[0].colBands, 1);
        QCOMPARE(p.pages[0].col2, qint64(29));
        // both: 100 rows of 20 pixels is 2,000 tall, which at 35 percent is 525 points: the width decides; one page tall as well
        f.request.setup.fitToHeight = 1;
        p = f.run();
        QCOMPARE(p.scalePercent, 35);
        QCOMPARE(p.pages.size(), size_t(1));
        // taller than wide: the height decides
        Fixture tall(299, 4);   // 300 rows = 6,000 pixels = 4,500 points: 684 / 4,500 = 15.2
        tall.request.setup.fitToPage = true;
        tall.request.setup.fitToWidth = 1;
        tall.request.setup.fitToHeight = 1;
        p = tall.run();
        QCOMPARE(p.scalePercent, 15);
        QCOMPARE(p.pages.size(), size_t(1));
        // two pages wide, automatic tall
        f.request.setup.fitToWidth = 2;
        f.request.setup.fitToHeight = 0;
        p = f.run();
        QVERIFY(p.scalePercent > 35 && p.scalePercent <= 100);
        QCOMPARE(p.pages[0].colBands, 2);
        // a sheet that already fits is not made larger
        Fixture small(5, 2);
        small.request.setup.fitToPage = true;
        small.request.setup.fitToWidth = 1;
        small.request.setup.fitToHeight = 1;
        QCOMPARE(small.run().scalePercent, 100);
        // automatic in both directions is the scale of the setup
        Fixture automatic(99, 29);
        automatic.request.setup.fitToPage = true;
        automatic.request.setup.fitToWidth = 0;
        automatic.request.setup.fitToHeight = 0;
        automatic.request.setup.scale = 80;
        QCOMPARE(automatic.run().scalePercent, 80);
        // the smallest scale is 10: a sheet that does not fit even then says so
        Fixture huge(99, 3000);
        huge.request.setup.fitToPage = true;
        huge.request.setup.fitToWidth = 1;
        huge.request.setup.fitToHeight = 1;
        p = huge.run();
        QCOMPARE(p.scalePercent, 10);
        QVERIFY(p.tooSmall);
        QVERIFY(p.pages.size() > 1);
    }

    // Breaks a person put in start a page; with fit to pages they are ignored.
    void manualBreaksStartPages()
    {
        Fixture f(99, 9);
        jp::office::sheet::PageBreak b;
        b.before = 30;
        f.request.setup.rowBreaks.append(b);
        b.before = 5;
        f.request.setup.colBreaks.append(b);
        SheetPrint p = f.run();
        QCOMPARE(rowsOf(p), QStringLiteral("0-29 30-74 75-99 0-29 30-74 75-99"));
        QCOMPARE(colsOf(p), QStringLiteral("0-4 0-4 0-4 5-9 5-9 5-9"));
        // an automatic break the file lists, and a break at the first row, are no breaks
        Fixture g(99, 9);
        b.before = 20;
        b.manual = false;
        g.request.setup.rowBreaks.append(b);
        QCOMPARE(rowsOf(g.run()), QStringLiteral("0-44 45-89 90-99"));
        g.request.setup.rowBreaks.clear();
        b.before = 0;
        b.manual = true;
        g.request.setup.rowBreaks.append(b);
        QCOMPARE(rowsOf(g.run()), QStringLiteral("0-44 45-89 90-99"));
        // fit to pages ignores them
        f.request.setup.fitToPage = true;
        f.request.setup.fitToWidth = 1;
        f.request.setup.fitToHeight = 1;
        QCOMPARE(f.run().pages.size(), size_t(1));
    }

    // Hidden rows and columns take no room and begin no page; a break before one is a break before the next that shows.
    void hiddenRowsAndColumnsAreSkipped()
    {
        Fixture f(99, 9);
        for (int r = 10; r < 20; ++r) f.rows.setSize(r, 0);   // ten hidden rows: 90 rows show
        SheetPrint p = f.run();
        QCOMPARE(rowsOf(p), QStringLiteral("0-54 55-99"));   // (45 shown rows: 0-9 and 20-54)
        // hidden rows at the end
        Fixture g(99, 9);
        for (int r = 90; r < 100; ++r) g.rows.setSize(r, 0);
        QCOMPARE(rowsOf(g.run()), QStringLiteral("0-44 45-89"));
        // a page does not begin on a hidden row, whatever the break says
        Fixture h(99, 9);
        for (int r = 30; r < 35; ++r) h.rows.setSize(r, 0);
        jp::office::sheet::PageBreak b;
        b.before = 32;
        h.request.setup.rowBreaks.append(b);
        QCOMPARE(rowsOf(h.run()), QStringLiteral("0-31 35-79 80-99"));
        // hidden columns
        Fixture c(9, 11);
        for (int i = 0; i < 4; ++i) c.cols.setSize(i, 0);   // 8 columns show
        QCOMPARE(colsOf(c.run()), QStringLiteral("4-11"));   // (the hidden ones at the start are not part of the block)
        // everything hidden: nothing to print
        Fixture none(9, 9);
        for (int r = 0; r < 10; ++r) none.rows.setSize(r, 0);
        QVERIFY(none.run().pages.empty());
    }

    // A row taller than the page has a page of its own, and the cutting goes on after it.
    void aRowTallerThanThePageHasItsOwnPage()
    {
        Fixture f(9, 2);
        f.rows.setSize(4, 2000);   // (more than the 912 pixels a page holds)
        const SheetPrint p = f.run();
        QCOMPARE(rowsOf(p), QStringLiteral("0-3 4-4 5-9"));
    }


    // Titles that take more than half a page are not repeated: a page of titles with one row of the sheet under it, for every row, is not a printout.
    void titlesTallerThanThePageAreLeftOff()
    {
        Fixture f(1999, 3);
        f.request.setup.titles.hasRows = true;
        f.request.setup.titles.row1 = 0;
        f.request.setup.titles.row2 = 1499;   // (1,500 rows of 20 pixels: 33 pages' worth)
        SheetPrint p = f.run();
        QCOMPARE(p.pages.size(), size_t(45));   // (2,000 rows at 45 a page, as with no titles)
        for (const PageCut &c : p.pages) QCOMPARE(c.titleRow1, qint64(-1));
        // titles that fit within half a page still repeat
        Fixture g(199, 3);
        g.request.setup.titles.hasRows = true;
        g.request.setup.titles.row1 = 0;
        g.request.setup.titles.row2 = 19;   // 400 of the 912 pixels
        SheetPrint q = g.run();
        QCOMPARE(q.pages[1].titleRow2, qint64(19));
    }

    // Titles are printed again above (and to the left of) every page after the one they are on.
    void titlesRepeatOnThePagesAfterThem()
    {
        Fixture f(99, 9);
        f.request.setup.titles.hasRows = true;
        f.request.setup.titles.row1 = 0;
        f.request.setup.titles.row2 = 1;
        SheetPrint p = f.run();
        QCOMPARE(rowsOf(p), QStringLiteral("0-44 45-87 88-99"));   // (the later pages have 43 rows and the two of the titles)
        QCOMPARE(p.pages[0].titleRow1, qint64(-1));
        QCOMPARE(p.pages[1].titleRow1, qint64(0));
        QCOMPARE(p.pages[1].titleRow2, qint64(1));
        QCOMPARE(p.pages[2].titleRow2, qint64(1));
        // titles in the middle: the pages before them have none
        Fixture m(99, 9);
        m.request.setup.titles.hasRows = true;
        m.request.setup.titles.row1 = 10;
        m.request.setup.titles.row2 = 10;
        p = m.run();
        QCOMPARE(p.pages[0].titleRow1, qint64(-1));
        QCOMPARE(rowsOf(p), QStringLiteral("0-44 45-88 89-99"));
        QCOMPARE(p.pages[1].titleRow1, qint64(10));
        // columns: 12 columns, the first repeated on the second band, which then has room for 9 more (the first band has 10)
        Fixture c(9, 14);
        c.request.setup.titles.hasCols = true;
        c.request.setup.titles.col1 = 0;
        c.request.setup.titles.col2 = 0;
        p = c.run();
        QCOMPARE(colsOf(p), QStringLiteral("0-9 10-14"));
        QCOMPARE(p.pages[1].titleCol1, qint64(0));
        QCOMPARE(p.pages[0].titleCol1, qint64(-1));
        // titles above a print area that does not reach them are printed on every page, the first too
        Fixture a(99, 9);
        a.request.setup.titles.hasRows = true;
        a.request.setup.titles.row1 = 0;
        a.request.setup.titles.row2 = 0;
        CellRange area;
        area.valid = true;
        area.row1 = 10;
        area.row2 = 99;
        area.col1 = 0;
        area.col2 = 9;
        a.request.setup.printArea.append(area);
        p = a.run();
        QCOMPARE(p.pages[0].titleRow1, qint64(0));
        QCOMPARE(rowsOf(p), QStringLiteral("10-53 54-97 98-99"));   // (44 rows with one of titles)
        // titles that fill the page do not stop the cutting
        Fixture g(99, 9);
        g.request.setup.titles.hasRows = true;
        g.request.setup.titles.row1 = 0;
        g.request.setup.titles.row2 = 60;
        p = g.run();
        QVERIFY(p.pages.size() >= 2);
        QVERIFY(p.pages.size() < 100);
    }

    // Each print area is cut by itself, on pages of its own, in order; the print area is ignored when the job says so.
    void printAreasHavePagesOfTheirOwn()
    {
        Fixture f(99, 9);
        CellRange a, b;
        a.valid = b.valid = true;
        a.row1 = 0;
        a.row2 = 2;
        a.col1 = 0;
        a.col2 = 2;
        b.row1 = 4;
        b.row2 = 5;
        b.col1 = 4;
        b.col2 = 5;
        f.request.setup.printArea = {a, b};
        SheetPrint p = f.run();
        QCOMPARE(p.pages.size(), size_t(2));
        QCOMPARE(p.pages[0].area, 0);
        QCOMPARE(p.pages[1].area, 1);
        QCOMPARE(p.pages[1].row1, qint64(4));
        QCOMPARE(p.pages[1].col2, qint64(5));
        QCOMPARE(p.areas.size(), size_t(2));
        f.request.ignorePrintArea = true;
        QCOMPARE(f.run().pages.size(), size_t(3));
        // only the selection
        f.request.onlySelection = true;
        f.request.selection = b;
        p = f.run();
        QCOMPARE(p.pages.size(), size_t(1));
        QCOMPARE(p.pages[0].row1, qint64(4));
        // a print area of whole columns or whole rows is printed as far as the sheet has anything, not a million rows of paper
        Fixture g(9, 9);
        CellRange off;
        off.valid = true;
        off.row1 = 0;
        off.row2 = 1048575;
        off.col1 = 0;
        off.col2 = 0;
        g.request.setup.printArea = {off};
        p = g.run();
        QCOMPARE(p.pages.size(), size_t(1));
        QCOMPARE(p.pages[0].row2, qint64(9));
        QCOMPARE(p.pages[0].col2, qint64(0));
        CellRange rowsOnly = off;
        rowsOnly.row1 = 2;
        rowsOnly.row2 = 3;
        rowsOnly.col2 = 16383;
        g.request.setup.printArea = {rowsOnly};
        p = g.run();
        QCOMPARE(p.pages.size(), size_t(1));
        QCOMPARE(p.pages[0].col2, qint64(9));
        QCOMPARE(p.pages[0].row1, qint64(2));
        // the whole sheet: the cells it has
        CellRange all = off;
        all.col2 = 16383;
        g.request.setup.printArea = {all};
        p = g.run();
        QCOMPARE(p.pages[0].row2, qint64(9));
        QCOMPARE(p.pages[0].col2, qint64(9));
        // and a sheet with nothing on it has nothing in a print area of whole lines
        Fixture blank(0, 0);
        blank.sheet = Sheet();
        blank.sheet.cells.finish();
        blank.request.setup.printArea = {all};
        QVERIFY(blank.run().pages.empty());
        // a block that only reaches far (not whole rows or columns) is printed as it is
        CellRange far = off;
        far.row1 = 5;
        g.request.setup.printArea = {far};
        QVERIFY(g.run().pages.size() > 100);
    }

    // What prints with no print area: the cells with something to show, the borders and fills that show, and the objects; not a number format alone.
    void theUsedRangeIsWhatHasSomethingToShow()
    {
        Fixture f(4, 4);
        // a cell with a style that shows a border at row 20, a fill at column 12, and one with a font only at row 50
        f.styles.formats = {CellFormat(), CellFormat(), CellFormat(), CellFormat()};
        f.styles.cellXfs = {0, 0, 0, 0};
        BorderSpec bordered;
        bordered.bottom.style = jp::office::sheet::BorderStyle::Thin;
        f.styles.borders = {BorderSpec(), bordered};
        FillSpec solid;
        solid.pattern = jp::office::sheet::PatternKind::Solid;
        f.styles.fills = {FillSpec(), FillSpec(), solid};
        f.styles.formats[1].border = 1;
        f.styles.formats[2].fill = 2;
        f.styles.formats[3].numFmtId = 10;
        Cell c = numberAt(0, 0);
        c.type = CellType::Empty;
        Cell border = c, fill = c, plain = c;
        border.row = 20;
        border.col = 1;
        border.style = 1;
        fill.row = 10;
        fill.col = 12;
        fill.style = 2;
        plain.row = 50;
        plain.col = 30;
        plain.style = 3;
        f.sheet.cells.add(border);
        f.sheet.cells.add(fill);
        f.sheet.cells.add(plain);
        f.sheet.cells.finish();
        const std::vector<CellRange> used = printedBlocks(f.request);
        QCOMPARE(used.size(), size_t(1));
        QCOMPARE(used[0].row1, 0u);
        QCOMPARE(used[0].col1, 0u);
        QCOMPARE(used[0].row2, 20u);
        QCOMPARE(used[0].col2, 12u);
        // an object over cells reaches as far as it does (a box from the grid's first pixel to 1,000 by 1,300)
        ObjectBox box;
        box.x = 0;
        box.y = 0;
        box.w = 1000;
        box.h = 1300;
        f.request.objects = {box};
        const std::vector<CellRange> withObject = printedBlocks(f.request);
        QCOMPARE(withObject[0].col2, 15u);      // (column 15 holds pixel 999: columns of 64)
        QCOMPARE(withObject[0].row2, 64u);      // (row 64 holds pixel 1,299)
        // the sheet with nothing on it prints nothing
        Fixture empty(0, 0);
        empty.sheet = Sheet();
        empty.sheet.cells.finish();
        QVERIFY(paginate(empty.request).pages.empty());
    }

    // A merged range with its value in the first cell reaches to its last.
    void aMergedRangeIsPrintedWhole()
    {
        Fixture f(0, 0);
        CellRange m;
        m.valid = true;
        m.row1 = 0;
        m.row2 = 3;
        m.col1 = 0;
        m.col2 = 5;
        f.sheet.merges.append(m);
        const std::vector<CellRange> used = printedBlocks(f.request);
        QCOMPARE(used[0].row2, 3u);
        QCOMPARE(used[0].col2, 5u);
    }

    // Headings take room from the page: the rows that fit are fewer by the height of the column headings.
    void headingsTakeRoomFromThePage()
    {
        Fixture f(99, 9);
        f.request.setup.headings = true;
        const SheetPrint p = f.run();
        QVERIFY(p.headings);
        QCOMPARE(rowsOf(p), QStringLiteral("0-43 44-87 88-99"));   // (912 less the 20 pixels of the letters: 892 pixels, 44 rows)
        QCOMPARE(colsOf(p), QStringLiteral("0-9 0-9 0-9"));       // (681.6 less 36: 645.6 pixels, 10 columns)
    }

    // A sheet of a million rows at the smallest scale still has a bound on its pages, and the work of cutting is that of the pages, not of the rows.
    void aHugeSheetIsCutShortAtTheBound()
    {
        Fixture f(1048575, 3);
        f.request.setup.scale = 10;   // 9,120 pixels: 456 rows a page; 2,299 pages for the rows
        f.request.maxPages = 1000;
        const qint64 before = f.rows.probes();
        SheetPrint p = f.run();
        QVERIFY(p.truncated);
        QCOMPARE(p.pages.size(), size_t(1000));
        QVERIFY(p.pagesNeeded > 1000);
        QVERIFY(f.rows.probes() - before < 400000);   // (a logarithm of the places for each page: nowhere near a million rows)
        f.request.maxPages = kMaxSheetPages;
        p = f.run();
        QVERIFY(!p.truncated);
        QCOMPARE(p.pages.size(), size_t(2300));
        QCOMPARE(p.pagesNeeded, qint64(2300));
        QCOMPARE(p.pages.back().row2, qint64(1048575));
    }

    // Sizes a file gives that are not usable (a column of no width, a hidden last row, a margin of a thousand inches) never end in a loop or a page without a cell.
    void oddSizesNeverMakeAnEmptyOrEndlessPage()
    {
        Fixture f(20, 5);
        f.request.setup.margins.left = 1000;
        f.request.setup.margins.top = 1000;
        f.request.setup.margins.right = 1000;
        f.request.setup.margins.bottom = 1000;
        const SheetPrint p = f.run();
        QVERIFY(!p.pages.empty());
        QVERIFY(p.pages.size() <= 21 * 6);
        for (const PageCut &c : p.pages) {
            QVERIFY(c.row2 >= c.row1);
            QVERIFY(c.col2 >= c.col1);
        }
    }
};

JP_TEST_CLASS(GridPrintTests)
#include "tst_grid_print.moc"
