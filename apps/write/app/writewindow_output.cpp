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
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <algorithm>
#include <QStatusBar>
#include <QTemporaryFile>
#include <memory>
#include "app/plaintext.h"

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
    } else if (options.range == WriteOutputOptions::Range::All) {
        for (int i = 0; i < count; ++i) out << i;
    } else {
        out = write::parsePageRange(options.pages, count);
    }
    // (the pages whose numbers are odd or even: the page counted from 1 is the page number the person means)
    if (options.sides != WriteOutputOptions::Sides::All) {
        const bool odd = options.sides == WriteOutputOptions::Sides::Odd;
        out.erase(std::remove_if(out.begin(), out.end(), [odd](int index) { return ((index + 1) % 2 == 1) != odd; }), out.end());
    }
    if (options.reverse) std::reverse(out.begin(), out.end());
    return out;
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
        jp::plainWarning(this, tr("Export to PDF"), tr("%1 couldn't export \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    statusBar()->showMessage(tr("Exported %1").arg(QFileInfo(path).fileName()), 5000);
    return true;
}

bool WriteWindow::exportText(const QString &path, write::TextEncoding encoding, write::LineEnds ends)
{
    if (!m_doc || path.isEmpty()) return false;
    int lost = 0;
    const QByteArray bytes = write::plainTextFile(*m_doc, encoding, ends, &lost);
    QSaveFile file(path);
    QString err;
    bool ok = file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size() && file.commit();
    if (!ok) err = file.errorString();
    if (!ok) {
        jp::plainWarning(this, tr("Save as Plain Text"), tr("%1 couldn't export \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    if (lost > 0) notice(tr("Save as Plain Text"), tr("Some characters couldn't be saved in this encoding (%1 in all). They were saved as question marks.").arg(lost), true);
    statusBar()->showMessage(tr("Exported %1").arg(QFileInfo(path).fileName()), 5000);
    return true;
}

bool WriteWindow::exportTextAsk()
{
    if (!m_doc) return false;
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("textExportDialog"));
    dialog.setWindowTitle(tr("Save as Plain Text"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *intro = new QLabel(tr("The file will hold the words of the document and nothing else: no fonts, pictures, or tables' borders. Choose how the characters and line ends are written."), &dialog);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *form = new QFormLayout;
    auto *encoding = new QComboBox(&dialog);
    encoding->setObjectName(QStringLiteral("textEncoding"));
    encoding->addItem(tr("Unicode (UTF-8)"), int(write::TextEncoding::Utf8));
    encoding->addItem(tr("Unicode (UTF-8 with mark)"), int(write::TextEncoding::Utf8WithMark));
    encoding->addItem(tr("Unicode (UTF-16)"), int(write::TextEncoding::Utf16));
    encoding->addItem(tr("Western European (ISO 8859-1)"), int(write::TextEncoding::Latin1));
    encoding->addItem(tr("US ASCII"), int(write::TextEncoding::Ascii));
    form->addRow(tr("Encoding:"), encoding);
    auto *ends = new QComboBox(&dialog);
    ends->setObjectName(QStringLiteral("textLineEnds"));
    ends->addItem(tr("Windows (CR LF)"), int(write::LineEnds::Windows));
    ends->addItem(tr("macOS and Linux (LF)"), int(write::LineEnds::Unix));
#ifndef Q_OS_WIN
    ends->setCurrentIndex(1);
#endif
    form->addRow(tr("Line ends:"), ends);
    layout->addLayout(form);
    auto *note = new QLabel(tr("A line break comes at the end of each paragraph. A character the encoding can't hold is saved as a question mark."), &dialog);
    note->setWordWrap(true);
    layout->addWidget(note);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return false;
    const QString dir = m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(m_path).absolutePath();
    const QString filter = tr("Text (*.txt)");
    const QString suggested = dir + QLatin1Char('/') + displayName() + QStringLiteral(".txt");
    QString path = m_pathChooser ? m_pathChooser(suggested, filter) : askSavePath(this, tr("Save as Plain Text"), suggested, filter);
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().isEmpty()) path += QStringLiteral(".txt");
    return exportText(path, static_cast<write::TextEncoding>(encoding->currentData().toInt()), static_cast<write::LineEnds>(ends->currentData().toInt()));
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
            jp::plainWarning(this, tr("Save as PDF"), tr("%1 couldn't make \"%2\".\n%3").arg(appInfo().name, QFileInfo(path).fileName(), made->errorString()));
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
        if (made && !err.isEmpty()) jp::plainWarning(this, tr("Save as PDF"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        else jp::plainWarning(this, tr("Print"), tr("%1 couldn't send \"%2\" to the printer.").arg(appInfo().name, displayName()));
        return false;
    }
    statusBar()->showMessage(options.toFile ? tr("Saved %1").arg(QFileInfo(path).fileName()) : tr("Sent %1 to the printer").arg(displayName()), 5000);
    return true;
}

} // namespace jp
