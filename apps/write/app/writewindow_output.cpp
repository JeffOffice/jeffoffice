// Printing and exporting from JeffWrite's window: Export to PDF, and the printing the File screen's Print page asks for.

#include "app/writewindow.h"

#include "app/filedialogs.h"
#include "app/pageoutput.h"
#include "app/pagepaint.h"
#include "core/editops.h"
#include "app/pageview.h"
#include "core/appinfo.h"
#include "core/emailops.h"
#include "core/pdfbookmarks.h"
#include "core/printing.h"
#include "office/officecrypt.h"
#include "office/packageprops.h"

#include <QApplication>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPainter>
#include <QPrinter>
#include <QSaveFile>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
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
    if (options.range == WriteOutputOptions::Range::Selection) {
        // The pages of a copy that holds the selection alone.
        m_selectionDoc = selectionDocument();
        if (!m_selectionDoc) return out;
        for (int i = 0, n = m_selectionDoc->pager().pageCount(); i < n; ++i) out << i;
    } else if (options.range == WriteOutputOptions::Range::Current) {
        out << std::clamp(m_view->currentPage(), 0, std::max(0, count - 1));
    } else if (options.range == WriteOutputOptions::Range::All) {
        for (int i = 0; i < count; ++i) out << i;
    } else {
        QVector<int> sections;
        for (int i = 0; i < count; ++i) sections << m_doc->pager().page(i).start.section;
        out = write::parsePageRange(options.pages, count, sections);
    }
    // (the pages whose numbers are odd or even: the page counted from 1 is the page number the person means)
    if (options.sides != WriteOutputOptions::Sides::All) {
        const bool odd = options.sides == WriteOutputOptions::Sides::Odd;
        out.erase(std::remove_if(out.begin(), out.end(), [odd](int index) { return ((index + 1) % 2 == 1) != odd; }), out.end());
    }
    if (options.reverse) std::reverse(out.begin(), out.end());
    return out;
}

std::unique_ptr<write::WriteDoc> WriteWindow::selectionDocument()
{
    if (!m_doc) return nullptr;
    const QTextCursor &c = m_view->cursor();
    if (!c.hasSelection() || c.document() != m_doc->story()) return nullptr;
    const int from = c.selectionStart(), to = c.selectionEnd();
    QString err;
    const QByteArray bytes = m_doc->toDocx(&err);
    if (bytes.isEmpty()) return nullptr;
    auto copy = write::WriteDoc::fromDocx(bytes, &err, nullptr);
    if (!copy) return nullptr;
    // What follows the selection goes first, so that the positions before it stay where they were.
    QTextDocument *story = copy->story();
    const int end = std::max(0, story->characterCount() - 1);
    if (to < end) {
        QTextCursor tail(story);
        tail.setPosition(std::min(to, end));
        tail.setPosition(end, QTextCursor::KeepAnchor);
        write::removeSelection(*copy, tail);
    }
    if (from > 0) {
        QTextCursor head(story);
        head.setPosition(0);
        head.setPosition(std::min(from, std::max(0, story->characterCount() - 1)), QTextCursor::KeepAnchor);
        write::removeSelection(*copy, head);
    }
    return copy;
}

QSizeF WriteWindow::outputPageSize(const WriteOutputOptions &options, int index)
{
    if (options.range == WriteOutputOptions::Range::Selection && m_selectionDoc) return m_selectionDoc->pager().page(index).size;
    return m_doc ? m_doc->pager().page(index).size : QSizeF();
}

QImage WriteWindow::outputPageImage(const WriteOutputOptions &options, int index, int width)
{
    if (options.range != WriteOutputOptions::Range::Selection) return m_view->pageImage(index, width);
    if (!m_selectionDoc) return {};
    const auto page = m_selectionDoc->pager().materialize(index);
    if (!page || page->size.width() <= 0) return {};
    QImage image(width, std::max(1, int(std::lround(width * page->size.height() / page->size.width()))), QImage::Format_RGB32);
    image.fill(Qt::white);
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(width / page->size.width(), width / page->size.width());
    LayoutCache cache;
    write::paintPage(&p, *page, &cache, write::PageDrawing());
    return image;
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

QByteArray WriteWindow::pdfBytes(const QVector<int> &pages, QString *error)
{
    if (!m_doc || pages.isEmpty()) {
        if (error) *error = tr("The pages can't be made.");
        return {};
    }
    EditedStoryHeld held(m_doc->pager(), m_view->inHeaderFooter() ? m_view->headerFooterEdit().story : nullptr);
    // The pages are made in memory: the bookmarks from the headings are added to the finished file.
    QBuffer made;
    made.open(QIODevice::WriteOnly);
    bool ok = false;
    {
        QPdfWriter pdf(&made);
        pdf.setCreator(appInfo().name);
        const office::PackageProps props = m_doc->packageProps();
        const QString title = props.title.isEmpty() ? displayName() : props.title;
        pdf.setTitle(title);
        pdf.setDocumentXmpMetadata(write::pdfPropertiesXmp(title, m_personalInfoRemoved ? QString() : props.creator, props.subject, props.keywords, appInfo().name));
        pdf.setResolution(1200);
        pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
        ok = write::paintSheets(*m_doc, &pdf, pages, m_view->pageCount());
    }
    if (!ok) {
        if (error) *error = tr("The pages can't be made.");
        return {};
    }
    return write::addPdfBookmarks(made.data(), write::headingBookmarks(*m_doc, pages));
}

bool WriteWindow::exportPdf()
{
    return exportPdfPages(WriteOutputOptions());
}

bool WriteWindow::exportPdfAsk()
{
    if (!m_doc) return false;
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("pdfDialog"));
    dialog.setWindowTitle(tr("Export to PDF"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *which = new QComboBox(&dialog);
    which->setObjectName(QStringLiteral("pdfRange"));
    which->addItem(tr("All pages"), int(WriteOutputOptions::Range::All));
    which->addItem(tr("Current page"), int(WriteOutputOptions::Range::Current));
    which->addItem(tr("Pages:"), int(WriteOutputOptions::Range::Pages));
    auto *text = new QLineEdit(&dialog);
    text->setObjectName(QStringLiteral("pdfPages"));
    text->setPlaceholderText(tr("1-3, 5, p2s3"));
    text->setEnabled(false);
    connect(which, &QComboBox::currentIndexChanged, &dialog, [which, text] { text->setEnabled(which->currentData().toInt() == int(WriteOutputOptions::Range::Pages)); });
    auto *form = new QFormLayout;
    form->addRow(tr("Pages to include:"), which);
    form->addRow(QString(), text);
    layout->addLayout(form);
    auto *note = new QLabel(tr("Type page numbers and ranges, separated by commas. p2s3 is the second page of section 3, and s3 is all of section 3."), &dialog);
    note->setWordWrap(true);
    layout->addWidget(note);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return false;
    WriteOutputOptions o;
    o.range = static_cast<WriteOutputOptions::Range>(which->currentData().toInt());
    o.pages = text->text();
    if (o.range == WriteOutputOptions::Range::Pages && outputPages(o).isEmpty()) {
        notice(tr("Export to PDF"), tr("None of those numbers is a page of the document. Type page numbers such as 1-3, 5."), true);
        return false;
    }
    return exportPdfPages(o);
}

bool WriteWindow::exportPdfPages(const WriteOutputOptions &options)
{
    if (!m_doc) return false;
    EditedStoryHeld held(m_doc->pager(), m_view->inHeaderFooter() ? m_view->headerFooterEdit().story : nullptr);
    const QVector<int> pages = outputPages(options);
    if (pages.isEmpty()) return false;
    const QString path = pdfPathToWrite(tr("Export to PDF"));
    if (path.isEmpty()) return false;
    QString err;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const QByteArray bytes = pdfBytes(pages, &err);
    bool ok = !bytes.isEmpty();
    if (ok) {
        QSaveFile file(path);
        ok = file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size() && file.commit();
        if (!ok) err = file.errorString();
    }
    QApplication::restoreOverrideCursor();
    if (!ok) {
        jp::plainWarning(this, tr("Export to PDF"), tr("%1 couldn't export \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    statusBar()->showMessage(tr("Exported %1").arg(QFileInfo(path).fileName()), 5000);
    return true;
}

bool WriteWindow::exportEmail(EmailWhat what)
{
    if (!m_doc) return false;
    constexpr int kMostPages = 30;   // (the pages of a message are pictures: a long document would make a message no mail program takes)
    const QString name = displayName();
    const office::PackageProps props = m_doc->packageProps();
    const QString subject = props.title.isEmpty() ? name : props.title;
    QString err;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QVector<write::EmailPart> attachments, pictures;
    QString text = tr("The document \"%1\" is attached.").arg(name), html;
    bool ok = true;
    const QVector<int> pages = outputPages(WriteOutputOptions());
    if (what == EmailWhat::Document) {
        QByteArray bytes = m_doc->toDocx(&err);
        // (a document with a password is sent with it)
        if (m_fileProtected && !bytes.isEmpty()) bytes = office::encryptPackage(bytes, m_filePassword);
        ok = !bytes.isEmpty();
        attachments.push_back({name + QStringLiteral(".docx"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document"), bytes, QString()});
    } else if (what == EmailWhat::Pdf) {
        const QByteArray bytes = pdfBytes(pages, &err);
        ok = !bytes.isEmpty();
        attachments.push_back({name + QStringLiteral(".pdf"), QStringLiteral("application/pdf"), bytes, QString()});
        text = tr("The document \"%1\" is attached as a PDF.").arg(name);
    } else {
        html = QStringLiteral("<html><body>");
        const int count = std::min<int>(pages.size(), kMostPages);
        for (int i = 0; i < count && ok; ++i) {
            const QImage image = m_view->pageImage(pages[i], 900);
            QByteArray png;
            QBuffer buffer(&png);
            buffer.open(QIODevice::WriteOnly);
            ok = !image.isNull() && image.save(&buffer, "PNG");
            const QString id = QStringLiteral("page%1").arg(i + 1);
            pictures.push_back({id + QStringLiteral(".png"), QStringLiteral("image/png"), png, id});
            html += QStringLiteral("<p><img src=\"cid:%1\" alt=\"%2\" style=\"max-width:100%\"></p>").arg(id, tr("Page %1").arg(i + 1));
        }
        html += QStringLiteral("</body></html>");
        text = tr("The pages of \"%1\" are shown as pictures in this message.").arg(name);
        if (pages.isEmpty()) ok = false;
        if (!ok && err.isEmpty()) err = tr("The pages can't be made.");
    }
    QApplication::restoreOverrideCursor();
    if (!ok) {
        jp::plainWarning(this, tr("Email"), tr("%1 couldn't make the message.\n%2").arg(appInfo().name, err));
        return false;
    }
    const QString dir = m_path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(m_path).absolutePath();
    const QString filter = tr("Email Message (*.eml)");
    const QString suggested = dir + QLatin1Char('/') + name + QStringLiteral(".eml");
    QString path = m_pathChooser ? m_pathChooser(suggested, filter) : askSavePath(this, tr("Save Email"), suggested, filter);
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().compare(QLatin1String("eml"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".eml");
    const QByteArray message = write::emailMessage(subject, text, attachments, html, pictures);
    QSaveFile file(path);
    if (!(file.open(QIODevice::WriteOnly) && file.write(message) == message.size() && file.commit())) {
        err = file.errorString();
        jp::plainWarning(this, tr("Email"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    if (what == EmailWhat::Pages && pages.size() > kMostPages) notice(tr("Email"), tr("Only the first %1 pages are in the message. Send the document or a PDF to share all of it.").arg(kMostPages));
    statusBar()->showMessage(tr("Saved %1. Open it in your email program, add who it is for, and send it.").arg(QFileInfo(path).fileName()), 8000);
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

bool WriteWindow::exportEmailAsk()
{
    if (!m_doc) return false;
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("emailDialog"));
    dialog.setWindowTitle(tr("Make an Email"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *intro = new QLabel(tr("This makes a message file. Nothing is sent. Open the file in your email program, add who it is for, and send it."), &dialog);
    intro->setWordWrap(true);
    layout->addWidget(intro);
    auto *what = new QComboBox(&dialog);
    what->setObjectName(QStringLiteral("emailWhat"));
    what->addItem(tr("The document, as a file"), int(EmailWhat::Document));
    what->addItem(tr("The document, as a PDF"), int(EmailWhat::Pdf));
    what->addItem(tr("The pages, shown in the message"), int(EmailWhat::Pages));
    auto *form = new QFormLayout;
    form->addRow(tr("Put in the message:"), what);
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return false;
    return exportEmail(static_cast<EmailWhat>(what->currentData().toInt()));
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

QVector<WriteOutputOptions> WriteWindow::handDuplexRuns(const WriteOutputOptions &options) const
{
    WriteOutputOptions odd = options, even = options;
    odd.byHand = even.byHand = false;
    odd.sides = WriteOutputOptions::Sides::Odd;
    odd.reverse = false;
    even.sides = WriteOutputOptions::Sides::Even;
    even.reverse = true;
    return {odd, even};
}

bool WriteWindow::printOutput(const WriteOutputOptions &options)
{
    if (!m_doc) return false;
    // Both sides by hand (a file has no sides): the odd pages, the stack turned over, then the even pages.
    if (options.byHand && !options.toFile) {
        const QVector<WriteOutputOptions> runs = handDuplexRuns(options);
        if (!printOutput(runs[0])) return false;
        if (outputPages(runs[1]).isEmpty()) return true;
        const QString ask = tr("The odd pages are printed. Turn the stack over and put it back in the printer, then choose OK to print the even pages.");
        bool go = true;
        if (m_stackTurner) go = m_stackTurner();
        else {
            QMessageBox box(QMessageBox::Information, tr("Print"), ask, QMessageBox::Ok | QMessageBox::Cancel, this);
            box.setTextFormat(Qt::PlainText);
            go = box.exec() == QMessageBox::Ok;
        }
        return go && printOutput(runs[1]);
    }
    EditedStoryHeld held(m_doc->pager(), m_view->inHeaderFooter() ? m_view->headerFooterEdit().story : nullptr);
    const QVector<int> pages = outputPages(options);
    if (pages.isEmpty()) return false;
    // (the selection prints from the copy that holds it alone)
    write::WriteDoc *source = options.range == WriteOutputOptions::Range::Selection ? m_selectionDoc.get() : m_doc.get();
    const int sourcePages = options.range == WriteOutputOptions::Range::Selection ? m_selectionDoc->pager().pageCount() : m_view->pageCount();
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
    bool ok = write::paintSheets(*source, &printer, sheets, sourcePages, printer.colorMode() == QPrinter::GrayScale);
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
