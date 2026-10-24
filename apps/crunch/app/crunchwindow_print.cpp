// Printing from JeffCrunch's window: the jobs the Print screen, Export to PDF, and the page commands ask for (core/printjob.h). What the person chooses on the Print screen and in the Page
// Setup dialog is in their own files.

#include "app/crunchwindow.h"

#include "core/appinfo.h"
#include "core/printjob.h"
#include "core/sheetobjects.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QPrinter>
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

bool CrunchWindow::exportPdfTo(const QString &path, crunch::PrintJobOptions options, const QString &pageRange, QString *why)
{
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
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const bool ok = job->exportPdf(path, pages, why);
    QApplication::restoreOverrideCursor();
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
    if (!exportPdfTo(path, options, QString(), &why)) {
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
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ok = job->exportPdf(path, pages, &why);
        QApplication::restoreOverrideCursor();
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
        const QVector<int> sheets = again ? crunch::sheetOrder(pages, c.copies, c.collate) : pages;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ok = job->printTo(&p, sheets, &why);
        QApplication::restoreOverrideCursor();
        if (!ok) notice(tr("Print"), tr("%1 couldn't send \"%2\" to the printer.\n%3").arg(appInfo().name, displayName(), why));
        else setStatus(tr("Sent %1 to the printer").arg(displayName()));
    }
    if (ok) armPageBreaks();
    return ok;
}

void CrunchWindow::armPageBreaks()
{
    m_breaksArmed = true;
    m_breaksTimer.start();
}

// Where the pages of the sheet in view begin, as lines over its cells.
void CrunchWindow::updatePageBreaks()
{
    if (!m_view) return;
    if (!m_breaksArmed || !act(QStringLiteral("view.pageBreaks"))->isChecked() || !m_loaded || !m_haveCalc || m_chartSheetShown) {
        if (m_view->hasPageBreaks()) m_view->setPageBreaks({}, {});
        return;
    }
    crunch::PrintJobOptions options;
    options.what = crunch::PrintWhat::ActiveSheet;
    const std::unique_ptr<crunch::PrintJob> job = makePrintJob(options);
    const jp::grid::SheetPrint *print = job ? job->printOf(m_sheet) : nullptr;
    const jp::office::sheet::PageSetup *setup = job ? job->setupOf(m_sheet) : nullptr;
    std::vector<grid::GridView::PageBreakLine> rows, cols;
    if (print) {
        auto manual = [](const QVector<jp::office::sheet::PageBreak> &list, qint64 before) {
            for (const jp::office::sheet::PageBreak &b : list)
                if (b.manual && qint64(b.before) == before) return true;
            return false;
        };
        for (const jp::grid::PageCut &page : print->pages) {
            if (page.area < 0 || size_t(page.area) >= print->areas.size()) continue;
            const jp::office::sheet::CellRange &area = print->areas[size_t(page.area)];
            if (page.rowBand > 0 && page.colBand == 0) rows.push_back({page.row1, qint64(area.col1), qint64(area.col2), manual(setup->rowBreaks, page.row1) && !setup->fitToPage});
            if (page.colBand > 0 && page.rowBand == 0) cols.push_back({page.col1, qint64(area.row1), qint64(area.row2), manual(setup->colBreaks, page.col1) && !setup->fitToPage});
        }
    }
    m_view->setPageBreaks(std::move(rows), std::move(cols));
}

} // namespace jp
