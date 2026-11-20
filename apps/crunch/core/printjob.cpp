#include "core/printjob.h"

#include "core/notebox.h"

#include "grid/hfpicturecache.h"
#include "io/pdftransparency.h"

#include "core/sheetobjects.h"
#include "sheet/cellref.h"
#include "sheet/links.h"
#include "sheet/sheetmodel.h"

#include <QCoreApplication>
#include <QFile>
#include <QLocale>
#include <QFontMetricsF>
#include <QFileInfo>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPen>
#include <QBuffer>
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

std::vector<PrintNote> BookPrintSource::notes(int sheet) const
{
    std::vector<PrintNote> out;
    if (!m_anchors) return out;
    for (const auto &kv : m_anchors->notes(sheet)) {
        PrintNote n;
        n.row = quint32(kv.first >> 32);
        n.col = quint32(kv.first & 0xFFFFFFFFu);
        n.author = kv.second.author;
        n.text = kv.second.text;
        n.shown = kv.second.shown;
        n.box = kv.second.box;
        out.push_back(std::move(n));
    }
    return out;
}

std::vector<PrintLink> BookPrintSource::links(int sheet) const
{
    using namespace jp::office::sheet;
    std::vector<PrintLink> out;
    if (m_anchors)
        for (const LinkSpec &l : m_anchors->links(sheet)) {
            if (out.size() >= size_t(kMaxLinks)) break;
            out.push_back({l.range, l.target, l.location});
        }
    // The cells that are HYPERLINK("address", ...) with the address typed in (one worked out from other cells has no address to give here).
    const Sheet *sh = data(sheet).sheet;
    if (!sh) return out;
    static const QRegularExpression call(QStringLiteral("^\\s*HYPERLINK\\s*\\(\\s*\"((?:[^\"]|\"\")*)\""), QRegularExpression::CaseInsensitiveOption);
    sh->cells.forEach([&](quint32 row, quint32 col, const Cell &cell) {
        if (out.size() >= size_t(kMaxLinks) || !cell.hasFormula()) return;
        const auto m = call.match(sh->formulaText(cell));
        if (!m.hasMatch()) return;
        QString address = m.captured(1);
        address.replace(QStringLiteral("\"\""), QStringLiteral("\""));
        PrintLink k;
        k.range.row1 = k.range.row2 = row;
        k.range.col1 = k.range.col2 = col;
        k.range.valid = true;
        if (address.startsWith(QLatin1Char('#'))) k.location = address.mid(1);
        else k.target = address;
        out.push_back(k);
    });
    return out;
}

QString BookPrintSource::nameText(int sheet, const QString &name) const
{
    QString text;
    if (!m_book || !m_book->calc) return {};
    if (!m_book->calc->nameText(sheet, name, &text) && !m_book->calc->nameText(-1, name, &text)) return {};
    return text.startsWith(QLatin1Char('=')) ? text.mid(1) : text;
}

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
namespace {

// The boxes of the notes that are shown, on the cells of one page: where the file puts each (from its cell's top left), else beside its cell as large as the usual box.
void paintShownNotes(QPainter *p, const jp::grid::GridLayout &layout, const jp::grid::PageCut &cut, const std::vector<PrintNote> &notes, const jp::grid::Axis &rows, const jp::grid::Axis &cols)
{
    const double z = layout.zoom();
    p->save();
    QFont font = p->font();
    font.setPixelSize(std::max(1, int(std::lround(12 * z))));
    p->setFont(font);
    for (const PrintNote &n : notes) {
        if (qint64(n.row) < cut.row1 || qint64(n.row) > cut.row2 || qint64(n.col) < cut.col1 || qint64(n.col) > cut.col2) continue;
        const QRect cell = layout.cellRect(n.row, n.col);
        if (cell.isEmpty()) continue;
        QRect rel(int(cell.width() / std::max(z, 0.001)) + 6, 0, 144, 79);
        if (n.box.set) rel = jp::crunch::noteBoxPixels(n.box, rows, cols, n.row, n.col);
        const QRect box(cell.left() + int(std::lround(rel.x() * z)), cell.top() + int(std::lround(rel.y() * z)), std::max(1, int(std::lround(rel.width() * z))), std::max(1, int(std::lround(rel.height() * z))));
        p->setPen(QPen(QColor(0x80, 0x80, 0x60), std::max(1.0, z)));
        p->setBrush(QColor(0xFF, 0xFF, 0xE1));
        p->drawRect(box);
        p->setPen(QColor(0x20, 0x20, 0x20));
        p->save();
        p->setClipRect(box.adjusted(1, 1, -1, -1), Qt::IntersectClip);
        p->drawText(box.adjusted(int(5 * z), int(4 * z), -int(5 * z), -int(4 * z)), Qt::TextWordWrap, n.text.left(4000));
        p->restore();
    }
    p->restore();
}

// The pages of notes of a sheet: every note as "Cell A1: author: words" (its lines kept), a blank line between them, broken into lines as wide as the page's room in the font the sheet's text is
// in, and into pages by the room it has down. (The text is laid out in eighths of a point, as the headers are.)
std::vector<QStringList> notePagesOf(const std::vector<PrintNote> &notes, const jp::grid::PaperBox &paper, const QString &family, double points, qint64 *room)
{
    const double U = 8;
    QFont font(family);
    font.setPixelSize(std::max(1, int(std::lround(std::min(points > 0 ? points : 11.0, 409.0) * U))));
    const QFontMetricsF fm(font);
    const double width = std::max(1.0, (paper.width - paper.left - paper.right) * U), height = std::max(1.0, (paper.height - paper.top - paper.bottom) * U);
    const int perPage = std::max(1, int(std::floor(height / std::max(1.0, fm.lineSpacing()))));
    QStringList lines;
    for (const PrintNote &n : notes) {
        if (!lines.isEmpty()) lines << QString();
        QString text = QCoreApplication::translate("jp::crunch::PrintJob", "Cell %1: ").arg(jp::office::sheet::cellName(n.row, n.col));
        if (!n.author.isEmpty()) text += n.author + QLatin1String(": ");
        text += n.text;
        for (const QString &paragraph : text.split(QLatin1Char('\n'))) {
            QString line;
            for (const QString &word : paragraph.split(QLatin1Char(' '))) {
                const QString candidate = line.isEmpty() ? word : line + QLatin1Char(' ') + word;
                if (line.isEmpty() || fm.horizontalAdvance(candidate) <= width) {
                    line = candidate;
                } else {
                    lines << line;
                    line = word;
                }
            }
            lines << line;
            if (qint64(lines.size()) > *room * perPage) break;   // (more than the job has room for: the rest is not laid out)
        }
        if (qint64(lines.size()) > *room * perPage) break;
    }
    std::vector<QStringList> pages;
    for (qsizetype at = 0; at < lines.size() && qint64(pages.size()) < *room; at += perPage) pages.push_back(lines.mid(at, perPage));
    return pages;
}

} // namespace

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
    } else if (m_options.sheets.size() > 1) {
        for (const int i : m_options.sheets)
            if (i >= 0 && i < m_source->sheetCount() && m_source->printable(i) && !m_source->hidden(i) && !sheets.contains(i)) sheets << i;
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
                info.rows = geometry->layout.rows;
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
            info.rows = jp::grid::printRowAxis(*data.sheet, 1, true);
            b.cutCols = jp::grid::printColumnAxis(*data.sheet, *data.styles, jp::grid::kCutFine);
            std::vector<jp::grid::ObjectBox> boxes;
            {
                PageObjectPainter objects(m_source->objects(sheet), info.rows, info.cols);
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
            b.rowTask = std::make_unique<jp::grid::RowAxisTask>(*data.sheet, jp::grid::kCutFine, true);
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
            // the notes of a sheet whose comments print at the end have pages after the sheet's own
            if (info.setup.comments == jp::office::sheet::PrintComments::AtEnd && !info.chart && b.room > 0) {
                const std::vector<PrintNote> notes = m_source->notes(sheet);
                if (!notes.empty()) {
                    qint64 room = b.room;
                    info.notePages = notePagesOf(notes, info.print.paper, m_source->baseFontFamily(), m_source->baseFontPoints(), &room);
                    for (size_t i = 0; i < info.notePages.size(); ++i) {
                        Page p;
                        p.info = int(m_sheets.size());
                        p.index = int(info.print.pages.size() + i);
                        p.number = b.next + int(i);
                        p.note = int(i);
                        m_pages.push_back(p);
                    }
                    b.next += int(info.notePages.size());
                    b.room -= qint64(info.notePages.size());
                }
            }
            // the notes that are shown print with the cells, when the sheet's comments are to print as displayed
            if (info.setup.comments == jp::office::sheet::PrintComments::AsDisplayed && !info.chart)
                for (PrintNote &n : m_source->notes(sheet))
                    if (n.shown) info.shownNotes.push_back(std::move(n));
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
int PrintJob::numberOf(int page) const { return page < 0 || page >= pageCount() ? 0 : m_pages[size_t(page)].number; }
QString PrintJob::sheetNameOf(int page) const { return page < 0 || page >= pageCount() ? QString() : m_source->sheetName(sheetOf(page)); }

const PrintJob::SheetInfo *PrintJob::infoOfSheet(int sheet) const
{
    for (const SheetInfo &i : m_sheets)
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

QStringList PrintJob::noteLines(int page) const
{
    if (page < 0 || page >= pageCount() || m_pages[size_t(page)].note < 0) return QStringList();
    return m_sheets[size_t(m_pages[size_t(page)].info)].notePages[size_t(m_pages[size_t(page)].note)];
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
    fillContext(c, info, false);
    return c;
}

void PrintJob::fillContext(Context &c, const SheetInfo &info, bool forBackdrop)
{
    c.sheet = info.sheet;
    if (info.chart) {
        // (a chart sheet: no cells; the chart is made as large as the page's room)
        c.grid = std::make_unique<GridSheet>();
        c.painter = std::make_unique<PagePainter>(c.grid.get());
        c.painter->setBaseFont(m_source->baseFontFamily(), m_source->baseFontPoints());
        c.painter->setSolidColors(m_options.archival);
        if (!forBackdrop) {
            c.objects = std::make_unique<PageObjectPainter>(m_source->objects(info.sheet), info.rows, info.cols);
            c.objects->fitFirstTo(double(info.cols.total()), double(info.rows.total()));
            if (m_tagger) c.painter->setTagger(m_tagger);
        }
        return;
    }
    GridData data = m_source->data(info.sheet);
    data.geometry = info.geometry;
    c.grid = std::make_unique<GridSheet>();
    c.grid->setData(data);
    c.grid->layout().cols = info.cols;
    c.grid->layout().rows = info.rows;
    c.grid->setErrorDisplay(info.setup.errors == jp::office::sheet::PrintErrors::Blank ? GridSheet::ErrorDisplay::Blank
                            : info.setup.errors == jp::office::sheet::PrintErrors::Dash  ? GridSheet::ErrorDisplay::Dash
                            : info.setup.errors == jp::office::sheet::PrintErrors::NA    ? GridSheet::ErrorDisplay::NA
                                                                                          : GridSheet::ErrorDisplay::Shown);
    c.painter = std::make_unique<PagePainter>(c.grid.get());
    c.painter->setBaseFont(m_source->baseFontFamily(), m_source->baseFontPoints());
    c.painter->setSolidColors(m_options.archival);   // (PDF/A: a see-through fill or bar is laid on what is beneath it)
    if (forBackdrop) return;
    c.painter->setAsyncPictures(m_asyncPictures);
    c.painter->setScreenMarks(m_screenGrid, m_screenHeadings);
    if (!info.setup.draft) c.objects = std::make_unique<PageObjectPainter>(m_source->objects(info.sheet), info.rows, info.cols);
    if (m_tagger) c.painter->setTagger(m_tagger);
}

namespace {
double kMaxGrayPixels = 16.0 * 1000 * 1000;   // (a picture of 64 MB)
}

void PrintJob::setGrayPixelLimitForTest(double pixels) { kMaxGrayPixels = pixels; }

bool PrintJob::paint(QPainter *painter, int page)
{
    if (!m_options.grayscale || m_inGray || m_grayInPdf) return paintColors(painter, page);
    if (page < 0 || page >= pageCount()) return false;
    // Gray: the page is painted in color into pixels (as many to the point as the device has, up to 300 to the inch), and drawn from there as gray ones.
    const QSizeF paper = pageSize(page);
    const QTransform toDevice = painter->deviceTransform();
    double k = std::clamp(std::hypot(toDevice.m11(), toDevice.m12()), 0.05, 300.0 / 72);
    // (no more than kMaxGrayPixels of pixels: a very large page is made gray from fewer)
    if (kMaxGrayPixels > 0 && paper.width() * k * paper.height() * k > kMaxGrayPixels) k *= std::sqrt(kMaxGrayPixels / (paper.width() * k * paper.height() * k));
    QImage img = kMaxGrayPixels < 0 ? QImage() : QImage((paper * k).toSize().expandedTo(QSize(1, 1)), QImage::Format_RGB32);   // (below nothing: a machine with no room, for a test)
    if (img.isNull()) {
        // (the pixels could not be had: the page is in color, and the person is told, never a blank page)
        const QString said = QCoreApplication::translate("jp::crunch::PrintJob", "There was not room to make the pages gray, so some are in color.");
        if (!m_messages.contains(said)) m_messages << said;
        return paintColors(painter, page);
    }
    img.fill(Qt::white);
    bool ok = false;
    {
        QPainter ip(&img);
        ip.setRenderHint(QPainter::Antialiasing);
        ip.setRenderHint(QPainter::TextAntialiasing);
        ip.setRenderHint(QPainter::SmoothPixmapTransform);
        ip.scale(img.width() / paper.width(), img.height() / paper.height());
        m_inGray = true;
        ok = paintColors(&ip, page);
        m_inGray = false;
    }
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawImage(QRectF(QPointF(0, 0), paper), img.convertToFormat(QImage::Format_Grayscale8));
    painter->restore();
    return ok;
}

jp::office::sheet::HfContext PrintJob::labelsOf(const Page &pg, const SheetInfo &info) const
{
    jp::office::sheet::HfContext labels;
    labels.page = pg.number;
    labels.pages = m_totalNumbers;
    labels.now = m_now;
    labels.file = m_options.fileName;
    labels.path = m_options.folder;
    if (!labels.path.isEmpty() && !labels.path.endsWith(QLatin1Char('/'))) labels.path += QLatin1Char('/');
    labels.sheet = m_source->sheetName(info.sheet);
    labels.locale = QLocale();
    return labels;
}

// PDF/A: what lies beneath the pictures of a page, painted apart from the page that is being painted (its own painters: the grid and the painter of a paint in progress are not
// asked to paint again; they are kept for the pages of the sheet that follow). `beneath` has the page's count of pictures asked for, and the page without its objects once there are many.
namespace {
struct Beneath {
    int asked = 0;       // pictures that were given the pixels beneath them one at a time
    QImage page;         // the page's cells painted once (when many pictures ask)
    double scale = 0;
};
} // namespace

void PrintJob::paintBeneath(QPainter *painter, int page, PageObjectPainter *objects, int stopAt)
{
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    if (!m_backdropContext.painter || m_backdropContext.sheet != info.sheet) fillContext(m_backdropContext, info, true);   // (kept for the pages of the sheet that follow)
    ++m_backdropPaints;
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    if (info.chart) {
        const jp::grid::PageMetrics m = jp::grid::pageMetrics(info.print, cut, info.setup, info.rows, info.cols);
        if (objects && objects->count() > 0) {
            painter->save();
            painter->translate(m.cells.left(), m.cells.top());
            painter->scale(m.unit, m.unit);
            painter->setClipRect(QRect(0, 0, int(m.widthUnits), int(m.heightUnits)));
            objects->paint(painter, cut, m.zoom, PageObjectPainter::Backdrop(), stopAt);
            painter->restore();
        }
        return;
    }
    jp::grid::PageObjects drawn;
    if (objects && objects->count() > 0)
        drawn = [objects, &cut, stopAt](QPainter *p, const jp::grid::GridLayout &layout, const QRect &) { objects->paint(p, cut, layout.zoom(), PageObjectPainter::Backdrop(), stopAt); };
    m_backdropContext.painter->paint(painter, info.print, cut, info.setup, labelsOf(pg, info), pg.index == 0, drawn);
}

bool PrintJob::paintColors(QPainter *painter, int page)
{
    if (page < 0 || page >= pageCount()) return false;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    Context &c = contextFor(pg.info);
    const jp::office::sheet::HfContext labels = labelsOf(pg, info);
    if (pg.note >= 0) {
        // a page of notes: their lines in the page's room, and the header and footer
        const double U = 8;
        const jp::grid::PaperBox &paper = info.print.paper;
        QFont font(m_source->baseFontFamily());
        const double points = m_source->baseFontPoints();
        font.setPixelSize(std::max(1, int(std::lround(std::min(points > 0 ? points : 11.0, 409.0) * U))));
        const QFontMetricsF fm(font);
        painter->save();
        painter->scale(1.0 / U, 1.0 / U);
        painter->setFont(font);
        painter->setPen(Qt::black);
        double y = paper.top * U + fm.ascent();
        // (the notes of a page are one paragraph for a screen reader)
        const int paragraph = m_tagger ? m_tagger->add({QStringLiteral("P")}) : -1;
        if (m_tagger) m_tagger->begin(painter, paragraph);
        for (const QString &line : info.notePages[size_t(pg.note)]) {
            painter->drawText(QPointF(paper.left * U, y), line);
            y += fm.lineSpacing();
        }
        if (m_tagger) m_tagger->end(painter, paragraph);
        painter->restore();
        c.painter->paintHeaderFooter(painter, paper, info.setup, labels, false, 1.0);
        return true;
    }
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    // PDF/A: what lies beneath a see-through picture is painted into pixels, so that the picture's clear parts are not left clear. The page beneath is painted by painters of its own, for each of
    // the first few pictures (with the objects that lie under that one); past kMostBackdropsPerPage the page is painted once, with the cells and every object (one above a picture is drawn over it again afterwards, so it looks right), and each picture is given its part of that.
    const QTransform base = painter->transform();
    PageObjectPainter::Backdrop under;
    if (m_options.archival && c.objects && c.objects->count() > 0) {
        const auto beneath = std::make_shared<Beneath>();
        PageObjectPainter *const objectsOf = c.objects.get();
        under = [this, base, page, beneath, objectsOf](const QRectF &where, const QSize &pixels, int index) {
            const QRectF points = base.inverted().mapRect(where);
            if (pixels.isEmpty() || points.width() <= 0 || points.height() <= 0) return QImage();
            if (beneath->asked < kMostBackdropsPerPage) {
                ++beneath->asked;
                QImage img(pixels, QImage::Format_RGB32);
                img.fill(Qt::white);
                QPainter ip(&img);
                ip.setRenderHint(QPainter::Antialiasing);
                ip.setRenderHint(QPainter::TextAntialiasing);
                ip.setRenderHint(QPainter::SmoothPixmapTransform);
                ip.scale(img.width() / points.width(), img.height() / points.height());
                ip.translate(-points.topLeft());
                ip.setClipRect(points);
                paintBeneath(&ip, page, objectsOf, index);
                ip.end();
                return img;
            }
            if (beneath->page.isNull()) {
                const QSizeF paper = pageSize(page);
                constexpr double kMostPixels = 16.0 * 1000 * 1000;
                beneath->scale = std::min(300.0 / 72.0, std::sqrt(kMostPixels / std::max(1.0, paper.width() * paper.height())));
                beneath->page = QImage(QSize(std::max(1, int(std::ceil(paper.width() * beneath->scale))), std::max(1, int(std::ceil(paper.height() * beneath->scale)))), QImage::Format_RGB32);
                beneath->page.fill(Qt::white);
                QPainter ip(&beneath->page);
                ip.setRenderHint(QPainter::Antialiasing);
                ip.setRenderHint(QPainter::TextAntialiasing);
                ip.scale(beneath->scale, beneath->scale);
                paintBeneath(&ip, page, objectsOf, -1);
            }
            const QRect part = QRectF(points.x() * beneath->scale, points.y() * beneath->scale, points.width() * beneath->scale, points.height() * beneath->scale).toAlignedRect().intersected(beneath->page.rect());
            QImage img(pixels, QImage::Format_RGB32);
            img.fill(Qt::white);
            if (part.isEmpty()) return img;
            QPainter ip(&img);
            ip.setRenderHint(QPainter::SmoothPixmapTransform);
            ip.drawImage(QRectF(0, 0, pixels.width(), pixels.height()), beneath->page, QRectF(part));
            return img;
        };
    }
    if (info.chart) {
        const jp::grid::PageMetrics m = jp::grid::pageMetrics(info.print, cut, info.setup, info.rows, info.cols);
        if (c.objects && c.objects->count() > 0) {
            painter->save();
            painter->translate(m.cells.left(), m.cells.top());
            painter->scale(m.unit, m.unit);
            painter->setClipRect(QRect(0, 0, int(m.widthUnits), int(m.heightUnits)));
            c.painter->markDecoration(painter, true);   // (a chart has no tag: it is left out of the reading order)
            c.objects->paint(painter, cut, m.zoom, under);
            c.painter->markDecoration(painter, false);
            painter->restore();
        }
        c.painter->paintHeaderFooter(painter, info.print.paper, info.setup, labels, true, 1.0);
        return true;
    }
    jp::grid::PageObjects objects;
    if (c.objects && c.objects->count() > 0) objects = [&c, &cut, under](QPainter *p, const jp::grid::GridLayout &layout, const QRect &) { c.objects->paint(p, cut, layout.zoom(), under); };
    if (!info.shownNotes.empty()) {
        // (the boxes of the notes that are shown, over the cells and the objects)
        objects = [&info, &cut, objects](QPainter *p, const jp::grid::GridLayout &layout, const QRect &cells) {
            if (objects) objects(p, layout, cells);
            paintShownNotes(p, layout, cut, info.shownNotes, info.rows, info.cols);
        };
    }
    c.painter->paint(painter, info.print, cut, info.setup, labels, pg.index == 0, objects);
    m_lastStats = c.painter->stats();
    m_lastMissed = c.painter->missedPictures();
    return true;
}

void PrintJob::setAsyncPictures(bool on)
{
    m_asyncPictures = on;
    for (Context &c : m_contexts)
        if (c.painter) c.painter->setAsyncPictures(on);
}

void PrintJob::setScreenMarks(bool gridlines, bool headings)
{
    m_screenGrid = gridlines;
    m_screenHeadings = headings;
    for (Context &c : m_contexts)
        if (c.painter) c.painter->setScreenMarks(gridlines, headings);
}

bool PrintJob::picturesPending() const { return jp::grid::HeaderPictureCache::instance().pendingCount() > 0; }

void PrintJob::prefetchPictures()
{
    for (const SheetInfo &info : m_sheets)
        for (const jp::office::sheet::HeaderPicture &p : info.setup.headerFooter.pictures) {
            bool pending = false;
            jp::grid::HeaderPictureCache::instance().get(p.image, false, &pending);
        }
}

qint64 PrintJob::layoutsWorkedOut() const
{
    qint64 n = 0;
    for (const Context &c : m_contexts)
        if (c.painter) n += c.painter->layoutsWorkedOut();
    return n;
}

bool PrintJob::cellRect(int page, qint64 row, qint64 col, QRectF *rect)
{
    if (page < 0 || page >= pageCount()) return false;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    if (info.chart || pg.note >= 0) return false;
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
    if (info.chart || pg.note >= 0) return false;
    return contextFor(pg.info).painter->cellAt(info.print, info.print.pages[size_t(pg.index)], info.setup, point, row, col);
}

bool PrintJob::rangeRect(int page, qint64 row1, qint64 col1, qint64 row2, qint64 col2, QRectF *rect)
{
    if (page < 0 || page >= pageCount()) return false;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    if (info.chart || pg.note >= 0) return false;
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    // (the part of the range among the page's own rows and columns)
    return shownRect(page, std::max(row1, cut.row1), std::max(col1, cut.col1), std::min(row2, cut.row2), std::min(col2, cut.col2), rect);
}

bool PrintJob::shownRect(int page, qint64 r1, qint64 c1, qint64 r2, qint64 c2, QRectF *rect)
{
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

std::vector<QRectF> PrintJob::linkRects(int page, qint64 row1, qint64 col1, qint64 row2, qint64 col2)
{
    std::vector<QRectF> out;
    if (page < 0 || page >= pageCount()) return out;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    if (info.chart || pg.note >= 0) return out;
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    // (the parts of the range among the page's own lines, and among the titles repeated above and to the left of them: each part a box)
    using Span = std::pair<qint64, qint64>;
    auto spans = [](qint64 a, qint64 b, qint64 first, qint64 last, qint64 titleFirst, qint64 titleLast) {
        std::vector<Span> v;
        if (titleFirst >= 0 && (titleLast < first || titleFirst > last) && std::max(a, titleFirst) <= std::min(b, titleLast)) v.push_back({std::max(a, titleFirst), std::min(b, titleLast)});
        if (std::max(a, first) <= std::min(b, last)) v.push_back({std::max(a, first), std::min(b, last)});
        return v;
    };
    for (const Span &r : spans(row1, row2, cut.row1, cut.row2, cut.titleRow1, cut.titleRow2))
        for (const Span &c : spans(col1, col2, cut.col1, cut.col2, cut.titleCol1, cut.titleCol2)) {
            QRectF box;
            if (shownRect(page, r.first, c.first, r.second, c.second, &box)) out.push_back(box);
        }
    return out;
}

bool PrintJob::guidesOf(int page, PreviewGuides *out)
{
    if (page < 0 || page >= pageCount()) return false;
    const Page &pg = m_pages[size_t(page)];
    const SheetInfo &info = m_sheets[size_t(pg.info)];
    if (info.chart || pg.note >= 0) return false;
    const jp::grid::PageCut &cut = info.print.pages[size_t(pg.index)];
    PreviewGuides g;
    g.sheet = info.sheet;
    g.scalePercent = info.print.scalePercent;
    // (the cells' own area: the body and the titles repeated beside and above it)
    const qint64 row1 = cut.titleRow1 >= 0 && cut.titleRow2 < cut.row1 ? cut.titleRow1 : cut.row1, col1 = cut.titleCol1 >= 0 && cut.titleCol2 < cut.col1 ? cut.titleCol1 : cut.col1;
    for (const QRectF &r : linkRects(page, row1, col1, cut.row2, cut.col2)) g.cells = g.cells.united(r);
    // (a column's own width is the narrowest the cells of its first rows have on the page: a merged cell is wider than the column)
    auto add = [&](qint64 first, qint64 last) {
        for (qint64 col = first; col <= last && g.columns.size() < kMostColumnEdges; ++col) {
            QRectF best;
            for (qint64 row = row1; row <= cut.row2 && row < row1 + 24; ++row) {
                QRectF r;
                if (cellRect(page, row, col, &r) && (best.isNull() || r.width() < best.width())) best = r;
            }
            if (best.isNull()) continue;
            g.columns.push_back({col, best.left(), best.right()});
        }
    };
    if (cut.titleCol1 >= 0 && cut.titleCol2 < cut.col1) add(cut.titleCol1, cut.titleCol2);
    add(cut.col1, cut.col2);
    *out = g;
    return !g.cells.isNull();
}

double PrintJob::fileWidthForPoints(int sheet, int scalePercent, double pointsOnPage) const
{
    // (the points on the page are the sheet's own points made smaller or larger by the scale; the printer's pixels are what a column's width is counted in)
    const jp::grid::GridData data = m_source->data(sheet);
    if (!data.styles || scalePercent <= 0 || !(pointsOnPage > 0)) return 0;
    const double own = pointsOnPage * 100.0 / scalePercent;
    const int digit = jp::grid::printDigitPixels(*data.styles);
    return jp::grid::GridSheet::widthForPixels(std::max(1, int(std::lround(own * jp::grid::kPrinterDpi / 72.0))), digit);
}

double PrintJob::pointsOnPageFor(int sheet, int scalePercent, double fileWidth) const
{
    const jp::grid::GridData data = m_source->data(sheet);
    if (!data.styles) return 0;
    return jp::grid::printColumnPoints(fileWidth, jp::grid::printDigitPixels(*data.styles)) * scalePercent / 100.0;
}

int PrintJob::pageOfCell(int sheet, qint64 row, qint64 col) const
{
    for (size_t i = 0; i < m_pages.size(); ++i) {
        const SheetInfo &info = m_sheets[size_t(m_pages[i].info)];
        if (info.sheet != sheet || m_pages[i].note >= 0) continue;
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

QImage PrintJob::previewSheet(const QVector<int> &group, int perSheet, int widthPixels)
{
    perSheet = perSheet == 2 || perSheet == 4 || perSheet == 6 || perSheet == 9 || perSheet == 16 ? perSheet : 1;
    if (group.isEmpty() || group.first() < 0 || group.first() >= pageCount() || widthPixels < 1) return QImage();
    const QSizeF size = sheetFor(group, perSheet);
    if (size.isEmpty()) return QImage();
    const double ratio = size.height() / size.width();
    if (double(widthPixels) * widthPixels * ratio > double(kMaxPreviewPixels)) widthPixels = std::max(1, int(std::sqrt(double(kMaxPreviewPixels) / ratio)));
    const double k = double(widthPixels) / size.width();
    QImage img(widthPixels, std::max(1, int(std::lround(size.height() * k))), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.scale(k, k);
    paintGroup(&p, group, size, perSheet);
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

namespace {

int cleanPerSheet(int n) { return n == 2 || n == 4 || n == 6 || n == 9 || n == 16 ? n : 1; }

// The pages cut into the groups that share a sheet.
QVector<QVector<int>> groupsOf(const QVector<int> &pages, int perSheet)
{
    QVector<QVector<int>> groups;
    for (int i = 0; i < pages.size(); i += perSheet) groups.push_back(pages.mid(i, perSheet));
    return groups;
}

void gridOf(int perSheet, int *cols, int *rows)
{
    *cols = perSheet == 2 ? 2 : perSheet == 4 ? 2 : perSheet == 6 ? 3 : perSheet == 9 ? 3 : 4;
    *rows = perSheet / *cols;
}

} // namespace

QSizeF PrintJob::sheetFor(const QVector<int> &group, int perSheet) const
{
    QSizeF size = pageSize(group.first());
    if (m_options.scaleToPaper > 0) {
        double w = 0, h = 0;
        jp::office::sheet::paperSizePoints(m_options.scaleToPaper, &w, &h);
        QSizeF paper(w, h);
        if ((size.width() > size.height()) != (paper.width() > paper.height())) paper.transpose();
        size = paper;
    }
    if ((perSheet == 2 || perSheet == 6) && size.height() > size.width()) size.transpose();
    return size;
}

void PrintJob::paintGroup(QPainter *painter, const QVector<int> &group, const QSizeF &sheet, int perSheet)
{
    if (perSheet == 1) {
        const QSizeF size = pageSize(group.first());
        if (m_options.scaleToPaper > 0 && size.width() > 0 && size.height() > 0 && (size != sheet)) {
            // (the page is shrunk or grown to the paper, keeping its proportions, and put in the middle)
            const double k = std::min(sheet.width() / size.width(), sheet.height() / size.height());
            painter->save();
            painter->translate((sheet.width() - size.width() * k) / 2, (sheet.height() - size.height() * k) / 2);
            painter->scale(k, k);
            painter->setClipRect(QRectF(QPointF(0, 0), size));
            paint(painter, group.first());
            painter->restore();
            return;
        }
        paint(painter, group.first());
        return;
    }
    int cols = 1, rows = 1;
    gridOf(perSheet, &cols, &rows);
    const double cw = sheet.width() / cols, ch = sheet.height() / rows;
    for (int i = 0; i < group.size(); ++i) {
        const int page = group[i];
        if (page < 0 || page >= pageCount()) continue;
        const QSizeF size = pageSize(page);
        if (size.width() <= 0 || size.height() <= 0) continue;
        const double k = std::min(cw / size.width(), ch / size.height()) * 0.96;
        const double x0 = (i % cols) * cw + (cw - size.width() * k) / 2, y0 = (i / cols) * ch + (ch - size.height() * k) / 2;
        painter->save();
        painter->translate(x0, y0);
        painter->scale(k, k);
        painter->setClipRect(QRectF(QPointF(0, 0), size));
        paint(painter, page);
        painter->restore();
        painter->save();
        painter->setPen(QPen(QColor(0x80, 0x80, 0x80), 0.5));
        if (m_tagger) m_tagger->begin(painter, m_artifactTag);
        painter->drawRect(QRectF(x0, y0, size.width() * k, size.height() * k));
        if (m_tagger) m_tagger->end(painter, m_artifactTag);
        painter->restore();
    }
}

void PrintJob::placeOnSheet(const QVector<int> &group, const QSizeF &sheet, int perSheet, QVector<PdfPlace> *places) const
{
    // (where paintGroup puts each page of the group: the corner of its box on the sheet and its scale)
    if (perSheet == 1) {
        const QSizeF size = pageSize(group.first());
        PdfPlace at{0, 0, 1, sheet.height()};
        if (m_options.scaleToPaper > 0 && size.width() > 0 && size.height() > 0 && size != sheet) {
            at.k = std::min(sheet.width() / size.width(), sheet.height() / size.height());
            at.x = (sheet.width() - size.width() * at.k) / 2;
            at.y = (sheet.height() - size.height() * at.k) / 2;
        }
        (*places)[group.first()] = at;
        return;
    }
    int cols = 1, rows = 1;
    gridOf(perSheet, &cols, &rows);
    const double cw = sheet.width() / cols, ch = sheet.height() / rows;
    for (int i = 0; i < group.size(); ++i) {
        const int page = group[i];
        if (page < 0 || page >= pageCount()) continue;
        const QSizeF size = pageSize(page);
        if (size.width() <= 0 || size.height() <= 0) continue;
        const double k = std::min(cw / size.width(), ch / size.height()) * 0.96;
        (*places)[page] = PdfPlace{(i % cols) * cw + (cw - size.width() * k) / 2, (i / cols) * ch + (ch - size.height() * k) / 2, k, sheet.height()};
    }
}

QVector<jp::PdfLink> PrintJob::pdfLinks(const QVector<int> &pdfPageOf, const QVector<PdfPlace> &places)
{
    using namespace jp::office::sheet;
    QVector<jp::PdfLink> out;
    std::map<int, std::vector<PrintLink>> bySheet;
    auto linksOf = [&](int sheet) -> const std::vector<PrintLink> & {
        auto found = bySheet.find(sheet);
        if (found == bySheet.end()) found = bySheet.emplace(sheet, m_source->links(sheet)).first;
        return found->second;
    };
    QHash<QString, int> sheetByName;
    for (int i = 0; i < m_source->sheetCount(); ++i) sheetByName.insert(m_source->sheetName(i).toLower(), i);
    for (int page = 0; page < pageCount() && page < pdfPageOf.size(); ++page) {
        if (pdfPageOf[page] < 0 || page >= places.size()) continue;
        const Page &pg = m_pages[size_t(page)];
        if (pg.note >= 0) continue;
        const int sheet = m_sheets[size_t(pg.info)].sheet;
        for (const PrintLink &l : linksOf(sheet)) {
            if (!l.range.valid) continue;
            // (the boxes first: most links are not on this page, and a link to a place in the workbook costs more to work out)
            const std::vector<QRectF> boxes = linkRects(page, l.range.row1, l.range.col1, l.range.row2, l.range.col2);
            if (boxes.empty()) continue;
            int toPage = -1;
            QString uri;
            bool hasSpot = false;
            QPointF spot;
            if (!l.target.isEmpty()) {
                if (!jp::clickableAddress(l.target)) continue;
                uri = l.target;
            } else {
                // a place in the workbook: "Sheet2!B5" (a defined name has no place to go to here)
                QString place = l.location.trimmed();
                if (place.startsWith(QLatin1Char('#'))) place.remove(0, 1);
                if (place.lastIndexOf(QLatin1Char('!')) < 0) place = m_source->nameText(sheet, place);   // (a defined name: the cells it stands for)
                const qsizetype bang = place.lastIndexOf(QLatin1Char('!'));
                if (bang < 0) continue;
                QString name = place.left(bang);
                if (name.size() >= 2 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
                const auto at = sheetByName.constFind(name.toLower());
                jp::office::sheet::CellRange to;
                if (at == sheetByName.cend() || !parseRangeRef(QStringView(place).mid(bang + 1), &to)) continue;
                const int target = pageOfCell(at.value(), to.row1, to.col1);
                QRectF cell;
                if (target >= 0 && target < pdfPageOf.size() && pdfPageOf[target] >= 0) {
                    toPage = pdfPageOf[target];
                    if (cellRect(target, to.row1, to.col1, &cell) && target < places.size()) {
                        hasSpot = true;
                        const PdfPlace &at = places[target];
                        spot = QPointF(at.x + cell.left() * at.k, at.sheetHeight - (at.y + cell.top() * at.k));
                    }
                } else {
                    // (the cell's page is not in the PDF: the sheet's first page that is, where there is one)
                    toPage = firstPdfPageOf(at.value(), pdfPageOf);
                    if (toPage < 0) continue;
                }
            }
            for (const QRectF &r : boxes) {
                jp::PdfLink k;
                k.page = pdfPageOf[page];
                const PdfPlace &at = places[page];
                k.rect = QRectF(QPointF(at.x + r.left() * at.k, at.sheetHeight - (at.y + r.bottom() * at.k)), QPointF(at.x + r.right() * at.k, at.sheetHeight - (at.y + r.top() * at.k)));
                k.uri = uri;
                k.toPage = toPage;
                k.hasSpot = hasSpot;
                k.spot = spot;
                out.push_back(k);
                if (out.size() >= jp::kMostPdfLinks) return out;
            }
        }
    }
    return out;
}

int PrintJob::firstPdfPageOf(int sheet, const QVector<int> &pdfPageOf) const
{
    for (size_t i = 0; i < m_pages.size() && int(i) < pdfPageOf.size(); ++i)
        if (m_sheets[size_t(m_pages[i].info)].sheet == sheet && pdfPageOf[int(i)] >= 0) return pdfPageOf[int(i)];
    return -1;
}

QVector<jp::PdfBookmark> PrintJob::pdfBookmarks(const QVector<int> &pdfPageOf)
{
    QVector<jp::PdfBookmark> out;
    QSet<int> seen;
    for (int page = 0; page < pageCount() && page < pdfPageOf.size() && out.size() < jp::kMostPdfBookmarks; ++page) {
        if (pdfPageOf[page] < 0) continue;
        const int sheet = m_sheets[size_t(m_pages[size_t(page)].info)].sheet;
        if (seen.contains(sheet)) continue;
        seen.insert(sheet);
        out.push_back({m_source->sheetName(sheet), 0, pdfPageOf[page]});
    }
    return out;
}

jp::PdfInfo PrintJob::pdfInfoForFile() const
{
    jp::PdfInfo info;
    if (m_options.hasPdfInfo) info = m_options.pdfInfo;
    else info.title = QFileInfo(m_options.fileName).completeBaseName();
    if (info.program.isEmpty()) info.program = QStringLiteral("JeffCrunch");
    if (!info.created.isValid()) info.created = (m_now.isValid() ? m_now : QDateTime::currentDateTimeUtc()).toUTC();
    return info;
}

bool PrintJob::exportPdf(const QString &path, const QVector<int> &pages, QString *error, const std::function<bool(int, int)> &progress, int perSheet, const jp::PdfProtection *protect,
                         const QVector<QVector<int>> *sheets)
{
    auto fail = [&](const QString &text) {
        if (error) *error = text;
        return false;
    };
    m_cancelled = false;
    perSheet = cleanPerSheet(perSheet);
    if (pages.isEmpty()) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "There are no pages to put in the file."));
    const QVector<QVector<int>> groups = sheets ? *sheets : groupsOf(pages, perSheet);
    const bool encrypted = protect && protect->isSet();
    // The PDF's page of each page of the job (-1: not in the PDF): the links (one page to a sheet only) and the bookmarks (a sheet's first page) go by it.
    QVector<int> pdfPageOf(pageCount(), -1);
    {
        int next = 0;
        const int most = int(std::min<qint64>(groups.size(), kMaxSentPages));
        for (int at = 0; at < most; ++at) {
            const QVector<int> &group = groups[at];
            if (group.isEmpty() || group.first() < 0 || group.first() >= pageCount()) continue;
            for (const int page : group)
                if (page >= 0 && page < pageCount() && pdfPageOf[page] < 0) pdfPageOf[page] = next;
            ++next;
        }
    }
    QVector<PdfPlace> places(pageCount());
    {
        const int most = int(std::min<qint64>(groups.size(), kMaxSentPages));
        for (int at = 0; at < most; ++at) {
            const QVector<int> &group = groups[at];
            if (group.isEmpty() || group.first() < 0 || group.first() >= pageCount()) continue;
            placeOnSheet(group, sheetFor(group, perSheet), perSheet, &places);
        }
    }
    const QVector<jp::PdfLink> links = pdfLinks(pdfPageOf, places);
    const QVector<jp::PdfBookmark> marks = pdfBookmarks(pdfPageOf);
    const bool tagging = m_options.pdfTags;
    const jp::PdfInfo info = pdfInfoForFile();
    // The PDF is made in memory (the bookmarks, the links, and the tags are added to the finished file; with a password the plain PDF is never on the disk).
    QSaveFile file(path);
    QBuffer memory;
    const bool archiving = m_options.archival && !encrypted;
    const bool inMemory = encrypted || tagging || archiving || !links.isEmpty() || !marks.isEmpty();
    if (inMemory) memory.open(QIODevice::WriteOnly);
    else if (!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
    auto make = [&](jp::PdfTagger *tagger) {
        if (inMemory) {
            memory.close();
            memory.setData(QByteArray());
            memory.open(QIODevice::WriteOnly);
        }
        // (the grids made for a page are given the tagger; those that stay from the last export are given none)
        m_tagger = tagger;
        m_artifactTag = tagger ? tagger->add({QStringLiteral("Artifact"), QString(), -1, false}) : -1;
        for (Context &c : m_contexts)
            if (c.painter) c.painter->setTagger(tagger);
        bool ok = true;
        {
            QPdfWriter pdf(inMemory ? static_cast<QIODevice *>(&memory) : static_cast<QIODevice *>(&file));
            pdf.setCreator(info.program);
            pdf.setTitle(info.title);
            // PDF/A-1b: Qt puts every font in the file, writes the identification (part 1, conformance B) and an sRGB output intent, and leaves transparency out
            if (m_options.archival && !encrypted) pdf.setPdfVersion(QPagedPaintDevice::PdfVersion_A1b);
            // (the metadata packet says what the information dictionary will: both are made from the same properties)
            if (m_options.hasPdfInfo || (m_options.archival && !encrypted)) pdf.setDocumentXmpMetadata(jp::pdfInfoXmp(info, m_options.archival && !encrypted, 1));
            // (a gray job is made gray by the PDF itself, so that its words stay words and can be tagged, and not a picture of each page)
            if (m_options.grayscale) pdf.setColorModel(QPdfWriter::ColorModel::Grayscale);
            m_grayInPdf = m_options.grayscale;
            constexpr int kResolution = 1200;
            pdf.setResolution(kResolution);
            QPainter p;
            bool started = false;
            int done = 0;
            // (no more pages than a job may send, as printing holds it)
            const int total = int(std::min<qint64>(groups.size(), kMaxSentPages));
            // (a tagged PDF's tagging is a second part of the work, as long as the pages: the bar counts both)
            const int span = tagger ? 2 * total : total;
            for (int at = 0; at < total; ++at) {
                if (tagger && tagger->full()) break;
                const QVector<int> &group = groups[at];
                if (group.isEmpty() || group.first() < 0 || group.first() >= pageCount()) continue;
                const QSizeF sheet = sheetFor(group, perSheet);
                setSheet(&pdf, sheet);
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
                paintGroup(&p, group, sheet, perSheet);
                p.restore();
                if (progress && !progress(++done, span)) {
                    m_cancelled = true;
                    ok = false;
                    break;
                }
            }
            if (started) p.end();
            ok = ok && started;
        }
        m_grayInPdf = false;
        m_tagger = nullptr;
        for (Context &c : m_contexts)
            if (c.painter) c.painter->setTagger(nullptr);
        return ok;
    };
    jp::PdfTagger tagger;
    bool ok = make(tagging ? &tagger : nullptr);
    // A PDF that would have more tags than the bound is made again without any, never half tagged.
    const bool tooBig = tagging && ok && tagger.full();
    if (tooBig) ok = make(nullptr);
    if (!ok) {
        file.cancelWriting();
        if (m_cancelled) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "It was cancelled, and no file was made."));
        return fail(QCoreApplication::translate("jp::crunch::PrintJob", "The pages could not be put in the file."));
    }
    if (inMemory) {
        QByteArray bytes = memory.data();
        // PDF/A-1 allows no transparency: what could not be made solid is not made into a file that claims to be one.
        if (archiving) {
            const jp::PdfTransparency clear = jp::pdfTransparency(bytes);
            if (clear.refused()) {
                file.cancelWriting();
                return fail(jp::pdfTransparencyRefusal(clear, QStringLiteral("PDF/A"), [&](int pdfPage) {
                    QStringList on;
                    for (int page = 0; page < pdfPageOf.size(); ++page)
                        if (pdfPageOf[page] == pdfPage - 1) on << QString::number(page + 1);
                    return on.isEmpty() ? QString()
                                        : on.size() == 1 ? QCoreApplication::translate("jp::crunch::PrintJob", "page %1").arg(on.first())
                                                         : QCoreApplication::translate("jp::crunch::PrintJob", "pages %1").arg(on.join(QStringLiteral(", ")));
                }));
            }
        }
        if (tooBig) {
            const QString said = QCoreApplication::translate("jp::crunch::PrintJob", "This PDF is too large to tag for screen readers, so it was saved without tags.");
            if (!m_messages.contains(said)) m_messages << said;
        } else if (tagging) {
            jp::PdfTagOptions tagOptions;
            tagOptions.language = QLocale().bcp47Name();
            tagOptions.title = info.title;
            const int pagesMade = std::max<int>(1, int(std::min<qint64>(groups.size(), kMaxSentPages)));
            if (progress)
                tagOptions.progress = [&](int done, int count) { return progress(pagesMade + int(qint64(done) * pagesMade / std::max(1, count)), 2 * pagesMade); };
            const jp::PdfTagResult made = jp::tagPdf(bytes, tagger, tagOptions);
            if (made.cancelled) {
                m_cancelled = true;
                file.cancelWriting();
                return fail(QCoreApplication::translate("jp::crunch::PrintJob", "It was cancelled, and no file was made."));
            }
            if (made.pdf.isEmpty()) {
                // The marks could not be taken out: the pages are made again, without them.
                if (!make(nullptr)) {
                    file.cancelWriting();
                    return fail(QCoreApplication::translate("jp::crunch::PrintJob", "The pages could not be put in the file."));
                }
                bytes = memory.data();
            } else {
                bytes = made.pdf;
            }
            if (made.pdf.isEmpty() || !made.tagged) {
                const QString said = QCoreApplication::translate("jp::crunch::PrintJob", "The PDF is not tagged for screen readers: %1.").arg(made.why);
                if (!m_messages.contains(said)) m_messages << said;
            }
        }
        if (!links.isEmpty()) bytes = jp::addPdfLinks(bytes, links);
        if (!marks.isEmpty()) bytes = jp::addPdfBookmarks(bytes, marks);
        bytes = jp::setPdfInfo(bytes, info);
        if (encrypted) {
            QString why;
            bytes = jp::encryptPdf(bytes, *protect, &why);
            if (bytes.isEmpty()) return fail(why);
        }
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size()) return fail(file.errorString());
    }
    if (!file.commit()) return fail(file.errorString());
    if (!inMemory) {
        // (a PDF written straight to the file has its properties put on at its end, without reading it all)
        QFile written(path);
        if (written.open(QIODevice::ReadWrite)) jp::appendPdfInfo(&written, info);
    }
    return true;
}

bool PrintJob::printTo(QPrinter *printer, const QVector<int> &pages, QString *error, const std::function<bool(int, int)> &progress, int copies, bool collate, int perSheet,
                       const QVector<QVector<int>> *sheets)
{
    auto fail = [&](const QString &text) {
        if (error) *error = text;
        return false;
    };
    m_cancelled = false;
    if (pages.isEmpty() && !sheets) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "There are no pages to print."));
    // (the pages and their copies, in the order they come out, are worked out one at a time, not listed)
    copies = std::max(1, copies);
    perSheet = cleanPerSheet(perSheet);
    const QVector<QVector<int>> groups = sheets ? *sheets : groupsOf(pages, perSheet);
    const qint64 n = groups.size();
    if (n == 0) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "There are no pages to print."));
    const qint64 total = std::min<qint64>(n * copies, kMaxSentPages);
    // (a blank sheet, from both sides by hand, is the size of the sheets around it)
    QSizeF blankSheet;
    for (const QVector<int> &g : groups)
        if (!g.isEmpty() && g.first() >= 0 && g.first() < pageCount()) {
            blankSheet = sheetFor(g, perSheet);
            break;
        }
    printer->setFullPage(true);
    QPainter p;
    bool started = false;
    for (qint64 i = 0; i < total; ++i) {
        const QVector<int> &group = groups[int(collate ? i % n : i / copies)];
        const bool blank = group.isEmpty();
        if (!blank && (group.first() < 0 || group.first() >= pageCount())) continue;
        const QSizeF sheet = blank ? blankSheet : sheetFor(group, perSheet);
        setSheet(printer, sheet);
        if (!started) {
            started = p.begin(printer);
            if (!started) return fail(QCoreApplication::translate("jp::crunch::PrintJob", "The printer could not be used."));
        } else {
            printer->newPage();
        }
        p.save();
        const double k = double(printer->logicalDpiX()) / 72.0;
        p.scale(k, k);
        if (!blank) paintGroup(&p, group, sheet, perSheet);
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

QVector<QVector<int>> sheetGroups(const QVector<int> &pages, int perSheet) { return groupsOf(pages, cleanPerSheet(perSheet)); }

HandRuns<QVector<int>> handSheetRuns(const QVector<int> &pages, int perSheet)
{
    perSheet = std::max(1, perSheet);
    return handRuns(groupsOf(pages, perSheet), QVector<int>());
}

QVector<int> pagesOfSide(const QVector<int> &pages, PrintSides sides)
{
    if (sides == PrintSides::All) return pages;
    QVector<int> kept;
    for (const int page : pages)
        if ((page % 2 == 0) == (sides == PrintSides::Odd)) kept << page;
    return kept;
}

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
