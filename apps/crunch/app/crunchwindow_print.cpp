// Printing from JeffCrunch's window: the jobs the Print screen, Export to PDF, and the page commands ask for (core/printjob.h). What the person chooses on the Print screen and in the Page
// Setup dialog is in their own files.

#include "app/crunchwindow.h"

#include "core/printjob.h"
#include "core/sheetobjects.h"

#include <QApplication>
#include <QFileInfo>

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

} // namespace jp
