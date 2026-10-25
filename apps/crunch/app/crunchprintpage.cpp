// The Print and Export pages of JeffCrunch's File screen: what to print on the left, and the pages that will come out on the right, one at a time. Every choice makes the job again (the cut
// of the pages is quick), so the preview is always of what Print will send.

#include "app/crunchfilescreen.h"

#include "app/crunchwindow.h"
#include "app/filescreenparts.h"
#include "app/icons.h"
#include "core/printjob.h"
#include "sheet/pagesetup.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>
#include <memory>
#include <cmath>
#include <memory>

namespace jp {

namespace {

// What the page keeps while it is shown: the job (made again when a choice changes), the page in the preview, and the zoom of the preview (1 shows the whole page).
struct PrintState {
    std::shared_ptr<crunch::PrintJob> job;
    int page = 0;
    double zoom = 1;
    QString why;
};

// The preview, which says when its box changed size so the page is drawn again to fit.
class PreviewScroll : public QScrollArea {
public:
    using QScrollArea::QScrollArea;
    std::function<void()> resized;

protected:
    void resizeEvent(QResizeEvent *e) override
    {
        QScrollArea::resizeEvent(e);
        if (resized) resized();
    }
};

} // namespace

QWidget *CrunchFileScreen::buildPrint()
{
    CrunchWindow *win = m_win;
    crunch::PrintChoices &choices = win->printChoices();
    auto *w = new QWidget();
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(40, 30, 40, 30);
    h->setSpacing(24);
    auto *left = new QVBoxLayout();
    left->addWidget(heading(tr("Print"), w));
    auto *printBtn = new QPushButton(tr("Print"), w);
    printBtn->setObjectName(QStringLiteral("printButton"));
    printBtn->setProperty("primary", true);
    printBtn->setMinimumHeight(40);
    printBtn->setMinimumWidth(120);
    printBtn->setCursor(Qt::PointingHandCursor);
    auto *copies = new QSpinBox(w);
    copies->setObjectName(QStringLiteral("printCopies"));
    copies->setRange(1, 999);
    copies->setValue(std::max(1, choices.copies));
    copies->setAccessibleName(tr("Copies"));
    auto *top = new QHBoxLayout();
    top->addWidget(printBtn);
    top->addSpacing(12);
    top->addWidget(new QLabel(tr("Copies:"), w));
    top->addWidget(copies);
    top->addStretch(1);
    left->addLayout(top);
    left->addSpacing(8);
    auto *form = new QFormLayout();
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    // the printer (a PDF file is one more), with the system's own dialog for what a printer needs
    auto *printer = new QComboBox(w);
    printer->setObjectName(QStringLiteral("printPrinter"));
    printer->setAccessibleName(tr("Printer"));
    for (const QPrinterInfo &info : QPrinterInfo::availablePrinters()) printer->addItem(info.printerName(), info.printerName());
    printer->addItem(tr("Save as PDF"), QString());
    printer->setItemData(printer->count() - 1, true, Qt::UserRole + 1);
    const int wanted = choices.toFile ? printer->count() - 1 : printer->findData(choices.printer.isEmpty() ? QPrinterInfo::defaultPrinter().printerName() : choices.printer);
    printer->setCurrentIndex(wanted >= 0 ? wanted : printer->count() - 1);
    auto *settings = new QPushButton(tr("Printer settings…"), w);
    settings->setObjectName(QStringLiteral("printPrinterSettings"));
    auto *printerRow = new QHBoxLayout();
    printerRow->addWidget(printer, 1);
    printerRow->addWidget(settings);
    form->addRow(tr("Printer:"), printerRow);
    auto *what = new QComboBox(w);
    what->setObjectName(QStringLiteral("printWhat"));
    what->setAccessibleName(tr("What to print"));
    what->addItem(tr("This sheet"), int(crunch::PrintWhat::ActiveSheet));
    what->addItem(tr("The whole workbook"), int(crunch::PrintWhat::Workbook));
    what->addItem(tr("The selected cells"), int(crunch::PrintWhat::Selection));
    what->setCurrentIndex(std::max(0, what->findData(int(choices.job.what))));
    form->addRow(tr("Print:"), what);
    auto *range = new QLineEdit(choices.pageRange, w);
    range->setObjectName(QStringLiteral("printRange"));
    range->setAccessibleName(tr("Pages to print"));
    range->setPlaceholderText(tr("All, or such as 1-3, 5"));
    range->setToolTip(tr("Page numbers and ranges, separated by commas."));
    form->addRow(tr("Pages:"), range);
    auto *collate = new QCheckBox(tr("Collated"), w);
    collate->setObjectName(QStringLiteral("printCollate"));
    collate->setChecked(choices.collate);
    collate->setToolTip(tr("Print each copy of all the pages in turn (1, 2, 3, 1, 2, 3), not each page's copies together (1, 1, 2, 2, 3, 3)."));
    form->addRow(QString(), collate);
    auto *orientation = new QComboBox(w);
    orientation->setObjectName(QStringLiteral("printOrientation"));
    orientation->setAccessibleName(tr("Which way the paper turns"));
    orientation->addItem(tr("As each sheet says"), -1);
    orientation->addItem(tr("Tall"), int(jp::office::sheet::PageOrientation::Portrait));
    orientation->addItem(tr("Wide"), int(jp::office::sheet::PageOrientation::Landscape));
    orientation->setCurrentIndex(choices.job.setOrientation ? std::max(0, orientation->findData(int(choices.job.orientation))) : 0);
    form->addRow(tr("Paper turned:"), orientation);
    auto *paper = new QComboBox(w);
    paper->setObjectName(QStringLiteral("printPaper"));
    paper->setAccessibleName(tr("Paper size"));
    paper->addItem(tr("As each sheet says"), 0);
    for (const int id : jp::office::sheet::commonPaperSizes()) paper->addItem(jp::office::sheet::paperSizeName(id), id);
    paper->setCurrentIndex(choices.job.setPaper ? std::max(0, paper->findData(choices.job.paperSize)) : 0);
    form->addRow(tr("Paper:"), paper);
    auto *margins = new QComboBox(w);
    margins->setObjectName(QStringLiteral("printMargins"));
    margins->setAccessibleName(tr("Margins"));
    margins->addItem(tr("As each sheet says"), 0);
    margins->addItem(tr("Standard"), 1);
    margins->addItem(tr("Wide"), 2);
    margins->addItem(tr("Narrow"), 3);
    {
        const auto &m = choices.job.margins;
        int at = 0;
        if (choices.job.setMargins) at = m.left == 0.7 ? 1 : m.left == 1.0 ? 2 : 3;
        margins->setCurrentIndex(at);
    }
    form->addRow(tr("Margins:"), margins);
    auto *scaling = new QComboBox(w);
    scaling->setObjectName(QStringLiteral("printScaling"));
    scaling->setAccessibleName(tr("How big the cells print"));
    scaling->addItem(tr("As each sheet says"), int(crunch::PrintScaling::AsSheet));
    scaling->addItem(tr("Full size"), int(crunch::PrintScaling::NoScaling));
    scaling->addItem(tr("The whole sheet on one page"), int(crunch::PrintScaling::FitSheet));
    scaling->addItem(tr("All the columns on one page"), int(crunch::PrintScaling::FitColumns));
    scaling->addItem(tr("All the rows on one page"), int(crunch::PrintScaling::FitRows));
    scaling->setCurrentIndex(std::max(0, scaling->findData(int(choices.job.scaling))));
    form->addRow(tr("Size:"), scaling);
    auto *ignore = new QCheckBox(tr("Ignore the print area"), w);
    ignore->setObjectName(QStringLiteral("printIgnoreArea"));
    ignore->setChecked(choices.job.ignorePrintArea);
    ignore->setToolTip(tr("Print everything that has something in it, not just the print areas the sheets have."));
    form->addRow(QString(), ignore);
    left->addLayout(form);
    auto *setupButton = new QPushButton(tr("Page Setup…"), w);
    setupButton->setObjectName(QStringLiteral("printPageSetup"));
    setupButton->setToolTip(tr("Margins, headers and footers, print titles, and more for the sheet you are on."));
    left->addWidget(setupButton, 0, Qt::AlignLeft);
    auto *notes = new QLabel(w);
    notes->setObjectName(QStringLiteral("printNotes"));
    notes->setWordWrap(true);
    notes->setMinimumWidth(260);
    left->addWidget(notes);
    left->addStretch(1);
    h->addLayout(left, 1);

    auto *right = new QVBoxLayout();
    auto *scroll = new PreviewScroll(w);
    scroll->setObjectName(QStringLiteral("printPreviewScroll"));
    scroll->setWidgetResizable(false);
    scroll->setAlignment(Qt::AlignCenter);
    scroll->setMinimumSize(320, 320);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *preview = new QLabel();
    preview->setObjectName(QStringLiteral("printPreview"));
    preview->setAlignment(Qt::AlignCenter);
    preview->setAccessibleName(tr("Preview of the printed page"));
    scroll->setWidget(preview);
    auto *nav = new QHBoxLayout();
    auto *previous = new QToolButton(w);
    previous->setObjectName(QStringLiteral("printPrevious"));
    previous->setIcon(icon(QStringLiteral("chevron-left")));
    previous->setToolTip(tr("Previous page"));
    previous->setAccessibleName(previous->toolTip());
    auto *next = new QToolButton(w);
    next->setObjectName(QStringLiteral("printNext"));
    next->setIcon(icon(QStringLiteral("chevron-right")));
    next->setToolTip(tr("Next page"));
    next->setAccessibleName(next->toolTip());
    auto *pageLabel = new QLabel(w);
    pageLabel->setObjectName(QStringLiteral("printPage"));
    auto *zoomOut = new QToolButton(w);
    zoomOut->setObjectName(QStringLiteral("printZoomOut"));
    zoomOut->setIcon(icon(QStringLiteral("zoom-out")));
    zoomOut->setToolTip(tr("Zoom out"));
    zoomOut->setAccessibleName(zoomOut->toolTip());
    auto *zoomIn = new QToolButton(w);
    zoomIn->setObjectName(QStringLiteral("printZoomIn"));
    zoomIn->setIcon(icon(QStringLiteral("zoom-in")));
    zoomIn->setToolTip(tr("Zoom in"));
    zoomIn->setAccessibleName(zoomIn->toolTip());
    auto *fit = new QToolButton(w);
    fit->setObjectName(QStringLiteral("printFitPage"));
    fit->setIcon(icon(QStringLiteral("maximize")));
    fit->setToolTip(tr("Show the whole page"));
    fit->setAccessibleName(fit->toolTip());
    nav->addWidget(zoomOut);
    nav->addWidget(zoomIn);
    nav->addWidget(fit);
    nav->addStretch(1);
    nav->addWidget(previous);
    nav->addWidget(pageLabel);
    nav->addWidget(next);
    nav->addStretch(1);
    right->addWidget(scroll, 1);
    right->addLayout(nav);
    h->addLayout(right, 2);

    auto state = std::make_shared<PrintState>();
    // what the page's controls say, as the choices of the job
    auto read = [=]() {
        crunch::PrintChoices c;
        c.job.what = crunch::PrintWhat(what->currentData().toInt());
        c.job.ignorePrintArea = ignore->isChecked();
        if (orientation->currentData().toInt() >= 0) {
            c.job.setOrientation = true;
            c.job.orientation = jp::office::sheet::PageOrientation(orientation->currentData().toInt());
        }
        if (paper->currentData().toInt() > 0) {
            c.job.setPaper = true;
            c.job.paperSize = paper->currentData().toInt();
        }
        if (margins->currentData().toInt() > 0) {
            c.job.setMargins = true;
            c.job.margins = margins->currentData().toInt() == 1 ? jp::office::sheet::PageMargins{0.7, 0.7, 0.75, 0.75, 0.3, 0.3}
                            : margins->currentData().toInt() == 2 ? jp::office::sheet::PageMargins{1, 1, 1, 1, 0.5, 0.5}
                                                                  : jp::office::sheet::PageMargins{0.25, 0.25, 0.75, 0.75, 0.3, 0.3};
        }
        c.job.scaling = crunch::PrintScaling(scaling->currentData().toInt());
        c.pageRange = range->text();
        c.copies = copies->value();
        c.collate = collate->isChecked();
        c.toFile = printer->currentData(Qt::UserRole + 1).toBool();
        c.printer = c.toFile ? QString() : printer->currentData().toString();
        return c;
    };
    auto pagesToPrint = [=]() {
        QVector<int> pages;
        if (!state->job) return pages;
        if (range->text().trimmed().isEmpty()) {
            for (int i = 0; i < state->job->pageCount(); ++i) pages << i;
            return pages;
        }
        return crunch::parsePageRange(range->text(), state->job->pageCount());
    };
    // (pictures in the header are made on a worker: the page is drawn at once without them, and again when they are ready)
    auto paintSelf = std::make_shared<std::function<void()>>();
    auto paint = [=]() { (*paintSelf)(); };
    *paintSelf = [=]() {
        const QVector<int> pages = pagesToPrint();
        if (!state->job || pages.isEmpty()) {
            preview->clear();
            preview->resize(scroll->viewport()->size());
            return;
        }
        state->page = std::clamp(state->page, 0, int(pages.size()) - 1);
        const int index = pages[state->page];
        const QSizeF paperSize = state->job->pageSize(index);
        const qreal dpr = preview->devicePixelRatioF();
        const QSize room = scroll->viewport()->size() - QSize(24, 24);
        const double fitScale = std::min(double(std::max(80, room.width())) / paperSize.width(), double(std::max(80, room.height())) / paperSize.height());
        const double scale = fitScale * state->zoom;
        const int widthPx = std::max(1, int(std::lround(paperSize.width() * scale * dpr)));
        QImage img = state->job->preview(index, widthPx);
        img.setDevicePixelRatio(img.isNull() ? dpr : double(img.width()) / (paperSize.width() * scale));   // (fewer pixels than asked for, past the budget: the same size on the screen)
        const QSize box(int(std::ceil(paperSize.width() * scale)) + 16, int(std::ceil(paperSize.height() * scale)) + 18);
        preview->setPixmap(paperPixmap(img, box, dpr));
        preview->resize(box.expandedTo(scroll->viewport()->size()));
        if (state->job->picturesPending()) QTimer::singleShot(60, preview, [=] { (*paintSelf)(); });
    };
    auto words = [=]() {
        const QVector<int> pages = pagesToPrint();
        const bool bad = !range->text().trimmed().isEmpty() && pages.isEmpty() && state->job;
        range->setStyleSheet(bad ? QStringLiteral("QLineEdit { border: 1px solid #c42b1c; }") : QString());
        QStringList lines;
        if (!state->job) {
            pageLabel->setText(tr("Nothing to print"));
            lines << state->why;
        } else if (bad) {
            pageLabel->setText(tr("Enter page numbers, such as 1-3, 5."));
        } else {
            pageLabel->setText(tr("Page %1 of %2").arg(pages[state->page] + 1).arg(state->job->pageCount()));
            lines << state->job->messages();
            const int sheet = state->job->sheetOf(pages[state->page]);
            if (const jp::grid::SheetPrint *p = state->job->printOf(sheet)) {
                if (p->scalePercent < 50) lines << tr("The cells are printed at %1% of full size, so the letters are small.").arg(p->scalePercent);
            }
        }
        notes->setText(lines.join(QLatin1Char('\n')));
        previous->setEnabled(state->job && state->page > 0);
        next->setEnabled(state->job && state->page + 1 < pages.size());
        printBtn->setEnabled(state->job && !pages.isEmpty());
        collate->setEnabled(copies->value() > 1);
        zoomOut->setEnabled(state->job && state->zoom > 1.01);
    };
    // the job is made again when a choice changes: the pages, the numbers, and what is in the preview all follow
    auto rebuild = [=]() {
        crunch::PrintChoices c = read();
        QString why;
        state->job = win->makePrintJob(c.job, &why);
        if (state->job) {
            state->job->setAsyncPictures(true);
            state->job->prefetchPictures();
        }
        state->why = why;
        state->page = 0;
        words();
        paint();
    };
    auto choose = [=]() {
        win->printChoices() = read();
        rebuild();
    };
    for (QComboBox *box : {what, orientation, paper, margins, scaling}) connect(box, &QComboBox::currentIndexChanged, w, [=] { choose(); });
    connect(ignore, &QCheckBox::toggled, w, [=] { choose(); });
    connect(range, &QLineEdit::textChanged, w, [=] {
        win->printChoices() = read();
        state->page = 0;
        words();
        paint();
    });
    connect(copies, &QSpinBox::valueChanged, w, [=] {
        win->printChoices() = read();
        words();
    });
    connect(collate, &QCheckBox::toggled, w, [=] { win->printChoices() = read(); });
    connect(printer, &QComboBox::currentIndexChanged, w, [=] { win->printChoices() = read(); });
    connect(previous, &QToolButton::clicked, w, [=] {
        --state->page;
        words();
        paint();
    });
    connect(next, &QToolButton::clicked, w, [=] {
        ++state->page;
        words();
        paint();
    });
    connect(zoomIn, &QToolButton::clicked, w, [=] {
        state->zoom = std::min(6.0, state->zoom * 1.25);
        words();
        paint();
    });
    connect(zoomOut, &QToolButton::clicked, w, [=] {
        state->zoom = std::max(1.0, state->zoom / 1.25);
        words();
        paint();
    });
    connect(fit, &QToolButton::clicked, w, [=] {
        state->zoom = 1;
        words();
        paint();
    });
    connect(settings, &QPushButton::clicked, this, [=] {
        QPrinter &p = win->printer();
        const QString name = printer->currentData(Qt::UserRole + 1).toBool() ? QString() : printer->currentData().toString();
        if (!name.isEmpty()) p.setPrinterName(name);
        QPrintDialog dialog(&p, this);
        if (dialog.exec() == QDialog::Accepted) {
            const int at = printer->findData(p.printerName());
            if (at >= 0) printer->setCurrentIndex(at);
            copies->setValue(std::max(1, p.copyCount()));
            collate->setChecked(p.collateCopies());
        }
    });
    connect(setupButton, &QPushButton::clicked, this, [=] {
        win->pageSetupDialog(0);
        rebuild();
    });
    connect(printBtn, &QPushButton::clicked, this, [=] {
        if (win->printWith(read())) Q_EMIT closeRequested();
    });
    rebuild();
    // the preview is drawn once the box has its size, and again when that changes
    auto *redraw = new QTimer(w);
    redraw->setSingleShot(true);
    redraw->setInterval(30);
    connect(redraw, &QTimer::timeout, w, paint);
    scroll->resized = [redraw] { redraw->start(); };
    QTimer::singleShot(0, w, paint);
    win->armPageBreaks();
    return w;
}

QWidget *CrunchFileScreen::buildExport()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("Export"), w));
    v->setSpacing(10);
    auto *sheet = new ActionCard(QStringLiteral("file-output"), tr("Export this sheet to PDF"), tr("The pages of the sheet you are on, as a PDF that looks the same everywhere."), w);
    sheet->setObjectName(QStringLiteral("exportSheetPdf"));
    connect(sheet, &QAbstractButton::clicked, this, [this] {
        if (m_win->exportPdf(false)) Q_EMIT closeRequested();
    });
    v->addWidget(sheet);
    auto *book = new ActionCard(QStringLiteral("file-output"), tr("Export the whole workbook to PDF"), tr("Every sheet that is shown, one after the other, in one PDF."), w);
    book->setObjectName(QStringLiteral("exportBookPdf"));
    connect(book, &QAbstractButton::clicked, this, [this] {
        if (m_win->exportPdf(true)) Q_EMIT closeRequested();
    });
    v->addWidget(book);
    v->addStretch(1);
    return w;
}

} // namespace jp
