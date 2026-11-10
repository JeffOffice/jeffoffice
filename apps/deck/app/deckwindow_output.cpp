// Printing and exporting from JeffDeck's window: Export to PDF, which asks
// first what to print, Export Slide as Picture, and the printing the File
// screen's Print page asks for (it chooses what to print, and previews it).

#include "app/deckwindow.h"
#include "app/printpage.h"
#include "app/printsides.h"

#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/notespane.h"
#include "app/pdfprotection.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "io/deckoutput.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrinter>
#include <QPushButton>
#include <QRadioButton>
#include <QScopeGuard>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QStatusBar>
#include <QVBoxLayout>
#include <functional>
#include <memory>
#include "app/plaintext.h"

namespace jp {

namespace {

using PathChooser = std::function<QString(const QString &suggested, const QString &filter)>;

// The dialog before Export to PDF: what to print, the slides, and the options
// that go with them.
class OutputDialog : public QDialog {
public:
    // `current` is the slide being edited (0-based).
    OutputDialog(QWidget *parent, const Document &doc, int current) : QDialog(parent), m_doc(doc), m_slides(int(doc.pages.size())), m_current(current)
    {
        setObjectName(QStringLiteral("outputDialog"));
        setWindowTitle(DeckWindow::tr("Export to PDF"));
        auto *form = new QFormLayout;
        m_what = new QComboBox(this);
        m_what->setObjectName(QStringLiteral("outputWhat"));
        m_what->setAccessibleName(DeckWindow::tr("Print what"));
        m_what->addItem(DeckWindow::tr("Slides"), int(deck::PrintLayout::Slides));
        m_what->addItem(DeckWindow::tr("Notes Pages"), int(deck::PrintLayout::NotesPages));
        m_what->addItem(DeckWindow::tr("Outline"), int(deck::PrintLayout::Outline));
        m_what->addItem(DeckWindow::tr("Handouts: 1 Slide Per Page"), int(deck::PrintLayout::Handout1));
        m_what->addItem(DeckWindow::tr("Handouts: 2 Slides Per Page"), int(deck::PrintLayout::Handout2));
        m_what->addItem(DeckWindow::tr("Handouts: 3 Slides Per Page"), int(deck::PrintLayout::Handout3));
        m_what->addItem(DeckWindow::tr("Handouts: 4 Slides Per Page"), int(deck::PrintLayout::Handout4));
        m_what->addItem(DeckWindow::tr("Handouts: 6 Slides Per Page"), int(deck::PrintLayout::Handout6));
        m_what->addItem(DeckWindow::tr("Handouts: 9 Slides Per Page"), int(deck::PrintLayout::Handout9));
        form->addRow(DeckWindow::tr("Print &what:"), m_what);
        m_order = new QComboBox(this);
        m_order->setObjectName(QStringLiteral("outputOrder"));
        m_order->setAccessibleName(DeckWindow::tr("Order of slides on a handout"));
        m_order->addItem(DeckWindow::tr("Horizontal"), true);
        m_order->addItem(DeckWindow::tr("Vertical"), false);
        form->addRow(DeckWindow::tr("&Order:"), m_order);

        auto *range = new QVBoxLayout;
        m_all = new QRadioButton(DeckWindow::tr("All slides"), this);
        m_all->setObjectName(QStringLiteral("outputRangeAll"));
        m_currentOnly = new QRadioButton(DeckWindow::tr("Current slide"), this);
        m_currentOnly->setObjectName(QStringLiteral("outputRangeCurrent"));
        m_some = new QRadioButton(DeckWindow::tr("Slides:"), this);
        m_some->setObjectName(QStringLiteral("outputRangeSlides"));
        m_text = new QLineEdit(this);
        m_text->setObjectName(QStringLiteral("outputRangeText"));
        m_text->setAccessibleName(DeckWindow::tr("Slides to print"));
        m_text->setPlaceholderText(DeckWindow::tr("1-3, 5"));
        m_text->setToolTip(DeckWindow::tr("Slide numbers and ranges, separated by commas."));
        auto *group = new QButtonGroup(this);
        for (QRadioButton *r : {m_all, m_currentOnly, m_some}) group->addButton(r);
        auto *some = new QHBoxLayout;
        some->addWidget(m_some);
        some->addWidget(m_text);
        range->addWidget(m_all);
        range->addWidget(m_currentOnly);
        range->addLayout(some);
        form->addRow(DeckWindow::tr("Range:"), range);

        m_hidden = new QCheckBox(DeckWindow::tr("Print &hidden slides"), this);
        m_hidden->setObjectName(QStringLiteral("outputHidden"));
        m_frame = new QCheckBox(DeckWindow::tr("&Frame slides"), this);
        m_frame->setObjectName(QStringLiteral("outputFrame"));
        m_gray = new QCheckBox(DeckWindow::tr("&Grayscale"), this);
        m_gray->setObjectName(QStringLiteral("outputGray"));
        form->addRow(QString(), m_hidden);
        form->addRow(QString(), m_frame);
        form->addRow(QString(), m_gray);
        m_archive = new QCheckBox(DeckWindow::tr("PDF/A, for keeping for a long time"), this);
        m_archive->setObjectName(QStringLiteral("pdfArchive"));
        m_archive->setToolTip(DeckWindow::tr("Makes a PDF/A-1b file: every font is in the file, so it looks the same in years. It is not tagged for screen readers."));
        form->addRow(QString(), m_archive);
        m_protect = new jp::PdfProtectionRow(this);
        m_protect->forbiddenBy({m_archive}, DeckWindow::tr("PDF/A is turned off, because a PDF with a password can't be a PDF/A file."));
        form->addRow(QString(), m_protect);
        m_summary = new QLabel(this);
        m_summary->setObjectName(QStringLiteral("outputSummary"));
        form->addRow(QString(), m_summary);

        auto *layout = new QVBoxLayout(this);
        layout->addLayout(form);
        m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        m_ok = m_buttons->button(QDialogButtonBox::Ok);
        m_ok->setText(DeckWindow::tr("Export…"));
        layout->addWidget(m_buttons);
        m_ok->setDefault(true);
        connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        // What was chosen last time.
        Settings &st = Settings::get();
        m_what->setCurrentIndex(std::max(0, m_what->findData(st.value(QStringLiteral("output/layout"), int(deck::PrintLayout::Slides)).toInt())));
        m_order->setCurrentIndex(st.value(QStringLiteral("output/horizontal"), true).toBool() ? 0 : 1);
        m_hidden->setChecked(st.value(QStringLiteral("output/hidden"), false).toBool());
        m_frame->setChecked(st.value(QStringLiteral("output/frame"), false).toBool());
        m_gray->setChecked(st.value(QStringLiteral("output/grayscale"), false).toBool());
        m_all->setChecked(true);

        auto update = [this] { refresh(); };
        connect(m_what, &QComboBox::currentIndexChanged, this, update);
        connect(m_order, &QComboBox::currentIndexChanged, this, update);
        connect(group, &QButtonGroup::buttonToggled, this, update);
        connect(m_text, &QLineEdit::textChanged, this, [this] {
            // Typing a range is choosing one.
            if (!m_some->isChecked()) m_some->setChecked(true);
            refresh();
        });
        for (QCheckBox *c : {m_hidden, m_frame, m_gray}) connect(c, &QCheckBox::toggled, this, update);
        refresh();
    }

    deck::OutputOptions options() const
    {
        deck::OutputOptions o;
        o.layout = deck::PrintLayout(m_what->currentData().toInt());
        o.hiddenSlides = m_hidden->isChecked();
        o.frameSlides = m_frame->isChecked();
        o.grayscale = m_gray->isChecked();
        o.horizontalOrder = m_order->currentData().toBool();
        o.archive = m_archive->isChecked();
        if (m_currentOnly->isChecked()) o.slides = {m_current};
        else if (m_some->isChecked()) o.slides = deck::parseSlideRange(m_text->text(), m_slides);
        return o;
    }
    // The password to open the PDF with, if one was set here; the caller wipes it. It is never kept.
    PdfProtection protection() const { return m_protect->protection(); }
    // Keeps the choices for next time.
    void remember() const
    {
        Settings &st = Settings::get();
        const deck::OutputOptions o = options();
        st.setValue(QStringLiteral("output/layout"), int(o.layout));
        st.setValue(QStringLiteral("output/horizontal"), o.horizontalOrder);
        st.setValue(QStringLiteral("output/hidden"), o.hiddenSlides);
        st.setValue(QStringLiteral("output/frame"), o.frameSlides);
        st.setValue(QStringLiteral("output/grayscale"), o.grayscale);
    }

private:
    void refresh()
    {
        const deck::OutputOptions o = options();
        const int per = deck::slidesPerPage(o.layout);
        m_order->setEnabled(per >= 4);
        m_frame->setEnabled(o.layout == deck::PrintLayout::Slides);
        m_text->setEnabled(m_some->isChecked());
        // A range that names no slide can't be printed.
        const bool badRange = m_some->isChecked() && o.slides.isEmpty();
        m_text->setStyleSheet(badRange && !m_text->text().isEmpty() ? QStringLiteral("QLineEdit { border: 1px solid #c42b1c; }") : QString());
        const int pages = deck::outputPageCount(m_doc, o);
        m_summary->setText(badRange ? DeckWindow::tr("Enter slide numbers, such as 1-3, 5.") : pages ? DeckWindow::tr("%n page(s)", nullptr, pages) : DeckWindow::tr("There are no slides to print."));
        m_ok->setEnabled(pages > 0 && !badRange);
    }

    const Document &m_doc;
    int m_slides, m_current;
    QComboBox *m_what = nullptr, *m_order = nullptr;
    QRadioButton *m_all = nullptr, *m_currentOnly = nullptr, *m_some = nullptr;
    QLineEdit *m_text = nullptr;
    QCheckBox *m_hidden = nullptr, *m_frame = nullptr, *m_gray = nullptr, *m_archive = nullptr;
    QLabel *m_summary = nullptr;
    jp::PdfProtectionRow *m_protect = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
    QPushButton *m_ok = nullptr;
};

// What the presentation holds when typing is done: the notes and the text being edited are in the slides.
void settle(DeckWindow *w)
{
    w->notesPane()->commit();
    w->editor()->endTextEdit();
}

// Asks what to print; false when cancelled.
bool askWhatToPrint(DeckWindow *w, deck::OutputOptions *options, PdfProtection *protect)
{
    OutputDialog dlg(w, *w->editor()->doc(), w->editor()->currentPage());
    if (dlg.exec() != QDialog::Accepted) return false;
    dlg.remember();
    *options = dlg.options();
    *protect = dlg.protection();
    return true;
}

// Where a file named for the presentation goes by default: beside the
// presentation's file, else in the documents folder.
QString suggestedPath(DeckWindow *w, const QString &name, const QString &ending)
{
    const QString file = w->editor()->filePath();
    const QString dir = file.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) : QFileInfo(file).absolutePath();
    return dir + QLatin1Char('/') + name + ending;
}

bool exportPdfCommand(DeckWindow *w, const PathChooser &chooser)
{
    settle(w);
    deck::OutputOptions options;
    PdfProtection protect;
    const auto wipePasswords = qScopeGuard([&protect] { protect.wipe(); });
    if (!askWhatToPrint(w, &options, &protect)) return false;
    const QString filter = DeckWindow::tr("PDF (*.pdf)");
    const QString suggested = suggestedPath(w, w->editor()->displayName(), QStringLiteral(".pdf"));
    QString path = chooser ? chooser(suggested, filter) : askSavePath(w, DeckWindow::tr("Export to PDF"), suggested, filter);
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".pdf");
    QString err;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const bool ok = deck::exportPdf(*w->editor()->doc(), path, options, &err, &protect);
    QApplication::restoreOverrideCursor();
    if (!ok) {
        jp::plainWarning(w, DeckWindow::tr("Export to PDF"),
                             DeckWindow::tr("%1 couldn't export \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    w->statusBar()->showMessage(DeckWindow::tr("Exported %1").arg(QFileInfo(path).fileName()), 5000);
    return true;
}

bool exportPictureCommand(DeckWindow *w, const PathChooser &chooser)
{
    settle(w);
    Editor *ed = w->editor();
    const int slide = ed->currentPage();
    // The dialog hands back the filter text it showed, so the choice is recognized by comparing with these same (translated) texts.
    const QString pngFilter = DeckWindow::tr("PNG Picture (*.png)"), jpegFilter = DeckWindow::tr("JPEG Picture (*.jpg *.jpeg)");
    QString selected = pngFilter;
    const QString suggested = suggestedPath(w, DeckWindow::tr("%1 - Slide %2").arg(ed->displayName()).arg(slide + 1), QStringLiteral(".png"));
    QString path = chooser ? chooser(suggested, selected)
                           : askSavePath(w, DeckWindow::tr("Export Slide as Picture"), suggested, pngFilter + QStringLiteral(";;") + jpegFilter, &selected);
    if (path.isEmpty()) return false;
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix != QLatin1String("png") && suffix != QLatin1String("jpg") && suffix != QLatin1String("jpeg"))
        path += selected == jpegFilter ? QStringLiteral(".jpg") : QStringLiteral(".png");
    const QImage img = deck::slideImage(*ed->doc(), &ed->cache(), slide);
    const bool jpeg = QFileInfo(path).suffix().toLower() != QLatin1String("png");
    // A picture is written whole or not at all: the old file stays if this one fails.
    QSaveFile file(path);
    bool ok = !img.isNull() && file.open(QIODevice::WriteOnly) && img.save(&file, jpeg ? "JPEG" : "PNG", jpeg ? 92 : -1) && file.commit();
    if (!ok) {
        const QString err = file.error() != QFileDevice::NoError ? file.errorString() : DeckWindow::tr("The picture can't be made.");
        jp::plainWarning(w, DeckWindow::tr("Export Slide as Picture"),
                             DeckWindow::tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
        return false;
    }
    w->statusBar()->showMessage(DeckWindow::tr("Exported %1").arg(QFileInfo(path).fileName()), 5000);
    return true;
}

// Sets the printer up for the output: the paper chosen, else the locale's paper upright, or, for printed slides, the sheet turned the way the slides are shaped.
void setUpPrinter(QPrinter *printer, const Document &doc, const deck::OutputOptions &options)
{
    const QSizeF slide = doc.pageSize();
    const bool wide = options.layout == deck::PrintLayout::Slides && slide.width() > slide.height();
    printer->setPageSize(QPageSize(options.paper.isEmpty() ? deck::localePaper() : options.paper, QPageSize::Point, QString(), QPageSize::FuzzyMatch));
    printer->setPageOrientation(wide ? QPageLayout::Landscape : QPageLayout::Portrait);
    printer->setColorMode(options.grayscale ? QPrinter::GrayScale : QPrinter::Color);
}

// Paints the output on a printer (the real one, or the preview's), a page on
// each sheet, inside the margins it can print within. `only` is the pages to
// paint (-1 is a blank sheet), else all of them, last sheet first when asked
// (a front stays with its back where `duplex` says both sides are used).
// `repeat` sends the pages again for the copies, in the order they come out:
// for a printer that can't make copies itself. False, with `why`, when the
// pages could not be sent.
bool paintOnPrinter(QPrinter *printer, const Document &doc, LayoutCache *cache, const deck::OutputOptions &optionsIn, const QVector<int> *only, bool repeat, bool duplex, QString *why)
{
    deck::OutputOptions options = optionsIn;
    if (printer->colorMode() == QPrinter::GrayScale) options.grayscale = true;
    const QSizeF sheet = printer->pageRect(QPrinter::Point).size();
    QVector<int> pages;
    if (only) {
        pages = *only;
    } else {
        for (int i = 0, n = deck::outputPageCount(doc, options, sheet); i < n; ++i) pages << i;
    }
    const int sent = repeat ? std::max(1, options.copies) : 1;
    if (tooManyToSend(pages.size(), sent)) {
        if (why) *why = DeckWindow::tr("That is more pages than one job can send. Print fewer slides or fewer copies.");
        return false;
    }
    pages = sheetSequence(pages, -1, !only && options.reverse, duplex && !only, sent, options.collate);
    QPainter p;
    if (!p.begin(printer)) return false;
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    const double scale = printer->resolution() / 72.0;
    bool first = true;
    for (int i : std::as_const(pages)) {
        if (!first) printer->newPage();
        first = false;
        if (i < 0) continue;   // (a blank sheet)
        p.save();
        p.scale(scale, scale);
        deck::paintOutputPage(&p, doc, cache, options, i, sheet);
        p.restore();
    }
    p.end();
    return true;
}

} // namespace

QPrinter &DeckWindow::printer()
{
    if (!m_printer) m_printer = std::make_unique<QPrinter>(QPrinter::HighResolution);
    return *m_printer;
}

bool DeckWindow::printOutput(const deck::OutputOptions &options, QPrinter::DuplexMode duplex, bool byHand)
{
    settle(this);
    const Document &doc = *m_ed->doc();
    if (!options.toFile && !options.printer.isEmpty() && !m_printerPreparer && !printerExists(options.printer)) {
        printNotice(this, tr("Print"), tr("The printer \"%1\" is not there any more. Choose another printer.").arg(options.printer));
        return false;
    }
    QPrinter &printer = this->printer();
    printer.setDocName(m_ed->displayName());
    // A PDF file is made in a temporary file and put in place when it is whole, so that a file that is there is not lost to one that fails.
    QString path;
    std::unique_ptr<QTemporaryFile> made;
    if (options.toFile) {
        const QString suggested = suggestedPath(this, m_ed->displayName(), QStringLiteral(".pdf"));
        path = m_pathChooser ? m_pathChooser(suggested, tr("PDF (*.pdf)")) : askSavePath(this, tr("Save as PDF"), suggested, tr("PDF (*.pdf)"));
        if (path.isEmpty()) return false;
        if (QFileInfo(path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".pdf");
        made = std::make_unique<QTemporaryFile>(QDir::tempPath() + QStringLiteral("/jeffdeck-XXXXXX.pdf"));
        if (!made->open()) {
            jp::plainWarning(this, tr("Save as PDF"), tr("%1 couldn't make \"%2\".\n%3").arg(appInfo().name, QFileInfo(path).fileName(), made->errorString()));
            return false;
        }
        made->close();
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(made->fileName());
    } else {
        printer.setOutputFormat(QPrinter::NativeFormat);
        if (!options.printer.isEmpty() && printer.printerName() != options.printer) printer.setPrinterName(options.printer);
    }
    setUpPrinter(&printer, doc, options);
    // (a PDF file has no sides)
    setPrinterDuplex(&printer, options.toFile ? QPrinter::DuplexNone : duplex);
    printer.setCopyCount(std::max(1, options.copies));
    printer.setCollateCopies(options.collate);
    const bool byHandRuns = byHand && !options.toFile && !printsToFile(printer);
    if (m_printerPreparer) m_printerPreparer(printer);
    const QString target = options.toFile ? QFileInfo(path).fileName() : options.printer.isEmpty() ? printer.printerName() : options.printer;
    auto failed = [&](const QString &why) {
        QApplication::restoreOverrideCursor();
        printNotice(this, tr("Print"), why.isEmpty() ? (options.toFile ? tr("%1 couldn't make the PDF file \"%2\".").arg(appInfo().name, target) : tr("Couldn't print to %1.").arg(target)) : why);
        return false;
    };
    // A printer that can't make copies is sent the pages again, in the order the copies are to come out (a PDF file is made once).
    auto again = [&] { return !options.toFile && !printer.supportsMultipleCopies(); };
    const bool bothSides = !options.toFile && duplex != QPrinter::DuplexNone;
    QString why;
    // Both sides by hand (a file has no sides): the first, third, ... pages, the stack turned over, then the others last first.
    if (byHandRuns) {
        QVector<int> all;
        for (int i = 0, n = deck::outputPageCount(doc, options, printer.pageRect(QPrinter::Point).size()); i < n; ++i) all << i;
        const HandRuns<int> runs = handRuns(all, -1);   // (a front with no back leaves a blank sheet, -1)
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if (!paintOnPrinter(&printer, doc, &m_ed->cache(), options, &runs.first, again(), false, &why)) return failed(why);
        QApplication::restoreOverrideCursor();
        if (!runs.second.isEmpty()) {
            if (!askToTurnStack(this)) return false;
            if (m_printerPreparer) m_printerPreparer(printer);
            QApplication::setOverrideCursor(Qt::WaitCursor);
            if (!paintOnPrinter(&printer, doc, &m_ed->cache(), options, &runs.second, again(), false, &why)) return failed(why);
            QApplication::restoreOverrideCursor();
        }
        statusBar()->showMessage(tr("Sent %1 to the printer").arg(m_ed->displayName()), 5000);
        return true;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (!paintOnPrinter(&printer, doc, &m_ed->cache(), options, nullptr, again(), bothSides, &why)) return failed(why);
    QApplication::restoreOverrideCursor();
    if (made) {
        QString err;
        QFile whole(made->fileName());
        QSaveFile file(path);
        bool ok = whole.open(QIODevice::ReadOnly) && file.open(QIODevice::WriteOnly);
        while (ok && !whole.atEnd()) ok = file.write(whole.read(1 << 20)) >= 0;
        if (ok) ok = file.commit();
        else file.cancelWriting();
        if (!ok) {
            err = file.error() != QFileDevice::NoError ? file.errorString() : whole.errorString();
            jp::plainWarning(this, tr("Save as PDF"), tr("%1 couldn't save \"%2\".\n%3%4").arg(appInfo().name, QFileInfo(path).fileName(), err, saveFailureHint(err)));
            return false;
        }
        statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(path).fileName()), 5000);
        return true;
    }
    statusBar()->showMessage(tr("Sent %1 to the printer").arg(m_ed->displayName()), 5000);
    return true;
}

bool DeckWindow::exportPdf() { return exportPdfCommand(this, m_pathChooser); }

bool DeckWindow::exportPicture() { return exportPictureCommand(this, m_pathChooser); }

void DeckWindow::createOutputActions()
{
    mk("file.exportPdf", tr("Export to PDF…"), "file-output", QKeySequence(), [this] { exportPdf(); });
    mk("file.exportPicture", tr("Export Slide as Picture…"), "image", QKeySequence(), [this] { exportPicture(); });
    mk("file.print", tr("Print…"), "printer", QKeySequence::Print, [this] { showFileScreen(QStringLiteral("print")); });
}

} // namespace jp
