#include "core/printjob.h"

#include "core/sheetobjects.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

using jp::grid::GridData;
using jp::grid::GridSheet;
using jp::grid::PagePainter;
using jp::grid::SheetPrint;
using jp::office::sheet::PageSetup;

// ---- the workbook as a source

int BookPrintSource::sheetCount() const { return m_book && m_book->book ? int(m_book->book->info().sheets.size()) : 0; }

QString BookPrintSource::sheetName(int sheet) const { return m_book->book->info().sheets.value(sheet).name; }

bool BookPrintSource::isChartSheet(int sheet) const
{
    return m_book && m_book->book && sheet >= 0 && sheet < sheetCount() && m_book->book->info().sheets[sheet].kind == jp::office::sheet::SheetKind::Chartsheet;
}

bool BookPrintSource::printable(int sheet) const
{
    if (!m_book || !m_book->book || sheet < 0 || sheet >= sheetCount()) return false;
    const jp::office::sheet::SheetEntry &e = m_book->book->info().sheets[sheet];
    if (e.kind != jp::office::sheet::SheetKind::Worksheet && e.kind != jp::office::sheet::SheetKind::Chartsheet) return false;
    return m_book->calc ? true : (m_book->preview && sheet == m_book->previewSheet);
}

bool BookPrintSource::hidden(int sheet) const { return m_book->book->info().sheets.value(sheet).state != jp::office::sheet::SheetState::Visible; }

GridData BookPrintSource::data(int sheet) const
{
    GridData d;
    d.styles = &m_book->book->styles();
    d.strings = &m_book->book->sharedStrings();
    d.dateSystem = m_book->book->info().dateSystem;
    d.sheetIndex = sheet;
    if (m_book->calc) {
        d.sheet = &m_book->calc->sheet(sheet);
        if (m_values) d.values = m_values;
        else d.calc = m_book->calc.get();
        if (size_t(sheet) < m_book->geometry.size()) d.geometry = m_book->geometry[size_t(sheet)];
    } else if (m_book->preview && sheet == m_book->previewSheet) {
        d.sheet = m_book->preview.get();
        d.geometry = m_book->previewGeometry;
    }
    return d;
}

PageSetup BookPrintSource::page(int sheet) const
{
    if (m_pages && sheet < m_pages->sheetCount()) return m_pages->sheet(sheet);
    if (m_book->calc) return m_book->calc->sheet(sheet).page;
    return m_book->preview && sheet == m_book->previewSheet ? m_book->preview->page : PageSetup();
}

std::vector<jp::grid::LayerObject> BookPrintSource::objects(int sheet) { return m_objects ? m_objects->printObjects(sheet) : std::vector<jp::grid::LayerObject>(); }

QString BookPrintSource::baseFontFamily() const { return m_book->book->styles().font(0).name; }

double BookPrintSource::baseFontPoints() const { return m_book->book->styles().font(0).size; }

// ---- the job

namespace {

void applyChoices(PageSetup *s, const PrintJobOptions &o)
{
    if (o.setOrientation) s->orientation = o.orientation;
    if (o.setPaper) {
        s->paperSize = o.paperSize;
        s->paperWidth = s->paperHeight = 0;
    }
    if (o.setMargins) s->margins = o.margins;
    switch (o.scaling) {
    case PrintScaling::AsSheet: break;
    case PrintScaling::NoScaling:
        s->fitToPage = false;
        s->scale = 100;
        break;
    case PrintScaling::FitSheet:
        s->fitToPage = true;
        s->fitToWidth = 1;
        s->fitToHeight = 1;
        break;
    case PrintScaling::FitColumns:
        s->fitToPage = true;
        s->fitToWidth = 1;
        s->fitToHeight = 0;
        break;
    case PrintScaling::FitRows:
        s->fitToPage = true;
        s->fitToWidth = 0;
        s->fitToHeight = 1;
        break;
    }
}

} // namespace

// What a build in slices holds between the slices: the sheets to cut, the one being cut, and the pieces of its cut.
struct PrintJob::Build {
    enum class Stage { Sheet, Rows, Pages, Finish };
    QVector<int> sheets;
    int at = 0;
    int next = 1;
    qint64 room = 0;
    Stage stage = Stage::Sheet;
    SheetInfo info;
    jp::grid::Axis cutRows, cutCols;
    jp::grid::PrintRequest request;     // (its axes are the two above)
    std::unique_ptr<jp::grid::RowAxisTask> rowTask;
    std::unique_ptr<jp::grid::PaginateTask> paginateTask;
};

PrintJob::PrintJob(std::unique_ptr<PrintSource> source, const PrintJobOptions &options) : m_source(std::move(source)), m_options(options) {}

PrintJob::~PrintJob() = default;

bool PrintJob::build(QString *why)
{
    if (!beginBuild(why)) return false;
    jp::grid::Work all;
    return buildStep(all, why) == BuildState::Done;
}

bool PrintJob::beginBuild(QString *why)
{
    m_sheets.clear();
    m_build.reset();
    // (the date and time the fields show are taken once for the whole job: every page of it has the same, and so has a page drawn again)
    m_now = m_options.now.isValid() ? m_options.now : m_options.clock ? m_options.clock() : QDateTime::currentDateTime();
    m_pages.clear();
    m_messages.clear();
    m_contexts.clear();
    m_totalNumbers = 0;
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    auto build = std::make_unique<Build>();
    QVector<int> &sheets = build->sheets;
    if (m_options.what == PrintWhat::Workbook) {
        for (int i = 0; i < m_source->sheetCount(); ++i)
            if (m_source->printable(i) && !m_source->hidden(i)) sheets << i;
    } else if (m_source->printable(m_options.activeSheet)) {
        sheets << m_options.activeSheet;
    } else {
        return refuse(QCoreApplication::translate("jp::crunch::PrintJob", "This sheet has no cells to print."));
    }
    if (sheets.isEmpty()) return refuse(QCoreApplication::translate("jp::crunch::PrintJob", "The workbook has no sheet to print."));
    build->room = std::max(1, m_options.maxPages);
    m_build = std::move(build);
    return true;
}

PrintJob::BuildState PrintJob::buildStep(jp::grid::Work &work, QString *why)
{
    if (!m_build) {
        if (why && why->isEmpty()) *why = QCoreApplication::translate("jp::crunch::PrintJob", "There is nothing to print.");
        return m_pages.empty() ? BuildState::Failed : BuildState::Done;
    }
    Build &b = *m_build;
    bool cutShort = false;
    while (b.at < b.sheets.size() && !cutShort) {
        const int sheet = b.sheets[b.at];
        SheetInfo &info = b.info;
        switch (b.stage) {
        case Build::Stage::Sheet: {
            if (work.spent()) return BuildState::Working;
            info = SheetInfo();
            info.sheet = sheet;
            info.setup = m_source->page(sheet);
            applyChoices(&info.setup, m_options);
            work.add();
            if (m_source->isChartSheet(sheet)) {
                // A chart sheet is one page: the chart over the whole of the paper's room (the page's margins and its orientation say how large), at the page's own proportions.
                info.chart = true;
                const jp::grid::PaperBox paper = jp::grid::paperBox(info.setup);
                const QRectF room = paper.content();
                const int w = int(std::max(1.0, std::floor(room.width() / jp::grid::kPointsPerPixel))), h = int(std::max(1.0, std::floor(room.height() / jp::grid::kPointsPerPixel)));
                auto geometry = std::make_shared<jp::grid::GridGeometry>();
                geometry->layout.rows = jp::grid::Axis(1, h);
                geometry->layout.cols = jp::grid::Axis(1, w);
                info.geometry = geometry;
                info.cols = geometry->layout.cols;
                info.print.paper = paper;
                jp::office::sheet::CellRange block;
                block.valid = true;
                info.print.areas.push_back(block);
                info.print.pages.push_back(jp::grid::PageCut());
                info.print.pages[0].row2 = info.print.pages[0].col2 = 0;
                info.print.scalePercent = 100;
                b.stage = Build::Stage::Finish;
                break;
            }
            GridData data = m_source->data(sheet);
            if (!data.sheet || !data.styles) {
                ++b.at;
                break;
            }
            info.geometry = data.geometry ? data.geometry : std::shared_ptr<const jp::grid::GridGeometry>(GridSheet::makeGeometry(*data.sheet, *data.styles));
            info.cols = jp::grid::printColumnAxis(*data.sheet, *data.styles);   // (what the page is drawn by; the cuts are made by the finer ones below)
            b.cutCols = jp::grid::printColumnAxis(*data.sheet, *data.styles, jp::grid::kCutFine);
            std::vector<jp::grid::ObjectBox> boxes;
            {
                PageObjectPainter objects(m_source->objects(sheet), info.geometry->layout.rows, info.cols);
                if (!info.setup.draft) boxes = objects.boxes();
                work.add(qint64(data.sheet->columns.size()) + qint64(objects.count()));
            }
            jp::grid::PrintRequest &q = b.request;
            q = jp::grid::PrintRequest();
            q.sheet = data.sheet;
            q.styles = data.styles;
            q.rows = &b.cutRows;
            q.cols = &b.cutCols;
            q.rowFine = q.colFine = jp::grid::kCutFine;
            q.setup = info.setup;
            q.objects = boxes;
            q.ignorePrintArea = m_options.ignorePrintArea;
            if (m_options.what == PrintWhat::Selection && sheet == m_options.activeSheet) {
                q.onlySelection = true;
                q.selection = m_options.selection;
            }
            q.maxPages = int(std::max<qint64>(1, std::min<qint64>(b.room, jp::grid::kMaxSheetPages)));
            b.rowTask = std::make_unique<jp::grid::RowAxisTask>(*data.sheet, jp::grid::kCutFine);
            b.stage = Build::Stage::Rows;
            break;
        }
        case Build::Stage::Rows:
            if (!b.rowTask->step(work)) return BuildState::Working;
            b.cutRows = b.rowTask->take();
            b.rowTask.reset();
            b.paginateTask = std::make_unique<jp::grid::PaginateTask>(b.request);
            b.stage = Build::Stage::Pages;
            break;
        case Build::Stage::Pages:
            if (!b.paginateTask->step(work)) return BuildState::Working;
            info.print = b.paginateTask->take();
            b.paginateTask.reset();
            b.stage = Build::Stage::Finish;
            break;
        case Build::Stage::Finish: {
            b.stage = Build::Stage::Sheet;
            ++b.at;
            if (info.print.pages.empty()) {
                m_messages << QCoreApplication::translate("jp::crunch::PrintJob", "%1 has nothing to print.").arg(m_source->sheetName(sheet));
                break;
            }
            if (info.print.truncated)
                m_messages << QCoreApplication::translate("jp::crunch::PrintJob", "%1 needs %2 pages. Only the first %3 are made. Print part of it, or make the pages larger, to get the rest.")
                                  .arg(m_source->sheetName(sheet)).arg(info.print.pagesNeeded).arg(info.print.pages.size());
            if (info.print.tooSmall)
                m_messages << QCoreApplication::translate("jp::crunch::PrintJob", "%1 does not fit the pages asked for even at 10%. It is printed at 10% on more pages.").arg(m_source->sheetName(sheet));
            info.firstNumber = info.setup.useFirstPageNumber ? info.setup.firstPageNumber : b.next;
            info.firstPage = int(m_pages.size());
            for (size_t i = 0; i < info.print.pages.size(); ++i) {
                Page p;
                p.info = int(m_sheets.size());
                p.index = int(i);
                p.number = info.firstNumber + int(i);
                m_pages.push_back(p);
            }
            b.next = info.firstNumber + int(info.print.pages.size());
            b.room -= qint64(info.print.pages.size());
            m_sheets.push_back(std::move(info));
            if (b.room <= 0) {
                if (b.at < b.sheets.size()) m_messages << QCoreApplication::translate("jp::crunch::PrintJob", "The job is cut short at %1 pages.").arg(m_options.maxPages);
                cutShort = true;
            }
            break;
        }
        }
    }
    m_build.reset();
    m_totalNumbers = int(m_pages.size());
    if (m_pages.empty()) {
        if (why) *why = m_messages.isEmpty() ? QCoreApplication::translate("jp::crunch::PrintJob", "There is nothing to print.") : m_messages.join(QLatin1Char(' '));
        return BuildState::Failed;
    }
    return BuildState::Done;
}

QSizeF PrintJob::pageSize(int page) const
{
    if (page < 0 || page >= pageCount()) return QSizeF();
    const SheetPrint &p = m_sheets[size_t(m_pages[size_t(page)].info)].print;
    return QSizeF(p.paper.width, p.paper.height);
}

int PrintJob::sheetOf(int page) const { return page < 0 || page >= pageCount() ? -1 : m_sheets[size_t(m_pages[size_t(page)].info)].sheet; }
int PrintJob::indexInSheet(int page) const { return page < 0 || page >= pageCount() ? -1 : m_pages[size_t(page)].index; }
int PrintJob::numberOf(int page) const { return page < 0 || page >= pageCount() ? 0 : m_pages[size_t(page)].number; }
QString PrintJob::sheetNameOf(int page) const { return page < 0 || page >= pageCount() ? QString() : m_source->sheetName(sheetOf(page)); }

const PrintJob::SheetInfo *PrintJob::infoOfSheet(int sheet) const
{
    for (const SheetInfo &i : m_sheets)
        if (i.sheet == sheet) return &i;
    return nullptr;
}

PrintJob::SheetInfo *PrintJob::infoOfSheet(int sheet)
{
    for (SheetInfo &i : m_sheets)
        if (i.sheet == sheet) return &i;
    return nullptr;
}

const SheetPrint *PrintJob::printOf(int sheet) const
{
    const SheetInfo *i = infoOfSheet(sheet);
    return i ? &i->print : nullptr;
}

const PageSetup *PrintJob::setupOf(int sheet) const
{
    const SheetInfo *i = infoOfSheet(sheet);
    return i ? &i->setup : nullptr;
}

int PrintJob::pagesOfSheet(int sheet) const
{
    const SheetInfo *i = infoOfSheet(sheet);
    return i ? int(i->print.pages.size()) : 0;
}

int PrintJob::firstPageOf(int sheet) const
{
    const SheetInfo *i = infoOfSheet(sheet);
    return i ? i->firstPage : -1;
}

PrintJob::Context &PrintJob::contextFor(int infoIndex)
{
    const SheetInfo &info = m_sheets[size_t(infoIndex)];
    ++m_tick;
    for (Context &c : m_contexts)
        if (c.sheet == info.sheet) {
            c.used = m_tick;
            return c;
        }
    // the grids of the sheets before the last one drawn are let go of
    if (m_contexts.size() >= 2) {
        auto oldest = std::min_element(m_contexts.begin(), m_contexts.end(), [](const Context &a, const Context &b) { return a.used < b.used; });
        m_contexts.erase(oldest);
    }
    m_contexts.emplace_back();
    Context &c = m_contexts.back();
    c.sheet = info.sheet;
    c.used = m_tick;
    if (info.chart) {
        // (a chart sheet: no cells; the chart is made as large as the page's room)
        c.grid = std::make_unique<GridSheet>();
        c.painter = std::make_unique<PagePainter>(c.grid.get());
        c.painter->setBaseFont(m_source->baseFontFamily(), m_source->baseFontPoints());
        c.objects = std::make_unique<PageObjectPainter>(m_source->objects(info.sheet), info.geometry->layout.rows, info.cols);
        c.objects->fitFirstTo(double(info.cols.total()), double(info.geometry->layout.rows.total()));
        return c;
    }
    GridData data = m_source->data(info.sheet);
    data.geometry = info.geometry;
    c.grid = std::make_unique<GridSheet>();
    c.grid->setData(data);
    c.grid->layout().cols = info.cols;
    c.grid->setErrorDisplay(info.setup.errors == jp::office::sheet::PrintErrors::Blank ? GridSheet::ErrorDisplay::Blank
                            : info.setup.errors == jp::office::sheet::PrintErrors::Dash  ? GridSheet::ErrorDisplay::Dash
                            : info.setup.errors == jp::office::sheet::PrintErrors::NA    ? GridSheet::ErrorDisplay::NA
                                                                                          : GridSheet::ErrorDisplay::Shown);
    c.painter = std::make_unique<PagePainter>(c.grid.get());
    c.painter->setBaseFont(m_source->baseFontFamily(), m_source->baseFontPoints());
    if (!info.setup.draft) c.objects = std::make_unique<PageObjectPainter>(m_source->objects(info.sheet), info.geometry->layout.rows, info.cols);
    return c;
}

bool PrintJob::paint(QPainter *painter, int page)
{
    if (page < 0 || page >= pageCount()) return false;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    Context &c = contextFor(pg.info);
    jp::office::sheet::HfContext labels;
    labels.page = pg.number;
    labels.pages = m_totalNumbers;
    labels.now = m_now;
    labels.file = m_options.fileName;
    labels.path = m_options.folder;
    if (!labels.path.isEmpty() && !labels.path.endsWith(QLatin1Char('/'))) labels.path += QLatin1Char('/');
    labels.sheet = m_source->sheetName(info.sheet);
    labels.locale = QLocale();
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    if (info.chart) {
        const jp::grid::PageMetrics m = jp::grid::pageMetrics(info.print, cut, info.setup, info.geometry->layout.rows, info.cols);
        if (c.objects && c.objects->count() > 0) {
            painter->save();
            painter->translate(m.cells.left(), m.cells.top());
            painter->scale(m.unit, m.unit);
            painter->setClipRect(QRect(0, 0, int(m.widthUnits), int(m.heightUnits)));
            c.objects->paint(painter, cut, m.zoom);
            painter->restore();
        }
        c.painter->paintHeaderFooter(painter, info.print.paper, info.setup, labels, true, 1.0);
        return true;
    }
    jp::grid::PageObjects objects;
    if (c.objects && c.objects->count() > 0) objects = [&c, &cut](QPainter *p, const jp::grid::GridLayout &layout, const QRect &) { c.objects->paint(p, cut, layout.zoom()); };
    c.painter->paint(painter, info.print, cut, info.setup, labels, pg.index == 0, objects);
    return true;
}

bool PrintJob::cellRect(int page, qint64 row, qint64 col, QRectF *rect)
{
    if (page < 0 || page >= pageCount()) return false;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    if (info.chart) return false;
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    const bool rowIn = (row >= cut.row1 && row <= cut.row2) || (cut.titleRow1 >= 0 && row >= cut.titleRow1 && row <= cut.titleRow2);
    const bool colIn = (col >= cut.col1 && col <= cut.col2) || (cut.titleCol1 >= 0 && col >= cut.titleCol1 && col <= cut.titleCol2);
    if (!rowIn || !colIn) return false;
    return contextFor(pg.info).painter->cellRect(info.print, cut, info.setup, row, col, rect);
}

bool PrintJob::cellAt(int page, const QPointF &point, qint64 *row, qint64 *col)
{
    if (page < 0 || page >= pageCount()) return false;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    if (info.chart) return false;
    return contextFor(pg.info).painter->cellAt(info.print, info.print.pages[size_t(pg.index)], info.setup, point, row, col);
}

bool PrintJob::rangeRect(int page, qint64 row1, qint64 col1, qint64 row2, qint64 col2, QRectF *rect)
{
    if (page < 0 || page >= pageCount()) return false;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    if (info.chart) return false;
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    // (the part of the range among the page's own rows and columns)
    const qint64 r1 = std::max(row1, cut.row1), r2 = std::min(row2, cut.row2), c1 = std::max(col1, cut.col1), c2 = std::min(col2, cut.col2);
    if (r1 > r2 || c1 > c2) return false;
    QRectF a, b;
    // (the first and last cells the page shows of it; a hidden row or column at an end has none, so the next one that is shown is looked for, a few steps)
    auto shown = [&](qint64 row, qint64 col, int step, QRectF *out) {
        for (int i = 0; i < 64; ++i) {
            if (cellRect(page, std::clamp<qint64>(row + step * i, r1, r2), std::clamp<qint64>(col + step * i, c1, c2), out)) return true;
        }
        return false;
    };
    if (!shown(r1, c1, 1, &a) || !shown(r2, c2, -1, &b)) return false;
    *rect = a.united(b);
    return true;
}

int PrintJob::pageOfCell(int sheet, qint64 row, qint64 col) const
{
    for (size_t i = 0; i < m_pages.size(); ++i) {
        const SheetInfo &info = m_sheets[size_t(m_pages[i].info)];
        if (info.sheet != sheet) continue;
        const jp::grid::PageCut &cut = info.print.pages[size_t(m_pages[i].index)];
        if (row >= cut.row1 && row <= cut.row2 && col >= cut.col1 && col <= cut.col2) return int(i);
    }
    return -1;
}

jp::grid::PaperBox PrintJob::paperOf(int page) const
{
    if (page < 0 || page >= pageCount()) return jp::grid::PaperBox();
    return m_sheets[size_t(m_pages[size_t(page)].info)].print.paper;
}

const jp::office::sheet::PageSetup *PrintJob::setupOfPage(int page) const
{
    if (page < 0 || page >= pageCount()) return nullptr;
    return &m_sheets[size_t(m_pages[size_t(page)].info)].setup;
}

bool PrintJob::isFirstOfSheet(int page) const { return page >= 0 && page < pageCount() && m_pages[size_t(page)].index == 0; }

QImage PrintJob::preview(int page, int widthPixels)
{
    const QSizeF size = pageSize(page);
    if (size.isEmpty() || widthPixels < 1) return QImage();
    // (held to the budget: a page at six times the zoom of a large screen is hundreds of megabytes)
    const double ratio = size.height() / size.width();
    if (double(widthPixels) * widthPixels * ratio > double(kMaxPreviewPixels)) widthPixels = std::max(1, int(std::sqrt(double(kMaxPreviewPixels) / ratio)));
    const double k = double(widthPixels) / size.width();
    QImage img(widthPixels, std::max(1, int(std::lround(size.height() * k))), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.scale(k, k);
    paint(&p, page);
    return img;
}

namespace {

// Sets the sheet of paper to the page's size. A printer is given a standard size where the paper is one, and the way up; a PDF has the size as it is.
void setSheet(QPagedPaintDevice *device, const QSizeF &points)
{
    if (dynamic_cast<QPrinter *>(device)) {
        const bool wide = points.width() > points.height();
        const QSizeF upright = wide ? QSizeF(points.height(), points.width()) : points;
        device->setPageSize(QPageSize(upright, QPageSize::Point, QString(), QPageSize::FuzzyMatch));
        device->setPageOrientation(wide ? QPageLayout::Landscape : QPageLayout::Portrait);
    } else {
        device->setPageSize(QPageSize(points, QPageSize::Point, QString(), QPageSize::ExactMatch));
    }
    device->setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Point);
}

} // namespace

bool PrintJob::exportPdf(const QString &path, const QVector<int> &pages, QString *error, const std::function<bool(int, int)> &progress)
{
    auto fail = [&](const QString &text) {
        if (error) *error = text;
        return false;
    };
    m_cancelled = false;
    if (pages.isEmpty()) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "There are no pages to put in the file."));
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
    bool ok = true;
    {
        QPdfWriter pdf(&file);
        pdf.setCreator(QStringLiteral("JeffCrunch"));
        pdf.setTitle(QFileInfo(m_options.fileName).completeBaseName());
        constexpr int kResolution = 1200;
        pdf.setResolution(kResolution);
        QPainter p;
        bool started = false;
        int done = 0;
        // (no more pages than a job may send, as printing holds it)
        const int total = int(std::min<qint64>(pages.size(), kMaxSentPages));
        for (int at = 0; at < total; ++at) {
            const int page = pages[at];
            if (page < 0 || page >= pageCount()) continue;
            setSheet(&pdf, pageSize(page));
            if (!started) {
                started = p.begin(&pdf);
                if (!started) {
                    ok = false;
                    break;
                }
            } else {
                pdf.newPage();
            }
            p.save();
            p.scale(kResolution / 72.0, kResolution / 72.0);
            paint(&p, page);
            p.restore();
            if (progress && !progress(++done, total)) {
                m_cancelled = true;
                ok = false;
                break;
            }
        }
        if (started) p.end();
        ok = ok && started;
    }
    if (!ok) {
        file.cancelWriting();
        if (m_cancelled) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "It was cancelled, and no file was made."));
        return fail(QCoreApplication::translate("jp::crunch::PrintJob", "The pages could not be put in the file."));
    }
    if (!file.commit()) return fail(file.errorString());
    return true;
}

bool PrintJob::printTo(QPrinter *printer, const QVector<int> &pages, QString *error, const std::function<bool(int, int)> &progress, int copies, bool collate)
{
    auto fail = [&](const QString &text) {
        if (error) *error = text;
        return false;
    };
    m_cancelled = false;
    if (pages.isEmpty()) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "There are no pages to print."));
    // (the pages and their copies, in the order they come out, are worked out one at a time, not listed)
    copies = std::max(1, copies);
    const qint64 n = pages.size();
    const qint64 total = std::min<qint64>(n * copies, kMaxSentPages);
    printer->setFullPage(true);
    QPainter p;
    bool started = false;
    for (qint64 i = 0; i < total; ++i) {
        const int page = pages[int(collate ? i % n : i / copies)];
        if (page < 0 || page >= pageCount()) continue;
        setSheet(printer, pageSize(page));
        if (!started) {
            started = p.begin(printer);
            if (!started) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "The printer could not be used."));
        } else {
            printer->newPage();
        }
        p.save();
        const double k = double(printer->logicalDpiX()) / 72.0;
        p.scale(k, k);
        paint(&p, page);
        p.restore();
        if (progress && !progress(int(i + 1), int(total))) {
            // (the job is called off before the painter ends: ending it is what hands the pages on, and a file the pages were going to is not left half made)
            m_cancelled = true;
            printer->abort();
            p.end();
            if (printer->outputFormat() == QPrinter::PdfFormat && !printer->outputFileName().isEmpty()) QFile::remove(printer->outputFileName());
            return fail(QCoreApplication::translate("jp::crunch::PrintJob", "Printing was stopped."));
        }
    }
    if (started) p.end();
    return started;
}

// ---- ranges

QVector<int> parsePageRange(const QString &text, int pageCount)
{
    QVector<int> out;
    if (pageCount <= 0) return out;
    QSet<int> seen;
    for (const QString &part : text.split(QRegularExpression(QStringLiteral("[,;]")), Qt::SkipEmptyParts)) {
        const QString t = part.trimmed();
        if (t.isEmpty()) continue;
        int first = 0, last = 0;
        const int dash = t.indexOf(QLatin1Char('-'));
        bool ok1 = true, ok2 = true;
        if (dash < 0) {
            first = last = t.toInt(&ok1);
        } else {
            const QString a = t.left(dash).trimmed(), b = t.mid(dash + 1).trimmed();
            first = a.isEmpty() ? 1 : a.toInt(&ok1);
            last = b.isEmpty() ? pageCount : b.toInt(&ok2);
        }
        if (!ok1 || !ok2) continue;
        if (first > last) std::swap(first, last);
        first = std::max(first, 1);
        last = std::min(last, pageCount);
        for (int n = first; n <= last; ++n)
            if (!seen.contains(n - 1)) {
                seen.insert(n - 1);
                out << n - 1;
            }
    }
    return out;
}

QVector<int> sheetOrder(const QVector<int> &pages, int copies, bool collate)
{
    QVector<int> out;
    copies = std::max(1, copies);
    // (held to the most a job may send: the list is never longer)
    const qint64 room = kMaxSentPages;
    if (collate) {
        for (int c = 0; c < copies && qint64(out.size()) + pages.size() <= room; ++c) out += pages;
    } else {
        for (const int p : pages)
            for (int c = 0; c < copies && qint64(out.size()) < room; ++c) out << p;
    }
    return out;
}

} // namespace jp::crunch
