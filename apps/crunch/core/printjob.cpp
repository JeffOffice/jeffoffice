#include "core/printjob.h"

#include "core/sheetobjects.h"

#include <QCoreApplication>
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

bool BookPrintSource::printable(int sheet) const
{
    if (!m_book || !m_book->book || sheet < 0 || sheet >= sheetCount()) return false;
    const jp::office::sheet::SheetEntry &e = m_book->book->info().sheets[sheet];
    if (e.kind != jp::office::sheet::SheetKind::Worksheet || e.state != jp::office::sheet::SheetState::Visible) return false;
    return m_book->calc ? true : (m_book->preview && sheet == m_book->previewSheet);
}

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

PrintJob::PrintJob(std::unique_ptr<PrintSource> source, const PrintJobOptions &options) : m_source(std::move(source)), m_options(options) {}

PrintJob::~PrintJob() = default;

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

bool PrintJob::build(QString *why)
{
    m_sheets.clear();
    m_pages.clear();
    m_messages.clear();
    m_contexts.clear();
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    QVector<int> sheets;
    if (m_options.what == PrintWhat::Workbook) {
        for (int i = 0; i < m_source->sheetCount(); ++i)
            if (m_source->printable(i)) sheets << i;
    } else if (m_source->printable(m_options.activeSheet)) {
        sheets << m_options.activeSheet;
    } else {
        return refuse(QCoreApplication::translate("jp::crunch::PrintJob", "This sheet has no cells to print."));
    }
    if (sheets.isEmpty()) return refuse(QCoreApplication::translate("jp::crunch::PrintJob", "The workbook has no sheet to print."));
    int next = 1;
    qint64 room = std::max(1, m_options.maxPages);
    for (int at = 0; at < sheets.size(); ++at) {
        const int sheet = sheets[at];
        SheetInfo info;
        info.sheet = sheet;
        info.setup = m_source->page(sheet);
        applyChoices(&info.setup, m_options);
        GridData data = m_source->data(sheet);
        if (!data.sheet || !data.styles) continue;
        info.geometry = data.geometry ? data.geometry : std::shared_ptr<const jp::grid::GridGeometry>(GridSheet::makeGeometry(*data.sheet, *data.styles));
        std::vector<jp::grid::ObjectBox> boxes;
        {
            PageObjectPainter objects(m_source->objects(sheet), info.geometry->layout.rows, info.geometry->layout.cols);
            if (!info.setup.draft) boxes = objects.boxes();
        }
        jp::grid::PrintRequest q;
        q.sheet = data.sheet;
        q.styles = data.styles;
        q.rows = &info.geometry->layout.rows;
        q.cols = &info.geometry->layout.cols;
        q.setup = info.setup;
        q.objects = boxes;
        q.ignorePrintArea = m_options.ignorePrintArea;
        if (m_options.what == PrintWhat::Selection && sheet == m_options.activeSheet) {
            q.onlySelection = true;
            q.selection = m_options.selection;
        }
        q.maxPages = int(std::max<qint64>(1, std::min<qint64>(room, jp::grid::kMaxSheetPages)));
        info.print = jp::grid::paginate(q);
        if (info.print.pages.empty()) {
            m_messages << QCoreApplication::translate("jp::crunch::PrintJob", "%1 has nothing to print.").arg(m_source->sheetName(sheet));
            continue;
        }
        if (info.print.truncated)
            m_messages << QCoreApplication::translate("jp::crunch::PrintJob", "%1 needs %2 pages. Only the first %3 are made. Print part of it, or make the pages larger, to get the rest.")
                              .arg(m_source->sheetName(sheet)).arg(info.print.pagesNeeded).arg(info.print.pages.size());
        if (info.print.tooSmall)
            m_messages << QCoreApplication::translate("jp::crunch::PrintJob", "%1 does not fit the pages asked for even at 10%. It is printed at 10% on more pages.").arg(m_source->sheetName(sheet));
        info.firstNumber = info.setup.useFirstPageNumber ? info.setup.firstPageNumber : next;
        info.firstPage = int(m_pages.size());
        for (size_t i = 0; i < info.print.pages.size(); ++i) {
            Page p;
            p.info = int(m_sheets.size());
            p.index = int(i);
            p.number = info.firstNumber + int(i);
            m_pages.push_back(p);
        }
        next = info.firstNumber + int(info.print.pages.size());
        room -= qint64(info.print.pages.size());
        m_sheets.push_back(std::move(info));
        if (room <= 0) {
            if (at + 1 < sheets.size()) m_messages << QCoreApplication::translate("jp::crunch::PrintJob", "The job is cut short at %1 pages.").arg(m_options.maxPages);
            break;
        }
    }
    m_totalNumbers = int(m_pages.size());
    if (m_pages.empty()) return refuse(m_messages.isEmpty() ? QCoreApplication::translate("jp::crunch::PrintJob", "There is nothing to print.") : m_messages.join(QLatin1Char(' ')));
    return true;
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
    GridData data = m_source->data(info.sheet);
    data.geometry = info.geometry;
    c.grid = std::make_unique<GridSheet>();
    c.grid->setData(data);
    c.grid->setErrorDisplay(info.setup.errors == jp::office::sheet::PrintErrors::Blank ? GridSheet::ErrorDisplay::Blank
                            : info.setup.errors == jp::office::sheet::PrintErrors::Dash  ? GridSheet::ErrorDisplay::Dash
                            : info.setup.errors == jp::office::sheet::PrintErrors::NA    ? GridSheet::ErrorDisplay::NA
                                                                                          : GridSheet::ErrorDisplay::Shown);
    c.painter = std::make_unique<PagePainter>(c.grid.get());
    c.painter->setBaseFont(m_source->baseFontFamily(), m_source->baseFontPoints());
    if (!info.setup.draft) c.objects = std::make_unique<PageObjectPainter>(m_source->objects(info.sheet), info.geometry->layout.rows, info.geometry->layout.cols);
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
    labels.now = m_options.now.isValid() ? m_options.now : QDateTime::currentDateTime();
    labels.file = m_options.fileName;
    labels.path = m_options.folder;
    if (!labels.path.isEmpty() && !labels.path.endsWith(QLatin1Char('/'))) labels.path += QLatin1Char('/');
    labels.sheet = m_source->sheetName(info.sheet);
    labels.locale = QLocale();
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    jp::grid::PageObjects objects;
    if (c.objects && c.objects->count() > 0) objects = [&c, &cut](QPainter *p, const jp::grid::GridLayout &layout, const QRect &) { c.objects->paint(p, cut, layout.zoom()); };
    c.painter->paint(painter, info.print, cut, info.setup, labels, pg.index == 0, objects);
    return true;
}

QImage PrintJob::preview(int page, int widthPixels)
{
    const QSizeF size = pageSize(page);
    if (size.isEmpty() || widthPixels < 1) return QImage();
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
        for (const int page : pages) {
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
            if (progress && !progress(++done, int(pages.size()))) {
                ok = false;
                break;
            }
        }
        if (started) p.end();
        ok = ok && started;
    }
    if (!ok) {
        file.cancelWriting();
        return fail(QCoreApplication::translate("jp::crunch::PrintJob", "The pages could not be put in the file."));
    }
    if (!file.commit()) return fail(file.errorString());
    return true;
}

bool PrintJob::printTo(QPrinter *printer, const QVector<int> &pages, QString *error, const std::function<bool(int, int)> &progress)
{
    auto fail = [&](const QString &text) {
        if (error) *error = text;
        return false;
    };
    if (pages.isEmpty()) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "There are no pages to print."));
    printer->setFullPage(true);
    QPainter p;
    bool started = false;
    int done = 0;
    for (const int page : pages) {
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
        if (progress && !progress(++done, int(pages.size()))) {
            p.end();
            printer->abort();
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
    if (collate) {
        for (int c = 0; c < copies; ++c) out += pages;
    } else {
        for (const int p : pages)
            for (int c = 0; c < copies; ++c) out << p;
    }
    return out;
}

} // namespace jp::crunch
