// Printing and exporting from JeffWrite's window: Export to PDF, and the printing the File screen's Print page asks for.

#include "app/writewindow.h"

#include "app/filedialogs.h"
#include "app/pageview.h"
#include "app/pagepaint.h"
#include "core/appinfo.h"

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPrintDialog>
#include <QPrinter>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
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
    // "1-3, 5": the pages that are there, once each.
    QSet<int> seen;
    for (const QString &part : options.pages.split(QRegularExpression(QStringLiteral("[,;]")), Qt::SkipEmptyParts)) {
        const QStringList ends = part.trimmed().split(QLatin1Char('-'));
        bool ok1 = false, ok2 = true;
        const int from = ends.value(0).trimmed().toInt(&ok1);
        const int to = ends.size() > 1 ? ends.value(1).trimmed().toInt(&ok2) : from;
        if (!ok1 || !ok2 || ends.size() > 2) continue;
        for (int p = std::max(1, std::min(from, to)); p <= std::min(count, std::max(from, to)); ++p)
            if (!seen.contains(p)) {
                seen.insert(p);
                out << p - 1;
            }
    }
    return out;
}

bool WriteWindow::exportPdf()
{
    if (!m_doc) return false;
    const QString dir = m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(m_path).absolutePath();
    const QString filter = tr("PDF (*.pdf)");
    const QString suggested = dir + QLatin1Char('/') + displayName() + QStringLiteral(".pdf");
    QString path = m_pathChooser ? m_pathChooser(suggested, filter) : askSavePath(this, tr("Export to PDF"), suggested, filter);
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".pdf");
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
            QPainter p;
            bool started = false;
            for (int index : pages) {
                const auto page = m_doc->pager().materialize(index);
                if (!page) continue;
                pdf.setPageSize(QPageSize(page->size, QPageSize::Point, QString(), QPageSize::ExactMatch));
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
                p.scale(1200.0 / 72, 1200.0 / 72);
                LayoutCache cache;
                write::PageDrawing d;
                d.output = true;
                d.textOutput = TextOutput::Pdf;
                d.pageCount = m_view->pageCount();
                write::paintPage(&p, *page, &cache, d);
                p.restore();
            }
            if (started) p.end();
            ok = ok && started;
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
    const QSizeF first = m_doc->pager().page(pages.first()).size;
    printer.setPageSize(QPageSize(first, QPageSize::Point, QString(), QPageSize::FuzzyMatch));
    printer.setFullPage(true);
    printer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Point);
    QPrintDialog dlg(&printer, this);
    dlg.setWindowTitle(tr("Print"));
    if (dlg.exec() != QDialog::Accepted) return false;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QPainter p;
    bool ok = p.begin(&printer);
    if (ok) {
        const double scale = printer.resolution() / 72.0;
        // A printer that can't make copies is sent the pages again.
        const int copies = printer.supportsMultipleCopies() ? 1 : std::max(1, printer.copyCount());
        bool firstSheet = true;
        for (int c = 0; c < copies; ++c)
            for (int index : pages) {
                const auto page = m_doc->pager().materialize(index);
                if (!page) continue;
                if (!firstSheet) {
                    printer.setPageSize(QPageSize(page->size, QPageSize::Point, QString(), QPageSize::FuzzyMatch));
                    printer.newPage();
                }
                firstSheet = false;
                p.save();
                p.scale(scale, scale);
                LayoutCache cache;
                write::PageDrawing d;
                d.output = true;
                d.textOutput = TextOutput::Print;
                d.pageCount = m_view->pageCount();
                d.grayscale = printer.colorMode() == QPrinter::GrayScale;
                write::paintPage(&p, *page, &cache, d);
                p.restore();
            }
        p.end();
    }
    QApplication::restoreOverrideCursor();
    if (!ok) {
        QMessageBox::warning(this, tr("Print"), tr("%1 couldn't send \"%2\" to the printer.").arg(appInfo().name, displayName()));
        return false;
    }
    statusBar()->showMessage(tr("Sent %1 to the printer").arg(displayName()), 5000);
    return true;
}

} // namespace jp
