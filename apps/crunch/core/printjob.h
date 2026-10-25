#pragma once
// A printing job: which sheets of a workbook print and how, cut into pages (grid/printlayout.h), numbered through the job, and drawn on any painter in points (grid/printpaint.h), so
// the preview on the Print screen, the PDF, and the printer show the same pages. The job reads the workbook through a PrintSource (the window's, or a tool's), takes the page setup of
// each sheet and puts what the person chose for this job over it (the paper, the margins, the scaling), and keeps the grid models of a few sheets at a time for drawing.
//
// Bounded: the pages of a job (kMaxJobPages; a sheet that needs more is cut short and the job says so), and the grids kept (the sheet being drawn and the one before).

#include "core/loader.h"
#include "core/pagesettings.h"
#include "core/printobjects.h"
#include "grid/gridsheet.h"
#include "grid/printlayout.h"
#include "grid/printpaint.h"
#include "grid/valuestore.h"

#include <QDateTime>
#include <QImage>
#include <QStringList>

#include <functional>
#include <map>
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
    jp::office::sheet::CellRange selection;     // for Selection, on the active sheet
    bool ignorePrintArea = false;
    // what the person chose for this job, over what the sheets say
    bool setOrientation = false;
    jp::office::sheet::PageOrientation orientation = jp::office::sheet::PageOrientation::Default;
    bool setPaper = false;
    int paperSize = 1;
    bool setMargins = false;
    jp::office::sheet::PageMargins margins;
    PrintScaling scaling = PrintScaling::AsSheet;
    QDateTime now;                              // what the date and time fields show (now, if not set)
    std::function<QDateTime()> clock;           // where "now" is read from when `now` is not set, once per job (the system's clock if none)
    QString fileName, folder;                   // what the file name and folder fields show
    int maxPages = kMaxJobPages;
};

// What the Print screen holds: the job's choices, and what is to be done with its pages.
struct PrintChoices {
    PrintJobOptions job;
    QString pageRange;          // "1-3, 5"; empty for all
    int copies = 1;
    bool collate = true;
    QString printer;            // the printer's name; empty for the default
    bool toFile = false;        // a PDF file, not a printer
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
    // The font the header and footer begin in.
    virtual QString baseFontFamily() const = 0;
    virtual double baseFontPoints() const = 0;
};

// The source made of a workbook the window (or a tool) has open: the page setups the session holds (else the sheets' own), the values the worker told (else the calculation's, read
// directly), and the objects SheetObjects lists. None of the pointers is owned.
class BookPrintSource : public PrintSource {
public:
    BookPrintSource(LoadedBook *book, const jp::grid::ValueStore *values, const PageSettings *pages, SheetObjects *objects) : m_book(book), m_values(values), m_pages(pages), m_objects(objects) {}
    int sheetCount() const override;
    QString sheetName(int sheet) const override;
    bool printable(int sheet) const override;
    bool hidden(int sheet) const override;
    bool isChartSheet(int sheet) const override;
    jp::grid::GridData data(int sheet) const override;
    jp::office::sheet::PageSetup page(int sheet) const override;
    std::vector<jp::grid::LayerObject> objects(int sheet) override;
    QString baseFontFamily() const override;
    double baseFontPoints() const override;

private:
    LoadedBook *m_book;
    const jp::grid::ValueStore *m_values;
    const PageSettings *m_pages;
    SheetObjects *m_objects;
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
    int indexInSheet(int page) const;
    int numberOf(int page) const;
    QString sheetNameOf(int page) const;
    // The cut of a sheet that is in the job (null if it is not), and the setup it prints with.
    const jp::grid::SheetPrint *printOf(int sheet) const;
    const jp::office::sheet::PageSetup *setupOf(int sheet) const;
    int pagesOfSheet(int sheet) const;
    // The first page of a sheet in the job's list, or -1.
    int firstPageOf(int sheet) const;

    // Draws a page in points on the painter (its origin the paper's corner). Gives false if the page is not one.
    bool paint(QPainter *painter, int page);
    // The page as a picture of the width (its height follows), on white.
    QImage preview(int page, int widthPixels);

    // The pages (indexes from 0, in the order given) as a PDF file, written at once; `progress` is told the pages done and the total, and says false to stop. The file is not there if it fails.
    bool exportPdf(const QString &path, const QVector<int> &pages, QString *error = nullptr, const std::function<bool(int done, int total)> &progress = nullptr);
    // The pages to a printer (any QPrinter, a file among them), one sheet of paper each; the printer's own copies are not used here (the caller repeats the pages).
    // `copies` of the pages are sent, each set together (collated) or each page's copies together; never made into a list (the number sent is held to kMaxSentPages).
    bool printTo(QPrinter *printer, const QVector<int> &pages, QString *error = nullptr, const std::function<bool(int done, int total)> &progress = nullptr, int copies = 1,
                 bool collate = true);
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
        int firstNumber = 1;
        int firstPage = 0;
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
    };
    struct Build;
    Context &contextFor(int infoIndex);
    SheetInfo *infoOfSheet(int sheet);
    const SheetInfo *infoOfSheet(int sheet) const;

    std::unique_ptr<PrintSource> m_source;
    PrintJobOptions m_options;
    std::vector<SheetInfo> m_sheets;
    std::vector<Page> m_pages;
    QStringList m_messages;
    std::vector<Context> m_contexts;
    quint64 m_tick = 0;
    bool m_cancelled = false;
    QDateTime m_now;
    int m_totalNumbers = 0;
    std::unique_ptr<Build> m_build;
};

// The pages (from 0) that text such as "1-3, 5" names, once each, in the order given; "8-" goes to the last and "-2" from the first. A part that names no page is passed by. Empty when none is named.
QVector<int> parsePageRange(const QString &text, int pageCount);
// The pages in the order the sheets come out for `copies` of them: collated, the whole set again for each copy (1 2 3 1 2 3), else each page's copies together (1 1 2 2 3 3).
QVector<int> sheetOrder(const QVector<int> &pages, int copies, bool collate);

} // namespace jp::crunch
