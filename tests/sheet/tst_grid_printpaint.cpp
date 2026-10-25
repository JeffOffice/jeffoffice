// Tests of drawing a printed page (libs/shell/grid/printpaint.h): where the cells land on the paper (the margins, the scale, centering), the gridlines and headings only when asked for, the
// repeated titles, black and white, the errors setting, the objects hook, and the header and footer in the margins. A page is drawn on an image at 2 pixels to the point; the checks are on
// colors at places and on where ink is, never on the shape of a letter, so they hold on every system's fonts.
#include "core/fonts.h"
#include "sheethelpers.h"

#include "grid/printpaint.h"

#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QPdfWriter>

using namespace jp::grid;

namespace {

constexpr double kPx = 2;   // image pixels to a point

struct Page {
    Sheet sheet;
    Styles styles;
    SharedStrings strings;
    GridSheet grid;
    std::unique_ptr<PagePainter> painter;
    Axis rows{1048576, 20}, cols{16384, 64};
    PageSetup setup;
    quint32 red = 0;

    Page()
    {
        FontSpec f;
        f.name = QStringLiteral("Calibri");
        f.size = 11;
        styles.fonts = {f};
        FillSpec none;
        styles.fills = {none};
        styles.borders = {BorderSpec()};
        styles.formats = {CellFormat()};
        styles.cellXfs = {0};
        FillSpec solid;
        solid.pattern = PatternKind::Solid;
        solid.fg.kind = ColorSpec::Kind::Rgb;
        solid.fg.rgb = 0xFF0000;
        styles.fills.append(solid);
        CellFormat cf;
        cf.fill = 1;
        styles.formats.append(cf);
        styles.cellXfs.append(0);
        red = 1;
        sheet.view.showHeadings = false;
    }
    void cell(quint32 row, quint32 col, quint32 style = 0, double value = 1)
    {
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.type = CellType::Number;
        c.number = value;
        c.style = style;
        sheet.cells.add(c);
    }
    void open()
    {
        sheet.cells.finish();
        GridData d;
        d.styles = &styles;
        d.strings = &strings;
        d.sheet = &sheet;
        grid.setData(d);
        painter = std::make_unique<PagePainter>(&grid);
    }
    SheetPrint print()
    {
        PrintRequest q;
        q.sheet = &sheet;
        q.styles = &styles;
        q.rows = &grid.layout().rows;
        q.cols = &grid.layout().cols;
        q.setup = setup;
        return paginate(q);
    }
    QImage render(const SheetPrint &p, size_t page, const jp::office::sheet::HfContext &ctx = jp::office::sheet::HfContext(), const PageObjects &objects = nullptr)
    {
        QImage img(int(p.paper.width * kPx), int(p.paper.height * kPx), QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter qp(&img);
        qp.scale(kPx, kPx);
        painter->paint(&qp, p, p.pages[page], setup, ctx, page == 0, objects);
        return img;
    }
};

QRgb at(const QImage &img, double xPt, double yPt) { return img.pixel(int(xPt * kPx), int(yPt * kPx)); }
bool isRed(QRgb c) { return qRed(c) > 200 && qGreen(c) < 60 && qBlue(c) < 60; }
bool isWhite(QRgb c) { return qRed(c) > 250 && qGreen(c) > 250 && qBlue(c) > 250; }
bool isDark(QRgb c) { return qRed(c) < 140 && qGreen(c) < 140 && qBlue(c) < 140; }

// The box of the pixels that are not white inside a region of the page (points), or an empty rect.
QRectF inkIn(const QImage &img, const QRectF &region)
{
    int x0 = 1 << 30, y0 = 1 << 30, x1 = -1, y1 = -1;
    for (int y = int(region.top() * kPx); y < int(region.bottom() * kPx) && y < img.height(); ++y)
        for (int x = int(region.left() * kPx); x < int(region.right() * kPx) && x < img.width(); ++x)
            if (!isWhite(img.pixel(x, y))) {
                x0 = std::min(x0, x);
                y0 = std::min(y0, y);
                x1 = std::max(x1, x);
                y1 = std::max(y1, y);
            }
    return x1 < 0 ? QRectF() : QRectF(x0 / kPx, y0 / kPx, (x1 - x0 + 1) / kPx, (y1 - y0 + 1) / kPx);
}

} // namespace

class GridPrintPaintTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The first cell is at the corner of the margins, 48 by 15 points at 100%, and at half the scale it is half that.
    void theCellsLandInsideTheMargins()
    {
        Page pg;
        pg.cell(0, 0, pg.red);
        pg.cell(40, 5);
        pg.open();
        SheetPrint p = pg.print();
        QImage img = pg.render(p, 0);
        QVERIFY(isRed(at(img, 51, 55)));
        QVERIFY(isRed(at(img, 97, 68)));
        QVERIFY(isWhite(at(img, 99.5, 60)));    // (past the 48 points)
        QVERIFY(isWhite(at(img, 60, 70)));      // (past the 15 points)
        QVERIFY(isWhite(at(img, 49, 60)));      // (in the margin)
        QVERIFY(isWhite(at(img, 60, 52)));
        pg.setup.scale = 50;
        p = pg.print();
        img = pg.render(p, 0);
        QVERIFY(isRed(at(img, 50.4 + 6, 54 + 5)));      // (the cell's number is at its right end: this is its left part)
        QVERIFY(isWhite(at(img, 50.4 + 26, 54 + 5)));
        QVERIFY(isWhite(at(img, 50.4 + 10, 54 + 9)));
        pg.setup.scale = 200;
        p = pg.print();
        img = pg.render(p, 0);
        QVERIFY(isRed(at(img, 50.4 + 90, 54 + 25)));
        QVERIFY(isWhite(at(img, 50.4 + 100, 54 + 25)));
        QVERIFY(isWhite(at(img, 50.4 + 50, 54 + 35)));
    }

    // Centering puts the block in the middle of the page, across or down.
    void theBlockCanBeCenteredOnThePage()
    {
        Page pg;
        pg.cell(0, 0, pg.red);
        pg.open();
        pg.setup.centerHorizontally = true;
        SheetPrint p = pg.print();
        QImage img = pg.render(p, 0);
        QVERIFY(isRed(at(img, 306, 58)));       // (the page is 612 wide: the cell is 48 wide around the middle)
        QVERIFY(isWhite(at(img, 60, 58)));
        pg.setup.centerVertically = true;
        p = pg.print();
        img = pg.render(p, 0);
        QVERIFY(isRed(at(img, 306, 396)));
        QVERIFY(isWhite(at(img, 306, 58)));
    }

    // Gridlines are on the paper only when the setup asks for them; a draft has none.
    void gridlinesPrintOnlyWhenAsked()
    {
        Page pg;
        pg.cell(0, 0);
        pg.cell(5, 5);
        pg.open();
        SheetPrint p = pg.print();
        QImage off = pg.render(p, 0);
        QVERIFY(inkIn(off, QRectF(50.4 + 100, 54 + 30, 60, 30)).isEmpty());   // (an empty part of the block)
        pg.setup.gridLines = true;
        QImage on = pg.render(p, 0);
        const QRectF lines = inkIn(on, QRectF(50.4 + 100, 54 + 30, 60, 30));
        QVERIFY(!lines.isEmpty());
        // the line along the bottom of row 3 (15 points a row) and the one at the right of column 3 (48 points a column)
        bool horizontal = false, vertical = false;
        for (int x = int((50.4 + 100) * kPx); x < int((50.4 + 150) * kPx); ++x)
            horizontal = horizontal || isDark(on.pixel(x, int((54 + 45) * kPx) - 1)) || isDark(on.pixel(x, int((54 + 45) * kPx)));
        for (int y = int((54 + 30) * kPx); y < int((54 + 50) * kPx); ++y)
            vertical = vertical || isDark(on.pixel(int((50.4 + 144) * kPx) - 1, y)) || isDark(on.pixel(int((50.4 + 144) * kPx), y));
        QVERIFY(horizontal);
        QVERIFY(vertical);
        pg.setup.draft = true;
        QVERIFY(inkIn(pg.render(p, 0), QRectF(50.4 + 100, 54 + 30, 60, 30)).isEmpty());
    }

    // The headings take room before the cells and are drawn in it: the numbers down the left, the letters across the top.
    void headingsAreDrawnBeforeTheCells()
    {
        Page pg;
        pg.cell(0, 0, pg.red);
        pg.open();
        pg.setup.headings = true;
        const SheetPrint p = pg.print();
        const QImage img = pg.render(p, 0);
        // 36 pixels (27 points) of numbers and 20 (15 points) of letters
        QVERIFY(isRed(at(img, 50.4 + 27 + 2, 54 + 15 + 2)));
        QVERIFY(!isWhite(at(img, 50.4 + 10, 54 + 15 + 7)));
        QVERIFY(!isWhite(at(img, 50.4 + 27 + 20, 54 + 5)));
        QVERIFY(!inkIn(img, QRectF(50.4, 54 + 15, 27, 15)).isEmpty());
        QVERIFY(!inkIn(img, QRectF(50.4 + 27, 54, 48, 15)).isEmpty());
    }

    // The title rows are drawn again above the later pages; the page's own rows follow them.
    void titlesAreDrawnAboveLaterPages()
    {
        Page pg;
        pg.cell(0, 0, pg.red);
        pg.cell(99, 0);
        pg.open();
        pg.setup.titles.hasRows = true;
        pg.setup.titles.row1 = 0;
        pg.setup.titles.row2 = 0;
        const SheetPrint p = pg.print();
        QVERIFY(p.pages.size() >= 2);
        const QImage first = pg.render(p, 0), second = pg.render(p, 1);
        QVERIFY(isRed(at(first, 60, 58)));
        QVERIFY(isRed(at(second, 60, 58)));     // (the first row again, at the top)
        QVERIFY(isWhite(at(second, 60, 54 + 15 + 5)));   // (and the rows after it are the page's own: no red among them)
    }

    // Black and white leaves no color: a fill is left out.
    void blackAndWhiteHasNoFills()
    {
        Page pg;
        pg.cell(0, 0, pg.red);
        pg.open();
        pg.setup.blackAndWhite = true;
        const SheetPrint p = pg.print();
        const QImage img = pg.render(p, 0);
        QVERIFY(isWhite(at(img, 51, 55)));
    }

    // Errors print as the setup says: as they are, blank, as "--", or as #N/A.
    void errorsPrintAsTheSetupSays()
    {
        Page pg;
        Cell c;
        c.row = 0;
        c.col = 0;
        c.type = CellType::Error;
        c.number = double(int(CellError::Div0));
        pg.sheet.cells.add(c);
        pg.open();
        const Cell *cell = pg.sheet.cells.find(0, 0);
        QCOMPARE(pg.grid.shown(0, 0, *cell).text, QStringLiteral("#DIV/0!"));
        pg.grid.setErrorDisplay(GridSheet::ErrorDisplay::Blank);
        QVERIFY(pg.grid.shown(0, 0, *cell).text.isEmpty());
        pg.grid.setErrorDisplay(GridSheet::ErrorDisplay::Dash);
        QCOMPARE(pg.grid.shown(0, 0, *cell).text, QStringLiteral("--"));
        pg.grid.setErrorDisplay(GridSheet::ErrorDisplay::NA);
        QCOMPARE(pg.grid.shown(0, 0, *cell).text, QStringLiteral("#N/A"));
        pg.grid.setErrorDisplay(GridSheet::ErrorDisplay::Shown);
        QCOMPARE(pg.grid.shown(0, 0, *cell).text, QStringLiteral("#DIV/0!"));
    }

    // The objects are drawn by the caller's function, in the grid's pixels from the corner of the cells and clipped to them.
    void theObjectsHookDrawsOverTheCells()
    {
        Page pg;
        pg.cell(0, 0);
        pg.open();
        int calls = 0;
        QSize cellsSize;
        double zoom = 0;
        const SheetPrint p = pg.print();
        const QImage img = pg.render(p, 0, jp::office::sheet::HfContext(), [&](QPainter *painter, const GridLayout &layout, const QRect &cells) {
            ++calls;
            cellsSize = cells.size();
            zoom = layout.zoom();
            painter->fillRect(QRect(0, 0, int(20 * zoom), int(20 * zoom)), Qt::green);   // 20 grid pixels square at the corner
            painter->fillRect(QRect(int(500 * zoom), 0, 100, 100), Qt::blue);             // (outside the cells: clipped away)
        });
        QCOMPARE(calls, 1);
        QCOMPARE(cellsSize.width(), int(std::llround(64 * zoom)));
        QCOMPARE(cellsSize.height(), int(std::llround(20 * zoom)));
        const QRgb in = at(img, 50.4 + 7, 54 + 7);
        QVERIFY(qGreen(in) > 200 && qRed(in) < 60);
        QVERIFY(isWhite(at(img, 50.4 + 400, 54 + 3)));
        // a draft leaves objects out
        pg.setup.draft = true;
        calls = 0;
        pg.render(pg.print(), 0, jp::office::sheet::HfContext(), [&](QPainter *, const GridLayout &, const QRect &) { ++calls; });
        QCOMPARE(calls, 0);
    }

    // The header is in the top margin and the footer in the bottom, each part at its side of the page: left, middle, right.
    void theHeaderAndFooterAreInTheMargins()
    {
        Page pg;
        pg.cell(0, 0);
        pg.open();
        pg.setup.headerFooter.oddHeader = QStringLiteral("&LLeft part&CMiddle part&RRight part");
        pg.setup.headerFooter.oddFooter = QStringLiteral("&CPage &P of &N");
        const SheetPrint p = pg.print();
        jp::office::sheet::HfContext ctx;
        ctx.page = 3;
        ctx.pages = 12;
        ctx.sheet = QStringLiteral("S");
        const QImage img = pg.render(p, 0, ctx);
        const QRectF top = inkIn(img, QRectF(0, 0, 612, 54)), bottom = inkIn(img, QRectF(0, 738, 612, 54));
        QVERIFY(!top.isEmpty());
        QVERIFY(!bottom.isEmpty());
        // the header begins at the left margin and ends at the right margin
        QVERIFY(qAbs(top.left() - 50.4) < 4);
        QVERIFY(qAbs(top.right() - (612 - 50.4)) < 4);
        QVERIFY(top.top() >= 21.6 - 1);
        QVERIFY(top.bottom() <= 54);
        // the footer is in the middle, with its bottom above the footer margin
        QVERIFY(qAbs(bottom.center().x() - 306) < 4);
        QVERIFY(bottom.bottom() <= 792 - 21.6 + 1);
        // the page's number is in it: another page has other ink
        jp::office::sheet::HfContext other = ctx;
        other.page = 10;
        QVERIFY(pg.render(p, 0, other) != img);
        // no header or footer where the setup has none
        pg.setup.headerFooter = HeaderFooter();
        const QImage plain = pg.render(p, 0, ctx);
        QVERIFY(inkIn(plain, QRectF(0, 0, 612, 54)).isEmpty());
        QVERIFY(inkIn(plain, QRectF(0, 738, 612, 54)).isEmpty());
    }

    // The first page, the even pages, and the rest have their own header and footer when the setup says so.
    void theFirstAndEvenPagesHaveTheirOwn()
    {
        Page pg;
        pg.cell(0, 0);
        pg.cell(99, 0);
        pg.open();
        HeaderFooter &hf = pg.setup.headerFooter;
        hf.oddHeader = QStringLiteral("&CODD odd odd odd");
        hf.evenHeader = QStringLiteral("&LEVEN");
        hf.firstHeader = QStringLiteral("&RFIRST");
        hf.differentOddEven = true;
        hf.differentFirst = true;
        const SheetPrint p = pg.print();
        QVERIFY(p.pages.size() >= 2);
        jp::office::sheet::HfContext ctx;
        ctx.page = 1;
        QRectF first = inkIn(pg.render(p, 0, ctx), QRectF(0, 0, 612, 54));
        QVERIFY(first.center().x() > 450);   // (the right part)
        ctx.page = 2;
        QRectF even = inkIn(pg.render(p, 1, ctx), QRectF(0, 0, 612, 54));
        QVERIFY(even.center().x() < 150);    // (the left part)
        // a third page of the same sheet: odd, in the middle (the second page is the only one here, so ask for it by number)
        ctx.page = 3;
        QRectF odd = inkIn(pg.render(p, 1, ctx), QRectF(0, 0, 612, 54));
        QVERIFY(qAbs(odd.center().x() - 306) < 8);
        // the first page of the sheet with differentFirst and nothing for it has no header
        hf.firstHeader.clear();
        ctx.page = 1;
        QVERIFY(inkIn(pg.render(p, 0, ctx), QRectF(0, 0, 612, 54)).isEmpty());
    }

    // The ink of the header follows the codes: a size makes it taller, a color is used, black and white makes it black.
    void sizesAndColorsOfTheHeaderAreUsed()
    {
        Page pg;
        pg.cell(0, 0);
        pg.open();
        pg.setup.headerFooter.oddHeader = QStringLiteral("&C&10Small");
        SheetPrint p = pg.print();
        const QRectF small = inkIn(pg.render(p, 0), QRectF(0, 0, 612, 54));
        pg.setup.headerFooter.oddHeader = QStringLiteral("&C&24Small");
        const QRectF big = inkIn(pg.render(p, 0), QRectF(0, 0, 612, 54));
        QVERIFY(big.width() > small.width() * 1.8);
        pg.setup.headerFooter.oddHeader = QStringLiteral("&C&KFF0000Small");
        QImage red = pg.render(p, 0);
        bool colored = false;
        for (int y = 0; y < int(54 * kPx); ++y)
            for (int x = 0; x < red.width(); ++x) colored = colored || isRed(red.pixel(x, y));
        QVERIFY(colored);
        pg.setup.blackAndWhite = true;
        red = pg.render(p, 0);
        colored = false;
        for (int y = 0; y < int(54 * kPx); ++y)
            for (int x = 0; x < red.width(); ++x) colored = colored || isRed(red.pixel(x, y));
        QVERIFY(!colored);
    }

    // A dashed or dotted border is one stroke of a dashed pen on paper, not a mark for every dash: a page of dotted borders is a few kilobytes of PDF, not megabytes (and a printer is
    // not sent tens of thousands of dots), and the dots are still there.
    void dottedBordersAreStrokesOnPaper()
    {
        Page pg;
        BorderSpec b;
        b.bottom.style = BorderStyle::Dotted;
        b.right.style = BorderStyle::Dashed;
        pg.styles.borders.append(b);
        CellFormat cf;
        cf.border = 1;
        pg.styles.formats.append(cf);
        pg.styles.cellXfs.append(0);
        const quint32 dotted = quint32(pg.styles.formats.size() - 1);
        for (quint32 r = 0; r < 40; ++r)
            for (quint32 c = 0; c < 10; ++c) pg.cell(r, c, dotted);
        pg.open();
        const SheetPrint p = pg.print();
        QCOMPARE(p.pages.size(), size_t(1));
        QByteArray bytes;
        {
            QBuffer buffer(&bytes);
            QVERIFY(buffer.open(QIODevice::WriteOnly));
            QPdfWriter pdf(&buffer);
            pdf.setResolution(1200);
            pdf.setPageSize(QPageSize(QSizeF(p.paper.width, p.paper.height), QPageSize::Point));
            pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
            QPainter qp(&pdf);
            qp.scale(1200 / 72.0, 1200 / 72.0);
            pg.painter->paint(&qp, p, p.pages[0], pg.setup, jp::office::sheet::HfContext(), true, nullptr);
        }
        QVERIFY2(bytes.size() > 1000, qPrintable(QString::number(bytes.size())));
        QVERIFY2(bytes.size() < 60000, qPrintable(QString::number(bytes.size())));   // (a mark for each dash made it 180 kilobytes: over fifty thousand marks)
        // the borders are there on an image (a hair of a line at this scale, so grey, not black): along the bottom of row 3, and down the right of column 3
        const QImage img = pg.render(p, 0);
        int inked = 0;
        const int y = int((54 + 45) * kPx) - 1;
        for (int x = int((50.4 + 10) * kPx); x < int((50.4 + 40) * kPx); ++x) inked += !isWhite(img.pixel(x, y));
        QVERIFY2(inked > 10, qPrintable(QString::number(inked)));
        int inkedDown = 0;
        const int x = int((50.4 + 144) * kPx) - 1;
        for (int row = int((54 + 15) * kPx); row < int((54 + 45) * kPx); ++row) inkedDown += !isWhite(img.pixel(x, row)) || !isWhite(img.pixel(x + 1, row));
        QVERIFY2(inkedDown > 10, qPrintable(QString::number(inkedDown)));
    }

    // A page's drawing is the work of the cells on it: counted, for a sheet of a million rows.
    void aPageCostsTheCellsOnIt()
    {
        Page pg;
        pg.cell(0, 0);
        pg.cell(1048575, 3);
        pg.open();
        pg.setup.rowBreaks = {};
        pg.setup.printArea = {CellRange{900000, 0, 900044, 9, true}};
        const SheetPrint p = pg.print();
        QCOMPARE(p.pages.size(), size_t(1));
        pg.render(p, 0);
        QVERIFY(pg.painter->stats().rowsVisited <= 50);
        QVERIFY(pg.painter->stats().colsVisited <= 12);
    }
};

JP_TEST_CLASS(GridPrintPaintTests)
#include "tst_grid_printpaint.moc"
