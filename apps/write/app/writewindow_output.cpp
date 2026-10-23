// Printing and exporting from JeffWrite's window: Export to PDF, and the printing the File screen's Print page asks for.

#include "app/writewindow.h"

#include "app/filedialogs.h"
#include "app/pageoutput.h"
#include "app/pageview.h"
#include "core/appinfo.h"
#include "core/printing.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPrinter>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTemporaryFile>
#include <memory>

namespace jp {

namespace {
// What a printer or a PDF shows of a header that is being edited is what the pages have: its fields that show a heading's text are worked out for each page, as they are for any header.
struct EditedStoryHeld {
    flow::Paginator &pager;
    const QTextDocument *story;
    EditedStoryHeld(flow::Paginator &p, const QTextDocument *s) : pager(p), story(s) { pager.setEditedStory(nullptr); }
    ~EditedStoryHeld() { pager.setEditedStory(story); }
};
} // namespace

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
    EditedStoryHeld held(m_doc->pager(), m_view->inHeaderFooter() ? m_view->headerFooterEdit().story : nullptr);
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
    EditedStoryHeld held(m_doc->pager(), m_view->inHeaderFooter() ? m_view->headerFooterEdit().story : nullptr);
    const QVector<int> pages = outputPages(options);
    if (pages.isEmpty()) return false;
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(displayName());
    QString path;
    // A PDF file is made in a temporary file and put in place when it is whole, as the export does, so that a file that is there is not lost to one that fails.
    std::unique_ptr<QTemporaryFile> made;
    if (options.toFile) {
        path = pdfPathToWrite(tr("Save as PDF"));
        if (path.isEmpty()) return false;
        made = std::make_unique<QTemporaryFile>(QDir::tempPath() + QStringLiteral("/jeffwrite-XXXXXX.pdf"));
        if (!made->open()) {
            QMessageBox::warning(this, tr("Save as PDF"), tr("%1 couldn't make \"%2\".\n%3").arg(appInfo().name, QFileInfo(path).fileName(), made->errorString()));
            return false;
        }
        made->close();
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(made->fileName());
    } else if (!options.printer.isEmpty()) {
        printer.setPrinterName(options.printer);
    }
    printer.setCopyCount(std::max(1, options.copies));
    printer.setCollateCopies(options.collate);
    if (options.grayscale) printer.setColorMode(QPrinter::GrayScale);
    if (m_printerPreparer) m_printerPreparer(printer);
    // A printer that can't make copies is sent the pages again, in the order the copies are to come out.
    const bool again = !options.toFile && !printer.supportsMultipleCopies();
    const QVector<int> sheets = again ? write::sheetOrder(pages, options.copies, options.collate) : pages;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool ok = write::paintSheets(*m_doc, &printer, sheets, m_view->pageCount(), printer.colorMode() == QPrinter::GrayScale);
    QString err;
    if (ok && made) {
        QFile whole(made->fileName());
        QSaveFile file(path);
        ok = whole.open(QIODevice::ReadOnly) && file.open(QIODevice::WriteOnly);
        while (ok && !whole.atEnd()) ok = file.write(whole.read(1 << 20)) >= 0;
        if (ok) ok = file.commit();
        else file.cancelWriting();
        if (!ok) err = file.error() != QFileDevice::NoError ? file.errorString() : whole.errorString();
    }
    QApplication::restoreOverrideCursor();
    if (!ok) {
        if (made && !err.isEmpty()) QMessageBox::warning(this, tr("Save as PDF"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        else QMessageBox::warning(this, tr("Print"), tr("%1 couldn't send \"%2\" to the printer.").arg(appInfo().name, displayName()));
        return false;
    }
    statusBar()->showMessage(options.toFile ? tr("Saved %1").arg(QFileInfo(path).fileName()) : tr("Sent %1 to the printer").arg(displayName()), 5000);
    return true;
}

} // namespace jp
