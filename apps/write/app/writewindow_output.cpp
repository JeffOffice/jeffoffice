// Printing and exporting from JeffWrite's window: Export to PDF, and the printing the File screen's Print page asks for.

#include "app/writewindow.h"

#include "app/filedialogs.h"
#include "app/pageoutput.h"
#include "app/pageview.h"
#include "core/appinfo.h"
#include "core/printing.h"

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPrinter>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStatusBar>

namespace jp {

QVector<int> WriteWindow::outputPages(const WriteOutputOptions &options)
{
    QVector<int> out;
    if (!m_doc) return out;
    m_view->paginateAll();
    const int count = m_view->pageCount();
    if (options.range == WriteOutputOptions::Range::Current) {
        out << std::clamp(m_view->currentPage(), 0, std::max(0, count - 1));
        return out;
    }
    if (options.range == WriteOutputOptions::Range::All) {
        for (int i = 0; i < count; ++i) out << i;
        return out;
    }
    return write::parsePageRange(options.pages, count);
}

QString WriteWindow::pdfPathToWrite(const QString &title)
{
    const QString dir = m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(m_path).absolutePath();
    const QString filter = tr("PDF (*.pdf)");
    const QString suggested = dir + QLatin1Char('/') + displayName() + QStringLiteral(".pdf");
    QString path = m_pathChooser ? m_pathChooser(suggested, filter) : askSavePath(this, title, suggested, filter);
    if (!path.isEmpty() && QFileInfo(path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".pdf");
    return path;
}

bool WriteWindow::exportPdf()
{
    if (!m_doc) return false;
    const QString path = pdfPathToWrite(tr("Export to PDF"));
    if (path.isEmpty()) return false;
    const QVector<int> pages = outputPages(WriteOutputOptions());
    QString err;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool ok = !pages.isEmpty();
    QSaveFile file(path);
    if (ok) ok = file.open(QIODevice::WriteOnly);
    if (ok) {
        {
            QPdfWriter pdf(&file);
            pdf.setCreator(appInfo().name);
            pdf.setTitle(displayName());
            pdf.setResolution(1200);
            pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
            ok = write::paintSheets(*m_doc, &pdf, pages, m_view->pageCount());
        }
        if (ok) ok = file.commit();
        else file.cancelWriting();
    }
    if (!ok) err = file.error() != QFileDevice::NoError ? file.errorString() : tr("The pages can't be made.");
    QApplication::restoreOverrideCursor();
    if (!ok) {
        QMessageBox::warning(this, tr("Export to PDF"), tr("%1 couldn't export \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    statusBar()->showMessage(tr("Exported %1").arg(QFileInfo(path).fileName()), 5000);
    return true;
}

bool WriteWindow::printOutput(const WriteOutputOptions &options)
{
    if (!m_doc) return false;
    const QVector<int> pages = outputPages(options);
    if (pages.isEmpty()) return false;
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(displayName());
    QString path;
    if (options.toFile) {
        path = pdfPathToWrite(tr("Save as PDF"));
        if (path.isEmpty()) return false;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(path);
    } else if (!options.printer.isEmpty()) {
        printer.setPrinterName(options.printer);
    }
    printer.setCopyCount(std::max(1, options.copies));
    printer.setCollateCopies(options.collate);
    if (options.grayscale) printer.setColorMode(QPrinter::GrayScale);
    // A printer that can't make copies is sent the pages again, in the order the copies are to come out.
    const bool again = !options.toFile && !printer.supportsMultipleCopies();
    const QVector<int> sheets = again ? write::sheetOrder(pages, options.copies, options.collate) : pages;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const bool ok = write::paintSheets(*m_doc, &printer, sheets, m_view->pageCount(), printer.colorMode() == QPrinter::GrayScale);
    QApplication::restoreOverrideCursor();
    if (!ok) {
        QMessageBox::warning(this, tr("Print"), tr("%1 couldn't send \"%2\" to the printer.").arg(appInfo().name, displayName()));
        return false;
    }
    statusBar()->showMessage(options.toFile ? tr("Saved %1").arg(QFileInfo(path).fileName()) : tr("Sent %1 to the printer").arg(displayName()), 5000);
    return true;
}

} // namespace jp
