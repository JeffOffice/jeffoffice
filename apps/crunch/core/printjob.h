#pragma once
// A printing job: which sheets of a workbook print and how, cut into pages (grid/printlayout.h), numbered through the job, and drawn on any painter in points (grid/printpaint.h), so
// the preview on the Print screen, the PDF, and the printer show the same pages. The job reads the workbook through a PrintSource (the window's, or a tool's), takes the page setup of
// each sheet and puts what the person chose for this job over it (the paper, the margins, the scaling), and keeps the grid models of a few sheets at a time for drawing.
//
// Bounded: the pages of a job (kMaxJobPages; a sheet that needs more is cut short and the job says so), and the grids kept (the sheet being drawn and the one before).

#include "app/printsides.h"
#include "core/anchors.h"
#include "core/loader.h"
#include "core/pagesettings.h"
#include "core/printobjects.h"
#include "grid/gridsheet.h"
#include "grid/printlayout.h"
#include "grid/printpaint.h"
#include "grid/valuestore.h"
#include "io/pdfbookmarks.h"
#include "io/pdfcrypt.h"
#include "io/pdftags.h"

#include <QDateTime>
#include <QImage>
#include <QPrinter>
#include <QStringList>

#include <functional>
#include <map>
#include <unordered_map>
#include <memory>
#include <vector>

class QPainter;
class QPrinter;

namespace jp::crunch {

class SheetObjects;


constexpr int kMaxJobPages = 100000;
// The most pixels a preview image has (16 million: 64 MB), whatever the zoom and the screen; a page asked for larger is drawn with fewer pixels to the point.
constexpr qint64 kMaxPreviewPixels = 16000000;
// The most pages sent to a printer in one job, copies counted (a hundred thousand pages in a hundred copies is not a print job).
constexpr qint64 kMaxSentPages = 2000000;

enum class PrintWhat { ActiveSheet, Workbook, Selection };
enum class PrintScaling { AsSheet, NoScaling, FitSheet, FitColumns, FitRows };

struct PrintJobOptions {
    PrintWhat what = PrintWhat::ActiveSheet;
    int activeSheet = 0;
    QVector<int> sheets;                        // for ActiveSheet, when more than one sheet tab is picked: all of them print, in order
    jp::office::sheet::CellRange selection;     // for Selection, on the active sheet
    bool ignorePrintArea = false;
    bool grayscale = false;                     // every page is painted in shades of gray, in the preview, on paper, and in a PDF
    bool archival = false;                      // for a PDF: PDF/A-1b, the form for keeping (every font inside, no transparency)
    bool pdfTags = false;                       // for a PDF: tagged for screen readers (each page's cells are a table)
    bool hasPdfInfo = false;                    // for a PDF: the document's own properties are in `pdfInfo` (the window makes them; none: the file's name and this program)
    jp::PdfInfo pdfInfo;
    // what the person chose for this job, over what the sheets say
    bool setOrientation = false;
    jp::office::sheet::PageOrientation orientation = jp::office::sheet::PageOrientation::Default;
    bool setPaper = false;
    int paperSize = 1;
    bool setMargins = false;
    jp::office::sheet::PageMargins margins;
    PrintScaling scaling = PrintScaling::AsSheet;
    int scaleToPaper = 0;                       // a paper size (the format's number) each page is scaled to fit onto, in its proportions and centered (0: pages print on their own paper)
    QDateTime now;                              // what the date and time fields show (now, if not set)
    std::function<QDateTime()> clock;           // where "now" is read from when `now` is not set, once per job (the system's clock if none)
    QString fileName, folder;                   // what the file name and folder fields show
    int maxPages = kMaxJobPages;
};

// Printing on both sides of the paper by hand: the odd pages of the job (1, 3, 5, ...) are printed, the stack is turned over, and then the even pages are.
enum class PrintSides : quint8 { All, Odd, Even };

// What the Print screen holds: the job's choices, and what is to be done with its pages.
struct PrintChoices {
    PrintJobOptions job;
    int pagesPerSheet = 1;      // 1, 2, 4, 6, 9, or 16 pages on each sheet of paper
    QString pageRange;          // "1-3, 5"; empty for all
    PrintSides sides = PrintSides::All;   // all the pages, or only the odd ones or the even ones (by page number)
    int copies = 1;
    bool collate = true;
    bool reverse = false;       // the last page first
    QString printer;            // the printer's name; empty for the default
    bool toFile = false;        // a PDF file, not a printer
    bool byHand = false;        // both sides by hand: the first, third, ... pages, a message to turn the stack over, then the others last first (never with a file)
    int duplexChoice = 0;       // the Sides choice as it was made (a jp::SidesChoice), kept for the next time the page is opened
    QPrinter::DuplexMode duplex = QPrinter::DuplexNone;   // what the printer is set to: both sides on the long or the short edge (never with a file, and never where the printer can't)
};

// A note on a cell, for the pages of notes that print at the end of a sheet.
struct PrintNote {
    quint32 row = 0, col = 0;
    QString author, text;
    bool shown = false;                     // the note is shown on the sheet (its box prints with the cells when the comments print as displayed)
    jp::office::sheet::NoteBox box;         // where its box is, if the file says
};

// A link on cells, for the PDF: the cells, and where it goes (an address outside the workbook, or a place in it such as "Sheet2!B5").
struct PrintLink {
    jp::office::sheet::CellRange range;
    QString target, location;
};

// What a job reads of a workbook.
class PrintSource {
public:
    virtual ~PrintSource() = default;
    virtual int sheetCount() const = 0;
    virtual QString sheetName(int sheet) const = 0;
    // A sheet of cells or a chart sheet, and whether it is hidden: the whole workbook leaves the hidden ones out.
    virtual bool printable(int sheet) const = 0;
    virtual bool hidden(int sheet) const = 0;
    // A chart sheet: one page, with its chart fitted to the page's room (it has no cells).
    virtual bool isChartSheet(int sheet) const { Q_UNUSED(sheet); return false; }
    virtual jp::grid::GridData data(int sheet) const = 0;
    virtual jp::office::sheet::PageSetup page(int sheet) const = 0;
    virtual std::vector<jp::grid::LayerObject> objects(int sheet) = 0;
    // The notes on the cells of a sheet, in the order of the cells (down the rows, across each).
    virtual std::vector<PrintNote> notes(int sheet) const { Q_UNUSED(sheet); return {}; }
    // The links on the cells of a sheet (those a person made, and the cells whose formula is HYPERLINK with an address typed in it).
    virtual std::vector<PrintLink> links(int sheet) const { Q_UNUSED(sheet); return {}; }
    // What a defined name stands for ("Sheet2!$A$1:$B$2"), of the sheet's own, else the workbook's; empty for none or a name that is not a reference.
    virtual QString nameText(int sheet, const QString &name) const { Q_UNUSED(sheet); Q_UNUSED(name); return {}; }
    // The font the header and footer begin in.
    virtual QString baseFontFamily() const = 0;
    virtual double baseFontPoints() const = 0;
};

// The source made of a workbook the window (or a tool) has open: the page setups the session holds (else the sheets' own), the values the worker told (else the calculation's, read
// directly), and the objects SheetObjects lists. None of the pointers is owned.
class BookPrintSource : public PrintSource {
public:
    BookPrintSource(LoadedBook *book, const jp::grid::ValueStore *values, const PageSettings *pages, SheetObjects *objects, const CellAnchors *anchors = nullptr)
        : m_book(book), m_values(values), m_pages(pages), m_objects(objects), m_anchors(anchors) {}
    int sheetCount() const override;
    QString sheetName(int sheet) const override;
    bool printable(int sheet) const override;
    bool hidden(int sheet) const override;
    bool isChartSheet(int sheet) const override;
    jp::grid::GridData data(int sheet) const override;
    jp::office::sheet::PageSetup page(int sheet) const override;
    std::vector<jp::grid::LayerObject> objects(int sheet) override;
    std::vector<PrintNote> notes(int sheet) const override;
    std::vector<PrintLink> links(int sheet) const override;
    QString nameText(int sheet, const QString &name) const override;
    QString baseFontFamily() const override;
    double baseFontPoints() const override;

private:
    LoadedBook *m_book;
    const jp::grid::ValueStore *m_values;
    const PageSettings *m_pages;
    SheetObjects *m_objects;
    const CellAnchors *m_anchors;
};

class PrintJob {
public:
    PrintJob(std::unique_ptr<PrintSource> source, const PrintJobOptions &options);
    ~PrintJob();

    // Cuts the sheets into pages. False, with a message, when there is nothing to print.
    bool build(QString *why = nullptr);
    // The same in slices, so that a sheet of a million rows does not hold the window: begin (false, with a message, when there is nothing to print), then step with a fresh Work (see
    // grid/axis.h) for each turn until it says Done or Failed. The pages are the job's only when it is Done. The job must not be used for anything else, and the workbook must not be
    // closed or replaced, between the slices; one that is let go of is simply let go of.
    enum class BuildState { Working, Done, Failed };
    bool beginBuild(QString *why = nullptr);
    BuildState buildStep(jp::grid::Work &work, QString *why = nullptr);
    int pageCount() const { return int(m_pages.size()); }
    // Words about the job: a sheet that was cut short, a fit that makes the type small, a sheet with nothing on it.
    const QStringList &messages() const { return m_messages; }

    // A page: its paper in points, the sheet it is of, its place among that sheet's pages, and the number its fields show.
    QSizeF pageSize(int page) const;
    int sheetOf(int page) const;
    int numberOf(int page) const;
    QString sheetNameOf(int page) const;
    // The cut of a sheet that is in the job (null if it is not), and the setup it prints with.
    const jp::grid::SheetPrint *printOf(int sheet) const;
    const jp::office::sheet::PageSetup *setupOf(int sheet) const;
    // The first page of a sheet in the job's list, or -1.
    int firstPageOf(int sheet) const;
    // The lines of a page of notes (a sheet whose comments print at the end has them after its last page), or none for any other page.
    QStringList noteLines(int page) const;

    // Where the cells of a page are, for a view of the pages: the cell's place on the page in points from the paper's corner (false if the page does not show it), the cell at a point of the paper
    // (false if the point is not on a cell of the page), the cells' own area, the range's part on the page, and the first page that shows a cell.
    bool cellRect(int page, qint64 row, qint64 col, QRectF *rect);
    bool cellAt(int page, const QPointF &point, qint64 *row, qint64 *col);
    bool rangeRect(int page, qint64 row1, qint64 col1, qint64 row2, qint64 col2, QRectF *rect);
    // The boxes a range has on a page, in points from its top left: one for the part among the page's own cells and one for each part among the titles repeated on it.
    std::vector<QRectF> linkRects(int page, qint64 row1, qint64 col1, qint64 row2, qint64 col2);
    int pageOfCell(int sheet, qint64 row, qint64 col) const;
    // What the preview of a page lets a person drag: the area of its cells (points from the page's top left), the sheet it is of, the scale it prints at, and the right edge of each column it
    // shows (at most kMostColumnEdges, the first ones). False for a page with no cells (a chart, the notes).
    static constexpr int kMostColumnEdges = 64;
    struct ColumnEdge {
        qint64 col = 0;
        double left = 0, right = 0;
    };
    struct PreviewGuides {
        int sheet = -1;
        int scalePercent = 100;
        QRectF cells;
        std::vector<ColumnEdge> columns;
        bool rightToLeft = false;   // the page is mirrored: a column's far edge (the one a drag widens it by) is its left
    };
    bool guidesOf(int page, PreviewGuides *out);
    // The edges of a column on a page (points from its top left), from the page's column axis (a merged cell over the column is wider, and is not what a column's width is).
    bool columnEdges(int page, qint64 col, double *left, double *right);
    // The width the file gives a column of a sheet (characters; -1 when it gives none), the width it has when it is `pixels` wider (printer pixels, whole or not: the sheet's own units, counted
    // from the width the file has), and the printer pixels a width on a page printed at `scalePercent` comes to.
    double columnFileWidth(int sheet, qint64 col) const;
    double fileWidthAtOffset(int sheet, double fileWidth, double pixels) const;
    static double printerPixelsFor(int scalePercent, double pagePoints);
    // How many times the links of a PDF asked for the boxes of a range on a page (counted, for tests).
    qint64 linkLookups() const { return m_linkLookups; }
    // The test sets it to make the writing of a PDF's properties fail (a file that is made in a way this cannot extend), which the program never does.
    static void failPropertiesForTest(bool on);
    // The width in characters a column of the sheet has when it is `pointsOnPage` wide on a page printed at `scalePercent`, and the points it is wide on such a page when it has this width.
    double fileWidthForPoints(int sheet, int scalePercent, double pointsOnPage) const;
    double pointsOnPageFor(int sheet, int scalePercent, double fileWidth) const;
    // Whether the margins a page prints with are the ones chosen for this job (not the sheet's own, which a drag on the preview would change).
    bool marginsChosen() const { return m_options.setMargins; }
    // The paper's box for the page (its margins and the header and footer distances), in points; the sheet's setup the page prints with.
    jp::grid::PaperBox paperOf(int page) const;
    const jp::office::sheet::PageSetup *setupOfPage(int page) const;
    bool isFirstOfSheet(int page) const;

    // For a job a view of the pages draws from: pictures in headers and footers are made on workers (a page is drawn without those not ready, and picturesPending() says whether some are being made),
    // and the screen's gridlines and headings are drawn (the paper never has them unless its setup says so). prefetchPictures() begins the pictures of the job's sheets.
    void setAsyncPictures(bool on);
    void setScreenMarks(bool gridlines, bool headings);
    bool picturesPending() const;
    // Whether the page drawn last lacked a picture that was not ready.
    bool lastPaintMissedPictures() const { return m_lastMissed; }
    void prefetchPictures();
    // How many times the contexts worked out a page's place (counted, for tests).
    qint64 layoutsWorkedOut() const;
    // PDF/A: how many see-through pictures a page may be given the pixels beneath it of one at a time (the page is painted for each); past this the page's cells are painted once and each picture is
    // given its part of that. How many pages were painted so far for these pixels (counted, for tests).
    static constexpr int kMostBackdropsPerPage = 50;
    qint64 backdropPaints() const { return m_backdropPaints; }
    // What the painter did for the page drawn last (cells looked at and the like): counted, for tests.
    const jp::grid::PaintStats &lastPaintStats() const { return m_lastStats; }

    // Draws a page in points on the painter (its origin the paper's corner). Gives false if the page is not one.
    bool paint(QPainter *painter, int page);
    // The test sets it to make the gray pictures fail (a machine with no room), which the program never does.
    static void setGrayPixelLimitForTest(double pixels);
    bool paintColors(QPainter *painter, int page);   // (the page as the sheet colors it; paint() makes it gray when the job is)
    // The paper size and the painting of a group of pages that share a sheet (see `perSheet` below).
    QSizeF sheetFor(const QVector<int> &group, int perSheet) const;
    void paintGroup(QPainter *painter, const QVector<int> &group, const QSizeF &sheet, int perSheet);
    // The sheet of paper a group makes, as a picture of the width (its height follows), on white: what the printer or the PDF gets for that group.
    QImage previewSheet(const QVector<int> &group, int perSheet, int widthPixels);
    // The page as a picture of the width (its height follows), on white.
    QImage preview(int page, int widthPixels);

    // The pages (indexes from 0, in the order given) as a PDF file, written at once; `progress` is told the pages done and the total, and says false to stop. The file is not there if it fails. With `protect` (its open password set) the PDF is made in memory and the file written once, encrypted.
    bool exportPdf(const QString &path, const QVector<int> &pages, QString *error = nullptr, const std::function<bool(int done, int total)> &progress = nullptr, int perSheet = 1,
                   const jp::PdfProtection *protect = nullptr, const QVector<QVector<int>> *sheets = nullptr);
    // The pages to a printer (any QPrinter, a file among them), one sheet of paper each; the printer's own copies are not used here (the caller repeats the pages).
    // `copies` of the pages are sent, each set together (collated) or each page's copies together; never made into a list (the number sent is held to kMaxSentPages).
    bool printTo(QPrinter *printer, const QVector<int> &pages, QString *error = nullptr, const std::function<bool(int done, int total)> &progress = nullptr, int copies = 1,
                 bool collate = true, int perSheet = 1, const QVector<QVector<int>> *sheets = nullptr);
    // With `sheets` the sheets are given (each a list of the pages that share it, an empty one is a blank sheet) and `pages` is not used: for both sides by hand.
    // `perSheet` (1, 2, 4, 6, 9, or 16) puts that many of the pages on each sheet of paper, each shrunk to fit its place in a grid and framed (the paper is that of the first page of the
    // sheet, turned to lie on its side for 2 and 6). Any other number is 1.
    // Whether the last export or print was stopped by its progress function (not a failure: nothing was written or sent past that point).
    bool wasCancelled() const { return m_cancelled; }
    // The date and time the date and time fields show on every page (taken when the job was made, or the moment the options gave).
    QDateTime moment() const { return m_now; }

private:
    struct SheetInfo {
        int sheet = 0;
        jp::office::sheet::PageSetup setup;
        jp::grid::SheetPrint print;
        std::shared_ptr<const jp::grid::GridGeometry> geometry;
        bool chart = false;                     // a chart sheet: one page with the chart over the whole of its room
        jp::grid::Axis cols;                    // the columns as they are on paper, in the grid's pixels (what the page is drawn by)
        jp::grid::Axis rows;                    // the rows as they are on paper, the same way
        int firstNumber = 1;
        int firstPage = 0;
        std::vector<PrintNote> shownNotes;      // the notes shown on the sheet, for comments printed as displayed: their boxes are drawn over the cells
        std::vector<QStringList> notePages;     // the lines of the pages of notes after the sheet's pages (comments printed at the end)
    };
    struct Context {
        int sheet = -1;
        std::unique_ptr<jp::grid::GridSheet> grid;
        std::unique_ptr<jp::grid::PagePainter> painter;
        std::unique_ptr<PageObjectPainter> objects;
        quint64 used = 0;
    };
    struct Page {
        int info = 0;       // the place in m_sheets
        int index = 0;      // the place among the sheet's pages
        int number = 1;
        int note = -1;      // for a page of notes, its place among the sheet's pages of notes
    };
    struct Build;
    // Where a page is on the sheet of the PDF: the corner of its box (points from the sheet's top left), its scale, and the sheet's height.
    struct PdfPlace {
        double x = 0, y = 0, k = 1, sheetHeight = 0;
    };
    void placeOnSheet(const QVector<int> &group, const QSizeF &sheet, int perSheet, QVector<PdfPlace> *places) const;
    // The bookmarks of a PDF of the pages: one for each sheet of the workbook that has a page in it, going to the PDF page its first page is on, at most jp::kMostPdfBookmarks.
    QVector<jp::PdfBookmark> pdfBookmarks(const QVector<int> &pdfPageOf);
    // The links of the pages as the PDF holds them: `pdfPageOf` is the PDF's page for each page of the job (-1: not in the PDF) and `places` where each page is put on its sheet. At most kMostPdfLinks.
    // The properties of a PDF file: the document's own when the job was given them, else the file's name, this program, and the time.
    jp::PdfInfo pdfInfoForFile() const;
    qint64 m_linkLookups = 0;
    // What the links left out: past the most a PDF holds, or for an address longer than the longest, and whether the work was stopped (`keepGoing` says no).
    struct PdfLinksLeftOut {
        bool cut = false;
        qint64 tooLong = 0;
        bool cancelled = false;
    };
    QVector<jp::PdfLink> pdfLinks(const QVector<int> &pdfPageOf, const QVector<PdfPlace> &places, const std::function<bool()> &keepGoing = nullptr, PdfLinksLeftOut *left = nullptr);
    int firstPdfPageOf(int sheet, const QVector<int> &pdfPageOf) const;
    // The range with the merged cells it touches taken in whole (a merged cell is boxed over the whole of its area).
    jp::office::sheet::CellRange withMerges(int info, jp::office::sheet::CellRange range);
    // The pages of a sheet's print (a place in m_sheets) by the rows and columns they show, made once: a link is asked only of the pages that can show its cells.
    struct PageIndex {
        struct Band {
            qint64 first = 0, last = -1;
        };
        struct Block {
            std::vector<Band> rowBands, colBands;   // by band number (the pages' own lines)
            qint64 titleRow1 = -1, titleRow2 = -1, titleCol1 = -1, titleCol2 = -1;
            std::unordered_map<quint64, int> pages;   // (row band, column band) to the job's page
        };
        std::vector<Block> blocks;   // by block of the print
        bool regular = true;         // the pages are a grid of bands, each band the same on every page it is on (when not, the pages are looked over one by one)
    };
    const PageIndex &pageIndexOf(int info) const;
    // The job's pages of the print that can show a cell of the range (its own lines, and with `titles` the titles repeated on a page), in order; false if `tick` said to stop.
    bool pagesShowing(int info, qint64 row1, qint64 col1, qint64 row2, qint64 col2, bool titles, const std::function<bool()> &tick, std::vector<int> *out) const;
    mutable std::vector<PageIndex> m_pageIndex;
    mutable bool m_indexed = false;
    bool shownRect(int page, qint64 r1, qint64 c1, qint64 r2, qint64 c2, QRectF *rect);
    Context &contextFor(int infoIndex);
    // The painters of a sheet (cells and, unless `forBackdrop`, objects).
    void fillContext(Context &c, const SheetInfo &info, bool forBackdrop);
    void paintBeneath(QPainter *painter, int page, PageObjectPainter *objects, int stopAt);
    jp::office::sheet::HfContext labelsOf(const Page &pg, const SheetInfo &info) const;
    const SheetInfo *infoOfSheet(int sheet) const;

    std::unique_ptr<PrintSource> m_source;
    PrintJobOptions m_options;
    std::vector<SheetInfo> m_sheets;
    std::vector<Page> m_pages;
    QStringList m_messages;
    std::vector<Context> m_contexts;
    bool m_asyncPictures = false, m_screenGrid = false, m_screenHeadings = false;
    jp::grid::PaintStats m_lastStats;
    bool m_lastMissed = false;
    quint64 m_tick = 0;
    bool m_cancelled = false;
    Context m_backdropContext;   // PDF/A: the cells of a sheet painted apart from the page in hand
    qint64 m_backdropPaints = 0;   // pages painted into the pixels beneath see-through pictures (PDF/A)
    bool m_grayInPdf = false;  // a PDF is made gray by its own color model: the pages are painted in color as they are
    bool m_inGray = false;     // a page is being painted in color into the pixels that will be made gray
    QDateTime m_now;
    int m_totalNumbers = 0;
    std::unique_ptr<Build> m_build;
    jp::PdfTagger *m_tagger = nullptr;   // set while a PDF is made that is to be tagged
    int m_artifactTag = -1;
};

// The pages (from 0) that text such as "1-3, 5" names, once each, in the order given; "8-" goes to the last and "-2" from the first. A part that names no page is passed by. Empty when none is named.
QVector<int> parsePageRange(const QString &text, int pageCount);
// The pages (places from 0, so page 1 is 0) that are of the side: the odd-numbered pages, or the even-numbered ones; all of them for All.
QVector<int> pagesOfSide(const QVector<int> &pages, PrintSides sides);
// Both sides by hand: the pages shared out over physical sheets (`perSheet` pages to a sheet, the last sheet possibly short), as the two runs of sheets. The first run is the
// first, third, ... sheets; the second is the others last first, with an empty sheet (a blank one) first when the last front has no back.
HandRuns<QVector<int>> handSheetRuns(const QVector<int> &pages, int perSheet);
// The pages cut into the sheets of paper that carry them (`perSheet` pages to a sheet, the last perhaps short), in order.
QVector<QVector<int>> sheetGroups(const QVector<int> &pages, int perSheet);
// The pages in the order the sheets come out for `copies` of them: collated, the whole set again for each copy (1 2 3 1 2 3), else each page's copies together (1 1 2 2 3 3).
QVector<int> sheetOrder(const QVector<int> &pages, int copies, bool collate);

} // namespace jp::crunch
