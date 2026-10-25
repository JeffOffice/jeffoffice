// Printing from JeffCrunch's window: the jobs the Print screen, Export to PDF, and the page commands ask for (core/printjob.h). What the person chooses on the Print screen and in the Page
// Setup dialog is in their own files.

#include "app/crunchwindow.h"

#include <QSet>

#include "core/appinfo.h"
#include "core/printjob.h"
#include "core/sheetobjects.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QPrinter>
#include <QProgressDialog>
#include <QStandardPaths>

namespace jp {

std::unique_ptr<crunch::PrintJob> CrunchWindow::makePrintJob(crunch::PrintJobOptions options, QString *why)
{
    if (!m_loaded || !m_loaded->book || !m_haveCalc || !m_loaded->calc) {
        if (why) *why = tr("The workbook is still being opened. It can be printed when it is open.");
        return nullptr;
    }
    options.activeSheet = m_sheet;
    options.fileName = QFileInfo(m_path).fileName();
    options.folder = m_path.isEmpty() ? QString() : QFileInfo(m_path).absolutePath();
    if (options.what == crunch::PrintWhat::Selection && !options.selection.valid) options.selection = selectedRange();
    auto source = std::make_unique<crunch::BookPrintSource>(m_loaded.get(), &m_values, m_session ? &m_session->pages() : nullptr, m_objects.get());
    auto job = std::make_unique<crunch::PrintJob>(std::move(source), options);
    if (!job->build(why)) return nullptr;
    return job;
}

std::function<bool(int, int)> CrunchWindow::progressFunction(const QString &title, const QString &text)
{
    if (m_progressHook) return m_progressHook;
    // (a bar with Cancel, shown only when the job takes a while; the events are let through so that Cancel can be pressed)
    auto dialog = std::make_shared<QProgressDialog>(text, tr("Cancel"), 0, 100, this);
    dialog->setWindowTitle(title);
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setMinimumDuration(700);
    dialog->setAutoClose(false);
    dialog->setAutoReset(false);
    return [dialog](int done, int total) {
        dialog->setMaximum(std::max(1, total));
        dialog->setValue(done);
        QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        return !dialog->wasCanceled();
    };
}

bool CrunchWindow::exportPdfTo(const QString &path, crunch::PrintJobOptions options, const QString &pageRange, QString *why, bool *cancelled)
{
    if (cancelled) *cancelled = false;
    const std::unique_ptr<crunch::PrintJob> job = makePrintJob(options, why);
    if (!job) return false;
    QVector<int> pages;
    if (pageRange.trimmed().isEmpty()) {
        for (int i = 0; i < job->pageCount(); ++i) pages << i;
    } else {
        pages = crunch::parsePageRange(pageRange, job->pageCount());
        if (pages.isEmpty()) {
            if (why) *why = tr("Those page numbers are not pages of this job. The job has %1 pages.").arg(job->pageCount());
            return false;
        }
    }
    const bool ok = job->exportPdf(path, pages, why, progressFunction(tr("Export to PDF"), tr("Making the pages...")));
    if (!ok && cancelled) *cancelled = job->wasCancelled();
    return ok;
}

QPrinter &CrunchWindow::printer()
{
    if (!m_printer) m_printer = std::make_unique<QPrinter>(QPrinter::HighResolution);
    return *m_printer;
}

bool CrunchWindow::exportPdf(bool wholeWorkbook)
{
    if (!m_loaded || !m_haveCalc) {
        notice(tr("Export to PDF"), tr("The workbook is still being opened. It can be exported when it is open."));
        return false;
    }
    crunch::PrintJobOptions options = m_printChoices->job;
    options.what = wholeWorkbook ? crunch::PrintWhat::Workbook : crunch::PrintWhat::ActiveSheet;
    options.activeSheet = m_sheet;
    const QString dir = m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(m_path).absolutePath();
    const QString start = QDir(dir).filePath(displayName() + QStringLiteral(".pdf"));
    QString path = askForSavePath(tr("Export to PDF"), start, tr("PDF (*.pdf)"));
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".pdf");
    QString why;
    bool cancelled = false;
    if (!exportPdfTo(path, options, QString(), &why, &cancelled)) {
        if (cancelled) {
            setStatus(tr("Export was cancelled"));
            return false;
        }
        notice(tr("Export to PDF"), tr("%1 couldn't export \"%2\".\n%3").arg(appInfo().name, QFileInfo(path).fileName(), why));
        return false;
    }
    setStatus(tr("Exported %1").arg(QFileInfo(path).fileName()));
    armPageBreaks();
    return true;
}

bool CrunchWindow::printWith(const crunch::PrintChoices &c)
{
    QString why;
    std::unique_ptr<crunch::PrintJob> job = makePrintJob(c.job, &why);
    if (!job) {
        notice(tr("Print"), why);
        return false;
    }
    QVector<int> pages;
    if (c.pageRange.trimmed().isEmpty()) {
        for (int i = 0; i < job->pageCount(); ++i) pages << i;
    } else {
        pages = crunch::parsePageRange(c.pageRange, job->pageCount());
    }
    if (pages.isEmpty()) {
        notice(tr("Print"), tr("Those page numbers are not pages of this job. The job has %1 pages.").arg(job->pageCount()));
        return false;
    }
    bool ok = false;
    if (c.toFile) {
        const QString dir = m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(m_path).absolutePath();
        QString path = askForSavePath(tr("Save as PDF"), QDir(dir).filePath(displayName() + QStringLiteral(".pdf")), tr("PDF (*.pdf)"));
        if (path.isEmpty()) return false;
        if (QFileInfo(path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".pdf");
        ok = job->exportPdf(path, pages, &why, progressFunction(tr("Save as PDF"), tr("Making the pages...")));
        if (!ok && job->wasCancelled()) {
            setStatus(tr("Saving the PDF was cancelled"));
            return false;
        }
        if (!ok) notice(tr("Save as PDF"), tr("%1 couldn't save \"%2\".\n%3").arg(appInfo().name, QFileInfo(path).fileName(), why));
        else setStatus(tr("Saved %1").arg(QFileInfo(path).fileName()));
    } else {
        QPrinter &p = printer();
        p.setDocName(displayName());
        if (!c.printer.isEmpty() && p.printerName() != c.printer) p.setPrinterName(c.printer);
        p.setCopyCount(std::max(1, c.copies));
        p.setCollateCopies(c.collate);
        if (m_printerPreparer) m_printerPreparer(p);
        // a printer that can't make copies is sent the pages again, in the order the copies are to come out
        const bool again = p.outputFormat() == QPrinter::NativeFormat && !p.supportsMultipleCopies();
        ok = job->printTo(&p, pages, &why, progressFunction(tr("Printing"), tr("Sending the pages...")), again ? c.copies : 1, c.collate);
        if (!ok && job->wasCancelled()) {
            setStatus(tr("Printing was cancelled"));
            return false;
        }
        if (!ok) notice(tr("Print"), tr("%1 couldn't send \"%2\" to the printer.\n%3").arg(appInfo().name, displayName(), why));
        else setStatus(tr("Sent %1 to the printer").arg(displayName()));
    }
    if (ok) armPageBreaks();
    return ok;
}

void CrunchWindow::armPageBreaks()
{
    m_breaksArmed = true;
    schedulePageBreaks();
}

void CrunchWindow::schedulePageBreaks()
{
    cancelPageBreaks();
    m_breaksTimer.start();
}

void CrunchWindow::cancelPageBreaks()
{
    m_breaksStepTimer.stop();
    m_breakJob.reset();
}

// Where the pages of the sheet in view begin, as lines over its cells.
void CrunchWindow::pageBreakLines(const jp::grid::SheetPrint &print, const jp::office::sheet::PageSetup &setup, std::vector<grid::GridView::PageBreakLine> *rows,
                                  std::vector<grid::GridView::PageBreakLine> *cols)
{
    // (the breaks set by hand, looked up in a set: a sheet of thousands of pages and of breaks is not searched once for each page)
    QSet<qint64> rowBreaks, colBreaks;
    if (!setup.fitToPage) {
        for (const jp::office::sheet::PageBreak &b : setup.rowBreaks)
            if (b.manual) rowBreaks.insert(qint64(b.before));
        for (const jp::office::sheet::PageBreak &b : setup.colBreaks)
            if (b.manual) colBreaks.insert(qint64(b.before));
    }
    for (const jp::grid::PageCut &page : print.pages) {
        if (page.area < 0 || size_t(page.area) >= print.areas.size()) continue;
        const jp::office::sheet::CellRange &area = print.areas[size_t(page.area)];
        if (page.rowBand > 0 && page.colBand == 0) rows->push_back({page.row1, qint64(area.col1), qint64(area.col2), rowBreaks.contains(page.row1)});
        if (page.colBand > 0 && page.rowBand == 0) cols->push_back({page.col1, qint64(area.row1), qint64(area.row2), colBreaks.contains(page.col1)});
    }
}

// The most work one slice of the page breaks does (see grid/axis.h: a place, a row, a band, a page each); a slice of this is a few milliseconds at most.
static constexpr qint64 kPageBreakSliceWork = 4096;

qint64 CrunchWindow::pageBreakSliceWork() { return kPageBreakSliceWork; }

void CrunchWindow::updatePageBreaks()
{
    cancelPageBreaks();
    if (!m_view) return;
    if (!m_breaksArmed || !act(QStringLiteral("view.pageBreaks"))->isChecked() || !m_loaded || !m_haveCalc || m_chartSheetShown) {
        if (m_view->hasPageBreaks()) m_view->setPageBreaks({}, {});
        return;
    }
    crunch::PrintJobOptions options;
    options.what = crunch::PrintWhat::ActiveSheet;
    options.activeSheet = m_sheet;
    options.fileName = QFileInfo(m_path).fileName();
    options.folder = m_path.isEmpty() ? QString() : QFileInfo(m_path).absolutePath();
    auto source = std::make_unique<crunch::BookPrintSource>(m_loaded.get(), &m_values, m_session ? &m_session->pages() : nullptr, m_objects.get());
    auto job = std::make_unique<crunch::PrintJob>(std::move(source), options);
    if (!job->beginBuild()) {
        if (m_view->hasPageBreaks()) m_view->setPageBreaks({}, {});
        return;
    }
    m_breakJob = std::move(job);
    m_breakBook = m_loaded.get();
    m_breakSheet = m_sheet;
    m_breakStats = PageBreakWork();
    stepPageBreaks();
}

void CrunchWindow::stepPageBreaks()
{
    if (!m_breakJob) return;
    // (the workbook or the sheet in view is not the one the job began on: its lines would be another sheet's)
    if (!m_view || !m_loaded || m_loaded.get() != m_breakBook || !m_haveCalc || m_sheet != m_breakSheet) {
        cancelPageBreaks();
        return;
    }
    jp::grid::Work work(kPageBreakSliceWork);
    const crunch::PrintJob::BuildState state = m_breakJob->buildStep(work);
    ++m_breakStats.slices;
    m_breakStats.most = std::max(m_breakStats.most, work.used);
    if (state == crunch::PrintJob::BuildState::Working) {
        m_breaksStepTimer.start();
        return;
    }
    const std::unique_ptr<crunch::PrintJob> job = std::move(m_breakJob);
    std::vector<grid::GridView::PageBreakLine> rows, cols;
    if (state == crunch::PrintJob::BuildState::Done) {
        const jp::grid::SheetPrint *print = job->printOf(m_sheet);
        const jp::office::sheet::PageSetup *setup = job->setupOf(m_sheet);
        if (print && setup) pageBreakLines(*print, *setup, &rows, &cols);
    }
    m_view->setPageBreaks(std::move(rows), std::move(cols));
}

} // namespace jp
