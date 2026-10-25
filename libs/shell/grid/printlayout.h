#pragma once
// Cutting a sheet into the pages it prints on: no drawing, and no window. From the sheet's cells, the sizes of its rows and columns, and its page setup this works out the part of the
// sheet that prints (the print area, or the cells that have anything to show, and the objects that print), the scale (a percent, or the largest that fits the pages asked for), and the
// rows and columns of each page: the paper less its margins, whole rows and columns only, hidden ones skipped, a break wherever a person put one, the titles repeated on every page after
// the one they are on, and the pages in the order the setup says. A sheet that needs more pages than the bound is cut short, and says so.
//
// Sizes are the grid's own: a column is as many pixels (at 96 to the inch) as GridSheet::columnPixels makes of its width, a row as many as rowPixels makes of its height, and a pixel is
// three quarters of a point, so the page breaks fall where the screen's sizes say.

#include "grid/axis.h"
#include "grid/objectlayer.h"
#include "sheet/pagesetup.h"
#include "sheet/sheetmodel.h"
#include "sheet/styles.h"

#include <QRectF>
#include <QString>

#include <memory>
#include <vector>

namespace jp::grid {

using jp::office::sheet::CellRange;
using jp::office::sheet::PageSetup;
using jp::office::sheet::Sheet;
using jp::office::sheet::Styles;

// A pixel of the grid at 100%, in points.
constexpr double kPointsPerPixel = 0.75;
// The most pages one sheet is cut into; a sheet that needs more has its first pages and a message.
constexpr int kMaxSheetPages = 50000;
// The scale of a page that is fitted to pages never goes above this, in percent (the sheet is made smaller to fit, not larger).
constexpr int kFitCeiling = 100;
// Row headings (the numbers) and column headings (the letters) when they are printed: the width of the first and the height of the second, in the grid's pixels. They take room from the page.
constexpr int kRowHeadingPixels = 36;
constexpr int kColumnHeadingPixels = 20;

// The resolution a page is laid out at: the printer's, not the screen's. A column's width in characters is counted in the width of the widest digit of the workbook's font, and on the
// screen that is a whole number of pixels at 96 to the inch (7 for the usual fonts: 5.25 points), while on paper it is a whole number of printer pixels (46 at 600 to the inch: 5.52
// points), so a column is a few percent wider on paper than on the screen and a page holds fewer of them.
constexpr int kPrinterDpi = 600;
// The width of the widest digit of the workbook's first font in printer pixels (at least 1).
int printDigitPixels(const Styles &styles, int dpi = kPrinterDpi);
// A column's width on paper in points: the file's width in characters (with its padding) is cut to whole printer pixels as the format says, then turned to points.
double printColumnPoints(double fileWidth, int digitPixels, int dpi = kPrinterDpi);
// The sizes of the sheet's columns on paper, in 1/`fine` of the grid's pixels (a pixel is three quarters of a point; each column is held to the nearest of those), for the page layout to
// cut by (a fine axis cuts to a small part of a point, where whole pixels would be wrong by a third of a point in every column) and, with `fine` 1, for the painter to draw with;
// hidden columns have none.
Axis printColumnAxis(const Sheet &sheet, const Styles &styles, int fine = 1, int dpi = kPrinterDpi);
// The sizes of the sheet's rows, the same way: the height in points the file gives, in 1/`fine` of a pixel (with `fine` 1, the rows the grid has).
Axis printRowAxis(const Sheet &sheet, int fine = 1);
// The same, made a step at a time (see Work): `step` goes on until the work is spent and says whether the axis is done; `take` is the axis once it is.
class RowAxisTask {
public:
    explicit RowAxisTask(const Sheet &sheet, int fine = 1);
    ~RowAxisTask();
    bool step(Work &work);
    Axis take();

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};
// The finer axes the pages are cut by, as many to the pixel as that.
constexpr int kCutFine = 8;

// What a column or row comes to on paper, in points, exactly as the screen's sizes make it: the file's width in characters of the widest digit, with the padding the file adds, in pixels
// (cut as the format says) and then in points; and a height in points through the pixels it is drawn in.
double columnPoints(double fileWidth, int maxDigitWidth);
double rowPoints(double heightInPoints);

// The paper a setup puts the sheet on, in points: its size turned as the orientation says, and the margins held to what the paper can have (not negative, and leaving room for a page).
struct PaperBox {
    double width = 612, height = 792;
    double left = 50.4, top = 54, right = 50.4, bottom = 54;
    double header = 21.6, footer = 21.6;   // the header's distance from the top edge, and the footer's from the bottom edge
    QRectF content() const { return QRectF(left, top, width - left - right, height - top - bottom); }
};
PaperBox paperBox(const PageSetup &setup);

// One page of a sheet: the rows and columns of its body, and the rows and columns of the titles that print above and to the left of it (-1 for none).
struct PageCut {
    int area = 0;                             // which of the printed blocks it is of
    qint64 row1 = 0, row2 = -1, col1 = 0, col2 = -1;
    qint64 titleRow1 = -1, titleRow2 = -1, titleCol1 = -1, titleCol2 = -1;
    int rowBand = 0, colBand = 0;             // its place among the pages of its block, counted from 0
    int rowBands = 1, colBands = 1;
};

struct SheetPrint {
    PaperBox paper;
    std::vector<CellRange> areas;             // the blocks printed (after hidden rows and columns at their ends are taken off), in the order they print
    std::vector<PageCut> pages;               // in the order they print
    int scalePercent = 100;                   // what the sheet is printed at
    bool fitted = false;                      // the scale was worked out to fit the pages asked for
    bool tooSmall = false;                    // it fits only at less than the smallest scale (the sheet is printed at that, on more pages than asked for)
    bool truncated = false;                   // the sheet needs more pages than the bound: these are the first
    qint64 pagesNeeded = 0;                   // the pages the whole sheet needs (counted, not all made, when truncated)
    bool headings = false;                    // the headings are printed
};

struct PrintRequest {
    const Sheet *sheet = nullptr;
    const Styles *styles = nullptr;
    const Axis *rows = nullptr;               // the sheet's row and column sizes (copied; skipped rows are not counted)
    const Axis *cols = nullptr;
    int rowFine = 1, colFine = 1;             // how many of the axes' units make a grid pixel (the sizes above are in them; the objects' boxes and the headings are in pixels)
    PageSetup setup;                          // what the job prints with (the sheet's own, with what the person chose for the job on top)
    std::vector<ObjectBox> objects;           // the boxes of the objects that print, in the grid's pixels from the first row and column
    bool ignorePrintArea = false;
    bool onlySelection = false;               // print this block and nothing else
    CellRange selection;
    int maxPages = kMaxSheetPages;
    int fitCeiling = kFitCeiling;
};

// The blocks of the sheet that print: the print area's blocks (each held to the sheet, hidden ends taken off), else the block from the first cell to the last that has a value, a formula, a
// border or fill that shows, or an object over it. None when the sheet has nothing to print.
std::vector<CellRange> printedBlocks(const PrintRequest &request);

SheetPrint paginate(const PrintRequest &request);

// The same, a step at a time: `step` goes on until the work is spent and says whether the cut is done (the request's axes, sheet, and styles stay as they are until it is); `take` is
// the result. A task stepped to its end with no limit gives what paginate gives, as paginate is exactly that.
class PaginateTask {
public:
    explicit PaginateTask(const PrintRequest &request);
    ~PaginateTask();
    bool step(Work &work);
    SheetPrint take();

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace jp::grid
