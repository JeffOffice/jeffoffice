#include "app/deckfilescreen.h"
#include "core/version.h"

#include "app/deckcanvas.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/aboutpage.h"
#include "app/filescreenparts.h"
#include "app/filetasks.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/printpage.h"
#include "app/filedialogs.h"
#include "app/printsides.h"
#include "app/settings.h"
#include "canvas/canvas.h"
#include "core/appinfo.h"
#include "core/deck.h"
#include "io/deckoutput.h"
#include "io/presentationdefaults.h"
#include "render/renderer.h"

#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStyleHints>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>
#include <functional>
#include <memory>
#include "app/plaintext.h"

namespace jp {

namespace {

// A way to start a presentation: the slide's picture over its name, a rounded
// highlight under the pointer, and an accent edge when it has the keyboard.
class StartTile : public QAbstractButton {
public:
    StartTile(const QString &name, const QPixmap &picture, QWidget *parent) : QAbstractButton(parent), m_picture(picture)
    {
        setText(name);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover);
        setFocusPolicy(Qt::StrongFocus);
        const QSize pic = picture.deviceIndependentSize().toSize();
        setFixedSize(pic.width() + 24, pic.height() + 52);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
        const QColor accent = uiAccent();
        if (hasFocus()) {
            QColor bg = accent;
            bg.setAlphaF(dark() ? 0.22f : 0.08f);
            p.setPen(QPen(accent, 2));
            p.setBrush(bg);
            p.drawRoundedRect(r, 10, 10);
        } else if (underMouse()) {
            p.setPen(Qt::NoPen);
            p.setBrush(dark() ? QColor(255, 255, 255, 16) : QColor(0, 0, 0, 10));
            p.drawRoundedRect(r, 10, 10);
        }
        const QSizeF pic = m_picture.deviceIndependentSize();
        p.drawPixmap(QPointF((width() - pic.width()) / 2.0, 8), m_picture);
        QFont f = font();
        f.setWeight(QFont::DemiBold);
        p.setFont(f);
        p.setPen(uiText());
        p.drawText(QRectF(8, 8 + pic.height() + 4, width() - 16, height() - pic.height() - 20), Qt::AlignHCenter | Qt::AlignTop, text());
    }
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) click();
        else QAbstractButton::keyPressEvent(e);
    }
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::HoverEnter || e->type() == QEvent::HoverLeave) update();
        return QAbstractButton::event(e);
    }

private:
    QPixmap m_picture;
};

} // namespace

DeckFileScreen::DeckFileScreen(DeckWindow *win) : FileScreen(win), m_win(win)
{
    setBackButton(tr("Back to presentation"), tr("Return to your presentation (Esc)"));
    setPageBuilder([this](const QString &key) { return buildPage(key); });
    addSection(tr("Start"));
    addPage(QStringLiteral("new"), QStringLiteral("file-plus"), tr("New"));
    addPage(QStringLiteral("open"), QStringLiteral("folder-open"), tr("Open"));
    addSection(tr("This presentation"));
    addPage(QStringLiteral("info"), QStringLiteral("file-text"), tr("Info"));
    addCommand(QStringLiteral("save"), QStringLiteral("save"), tr("Save"), [this] { if (m_win->save()) Q_EMIT closeRequested(); });
    addCommand(QStringLiteral("saveas"), QStringLiteral("copy"), tr("Save As"), [this] { if (m_win->saveAs()) Q_EMIT closeRequested(); });
    addCommand(QStringLiteral("savecopy"), QStringLiteral("files"), tr("Save a Copy"), [this] { if (m_win->saveCopy()) Q_EMIT closeRequested(); });
    addCommand(QStringLiteral("savetemplate"), QStringLiteral("layout-template"), tr("Save as Template"), [this] { if (m_win->saveAsTemplate()) Q_EMIT closeRequested(); });
    addPage(QStringLiteral("print"), QStringLiteral("printer"), tr("Print"));
    addPage(QStringLiteral("export"), QStringLiteral("file-output"), tr("Export"));
    addCommand(QStringLiteral("close"), QStringLiteral("circle-x"), tr("Close"), [this] { m_win->act("file.close")->trigger(); Q_EMIT closeRequested(); });
    addSection(appInfo().name);
    addCommand(QStringLiteral("options"), QStringLiteral("settings"), tr("Options"), [this] { m_win->act("file.options")->trigger(); });
    addPage(QStringLiteral("about"), QStringLiteral("info"), tr("About"));
}

// The pages are built when they are shown, so they show what is now.
QWidget *DeckFileScreen::buildPage(const QString &key)
{
    if (key == "info") return m_win->buildInfoPage(nullptr);
    if (key == "new") return buildNew();
    if (key == "open") return buildOpen();
    if (key == "print") return buildPrint();
    if (key == "export") return buildExport();
    if (key == "about") return buildAbout();
    return nullptr;
}

// ---------------- New ----------------
void DeckFileScreen::startBlank()
{
    // A new presentation nobody has touched gives way; any other stays, and
    // the new one opens in a window of its own (as Open does).
    m_win->notesPane()->commit();
    Editor *ed = m_win->editor();
    ed->flushTyping();
    if (ed->isModified() || !ed->filePath().isEmpty()) {
        m_win->act("file.new")->trigger();
    } else {
        ed->endTextEdit();
        ed->setDocument(deck::newDefaultPresentation());
        m_win->canvas()->zoomToFit(Canvas::Fit::WholePage);
    }
    Q_EMIT closeRequested();
}

QWidget *DeckFileScreen::buildNew()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("New Presentation"), w));
    // The tile shows the blank presentation's first slide.
    const qreal dpr = devicePixelRatioF();
    const QSize box(336, 210);
    auto blank = deck::newDefaultPresentation();
    LayoutCache cache;
    QImage first = deck::slideImage(*blank, &cache, 0, qRound((box.width() - 16) * dpr));
    first.setDevicePixelRatio(dpr);
    auto *tile = new StartTile(tr("Blank Presentation"), paperPixmap(first, box, dpr), w);
    connect(tile, &QAbstractButton::clicked, this, [this] { startBlank(); });
    auto *row = new QHBoxLayout();
    row->addWidget(tile);
    row->addStretch(1);
    v->addLayout(row);
    // The templates a person saved: a presentation made from one is a new presentation (the template stays as it was).
    v->addWidget(myTemplatesPanel(w, {QStringLiteral("*.potx"), QStringLiteral("*.otp")}, QStringLiteral("file"), [this](const QString &path) {
        m_win->openFile(path);
        Q_EMIT closeRequested();
    }));
    v->addStretch(1);
    return w;
}

// ---------------- Open ----------------
QWidget *DeckFileScreen::buildOpen()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("Open"), w));
    auto *cards = new CardRow(12);
    auto *browse = new ActionCard(QStringLiteral("folder-open"), tr("Browse"), tr("Open a presentation from your computer."), w);
    connect(browse, &QAbstractButton::clicked, this, [this] { m_win->act("file.open")->trigger(); });
    auto *recover = new ActionCard(QStringLiteral("life-buoy"), tr("Recover Unsaved Work"),
                                   tr("Open copies of unsaved work %1 kept when it closed unexpectedly.").arg(appInfo().name), w);
    connect(recover, &QAbstractButton::clicked, this, [this] { m_win->offerRecovery(true); });
    cards->addWidget(browse);
    cards->addWidget(recover);
    v->addLayout(cards);
    v->addWidget(moreWaysToOpenCard(w, [this](OpenHow how) { m_win->browseAndOpen(how); }));
    v->addSpacing(14);
    auto section = [&](const QString &t) {
        QLabel *l = sectionLabel(t, w);
        l->setContentsMargins(2, 12, 0, 2);
        v->addWidget(l);
    };
    const QStringList pinned = Settings::get().pinnedFiles();
    auto addList = [&](const QStringList &files, bool isPinned) {
        auto *list = new QListWidget(w);
        list->setIconSize(QSize(32, 32));
        list->setContextMenuPolicy(Qt::CustomContextMenu);
        list->setMouseTracking(true);
        list->setMaximumWidth(820);
        list->setStyleSheet(QStringLiteral("QListWidget{background:%1; border:1px solid %2; border-radius:10px; padding:6px; outline:0;}"
                                           "QListWidget::item{padding:6px 8px; border-radius:7px; color:%4;}"
                                           "QListWidget::item:hover{background:%3;}"
                                           "QListWidget::item:selected{background:%3; color:%4;}")
                                .arg(uiCard().name(), uiLine().name(), uiCardPressed().name(), uiText().name()));
        for (const QString &f : files) {
            if (!isPinned && pinned.contains(f)) continue;
            const QFileInfo fi(f);
            auto *it = new QListWidgetItem(icon(QStringLiteral("presentation")), QStringLiteral("%1\n%2").arg(fi.fileName(), fi.absolutePath()));
            it->setData(Qt::UserRole, f);
            if (!fi.exists()) it->setForeground(QColor(150, 150, 150));
            list->addItem(it);
        }
        if (list->count() == 0) {
            delete list;
            auto *empty = mutedLabel(tr("Presentations you open or save will appear here."), w, 0.6);
            empty->setContentsMargins(2, 4, 0, 0);
            v->addWidget(empty);
            return;
        }
        // As tall as its rows, so a short list leaves no empty space; the page scrolls.
        list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list->setFixedHeight(list->sizeHintForRow(0) * list->count() + 2 * list->frameWidth() + 12);
        // Enter opens one, and so does a click: where a click activates an item already, that is the same step.
        auto open = [this](QListWidgetItem *it) { m_win->openFile(it->data(Qt::UserRole).toString()); };
        connect(list, &QListWidget::itemActivated, this, open);
        if (!QGuiApplication::styleHints()->singleClickActivation()) connect(list, &QListWidget::itemClicked, this, open);
        connect(list, &QListWidget::customContextMenuRequested, this, [this, list, isPinned](const QPoint &pos) {
            QListWidgetItem *it = list->itemAt(pos);
            if (!it) return;
            const QString f = it->data(Qt::UserRole).toString();
            QMenu m;
            m.addAction(isPinned ? tr("Unpin from list") : tr("Pin to list"), [this, f, isPinned] {
                Settings::get().setPinned(f, !isPinned);
                showPage(QStringLiteral("open"));
            });
            m.addAction(tr("Remove from list"), [this, f] {
                Settings::get().removeRecentFile(f);
                showPage(QStringLiteral("open"));
            });
            m.addSeparator();
            addOpenWays(&m, [this, f](OpenHow how) { m_win->openFile(f, how); });
            m.exec(list->viewport()->mapToGlobal(pos));
        });
        v->addWidget(list);
    };
    if (!pinned.isEmpty()) {
        section(tr("Pinned"));
        addList(pinned, true);
    }
    section(tr("Recent Presentations"));
    addList(Settings::get().recentFiles(), false);
    v->addStretch(1);
    return w;
}

// ---------------- Print ----------------
// What to print on the left, as the dialog before Export to PDF asks it, and
// the pages that will come out on the right, one at a time. What is chosen is
// kept for next time.
QWidget *DeckFileScreen::buildPrint()
{
    Editor *ed = m_win->editor();
    const int slides = int(ed->doc()->pages.size());
    const int current = ed->currentPage();
    auto *w = new QWidget();
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(40, 30, 40, 30);
    h->setSpacing(24);
    auto *left = new QVBoxLayout();
    left->addWidget(heading(tr("Print"), w));
    auto *printBtn = new QPushButton(tr("Print"), w);
    printBtn->setObjectName(QStringLiteral("printButton"));
    QSpinBox *copies = makeCopiesBox(w);
    PrintColumn column(w);
    column.top(printBtn, copies);

    QComboBox *printer = makePrinterBox(w);
    QPushButton *properties = makePrinterPropertiesButton(w);
    auto *printerRow = new QWidget(w);
    auto *printerLine = new QHBoxLayout(printerRow);
    printerLine->setContentsMargins(0, 0, 0, 0);
    printerLine->addWidget(printer, 1);
    printerLine->addWidget(properties);
    QCheckBox *collate = makeCollateBox(w, tr("Print each copy of all the pages in turn (1, 2, 3, 1, 2, 3), not each page's copies together (1, 1, 2, 2, 3, 3)."));
    auto *what = new QComboBox(w);
    what->setObjectName(QStringLiteral("printWhat"));
    what->setAccessibleName(DeckWindow::tr("Print what"));
    what->addItem(DeckWindow::tr("Slides"), int(deck::PrintLayout::Slides));
    what->addItem(DeckWindow::tr("Notes Pages"), int(deck::PrintLayout::NotesPages));
    what->addItem(DeckWindow::tr("Outline"), int(deck::PrintLayout::Outline));
    what->addItem(DeckWindow::tr("Handouts: 1 Slide Per Page"), int(deck::PrintLayout::Handout1));
    what->addItem(DeckWindow::tr("Handouts: 2 Slides Per Page"), int(deck::PrintLayout::Handout2));
    what->addItem(DeckWindow::tr("Handouts: 3 Slides Per Page"), int(deck::PrintLayout::Handout3));
    what->addItem(DeckWindow::tr("Handouts: 4 Slides Per Page"), int(deck::PrintLayout::Handout4));
    what->addItem(DeckWindow::tr("Handouts: 6 Slides Per Page"), int(deck::PrintLayout::Handout6));
    what->addItem(DeckWindow::tr("Handouts: 9 Slides Per Page"), int(deck::PrintLayout::Handout9));
    auto *order = new QComboBox(w);
    order->setObjectName(QStringLiteral("printOrder"));
    order->setAccessibleName(DeckWindow::tr("Order of slides on a handout"));
    order->addItem(DeckWindow::tr("Horizontal"), true);
    order->addItem(DeckWindow::tr("Vertical"), false);

    auto *all = new QRadioButton(DeckWindow::tr("All slides"), w);
    all->setObjectName(QStringLiteral("printRangeAll"));
    auto *currentOnly = new QRadioButton(DeckWindow::tr("Current slide"), w);
    currentOnly->setObjectName(QStringLiteral("printRangeCurrent"));
    auto *selection = new QRadioButton(tr("Selection"), w);
    selection->setObjectName(QStringLiteral("printRangeSelection"));
    selection->setToolTip(tr("Print only the slides selected in the slide sorter, or the slide you are on."));
    auto *some = new QRadioButton(DeckWindow::tr("Slides"), w);
    some->setObjectName(QStringLiteral("printRangeSlides"));
    auto *text = new QLineEdit(w);
    text->setObjectName(QStringLiteral("printRangeText"));
    text->setAccessibleName(DeckWindow::tr("Slides to print"));
    text->setPlaceholderText(DeckWindow::tr("1-3, 5"));
    text->setToolTip(DeckWindow::tr("Slide numbers and ranges, separated by commas."));
    auto *group = new QButtonGroup(w);
    for (QRadioButton *r : {all, currentOnly, selection, some}) group->addButton(r);
    auto *someRow = new QHBoxLayout();
    someRow->addWidget(some);
    someRow->addWidget(text);
    auto *someWidget = new QWidget(w);
    someRow->setContentsMargins(0, 0, 0, 0);
    someWidget->setLayout(someRow);

    // One side, or both sides flipped on the long or the short edge (the system's window starts with it), or both sides by hand. It was kept as "output/sides" once.
    Settings &sideSettings = Settings::get();
    const int wantedSides = sideSettings.value(QStringLiteral("print/sides"), sideSettings.value(QStringLiteral("output/sides"), int(SidesChoice::OneSide))).toInt();
    auto *sides = new QComboBox(w);
    sides->setObjectName(QStringLiteral("printDuplex"));
    fillSidesBox(sides, true, wantedSides);
    QComboBox *paper = makePaperBox(w, tr("As the slides say"));

    auto *hidden = new QCheckBox(tr("Print hidden slides"), w);
    hidden->setObjectName(QStringLiteral("printHidden"));
    auto *frame = new QCheckBox(tr("Frame slides"), w);
    frame->setObjectName(QStringLiteral("printFrame"));
    QComboBox *color = makeColorModeBox(w);
    auto *scaleFit = new QCheckBox(tr("Scale to Fit Paper"), w);
    scaleFit->setObjectName(QStringLiteral("printScaleToFit"));
    scaleFit->setToolTip(tr("Make each slide as large as the paper lets it be. Off, a slide that fits the paper is printed at its own size."));
    auto *headerFooter = new QPushButton(tr("Header && Footer…"), w);
    headerFooter->setObjectName(QStringLiteral("printHeaderFooter"));
    headerFooter->setToolTip(tr("The date, header, footer, and page number of notes pages and handouts."));
    QCheckBox *reverse = makeReverseBox(w);
    auto *comments = new QCheckBox(tr("Print comments"), w);
    comments->setObjectName(QStringLiteral("printComments"));
    comments->setToolTip(tr("A page of the slide's review comments after each slide that has any (slides and notes pages)."));
    auto *highQuality = new QCheckBox(tr("High quality"), w);
    highQuality->setObjectName(QStringLiteral("printHighQuality"));
    highQuality->setToolTip(tr("Grayscale and pure black and white pages are made at 600 to the inch, not 300 (300 for a very large page). It takes longer and more memory."));
    // (said when the pages are larger than the picture High quality makes: they are made at 300 to the inch)
    auto *highQualityNote = new QLabel(w);
    highQualityNote->setObjectName(QStringLiteral("printHighQualityNote"));
    highQualityNote->setWordWrap(true);
    highQualityNote->setTextFormat(Qt::PlainText);
    highQualityNote->hide();
    QCheckBox *openPdf = makeOpenPdfBox(w, QStringLiteral("printOpenPdf"));
    // the groups of the Print page (docs/design/STYLE.md)
    column.group(PrintColumn::printerHeading());
    column.wide(printerRow);
    column.checks(openPdf);
    column.group(PrintColumn::pagesHeading());
    column.pair(tr("Print what"), what, tr("Order"), order);
    for (QRadioButton *r : {all, currentOnly, selection}) column.wide(r);
    column.wide(someWidget);
    column.field(tr("Sides"), sides);
    column.checks(collate, reverse);
    column.checks(hidden);
    column.group(PrintColumn::layoutHeading());
    column.checks(scaleFit, frame);
    column.group(PrintColumn::paperHeading(), tr("(as each slide says unless changed)"));
    column.pair(tr("Paper"), paper, tr("Color"), color);
    column.checks(comments, highQuality);
    column.wide(highQualityNote);
    column.link(headerFooter);
    left->addLayout(column.layout());
    left->addStretch(1);
    h->addLayout(left, 1);

    // The pages, with arrows, and how large they are shown.
    auto *right = new QVBoxLayout();
    auto *preview = new PreviewLabel(w);
    preview->setObjectName(QStringLiteral("printPreview"));
    preview->setMinimumSize(280, 280);
    preview->setAlignment(Qt::AlignCenter);
    preview->setAccessibleName(tr("Preview of the printed page"));
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
    nav->addStretch(1);
    nav->addWidget(previous);
    nav->addWidget(pageLabel);
    nav->addWidget(next);
    nav->addStretch(1);
    QComboBox *zoom = makeZoomBox(w);
    auto *look = new QHBoxLayout();
    look->addStretch(1);
    look->addWidget(new QLabel(tr("Zoom"), w));
    look->addWidget(zoom);
    auto *scroll = new QScrollArea(w);
    scroll->setObjectName(QStringLiteral("printPreviewScroll"));
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setWidget(preview);
    right->addLayout(look);
    right->addWidget(scroll, 1);
    right->addLayout(nav);
    h->addLayout(right, 2);

    // What was chosen last time.
    Settings &st = Settings::get();
    what->setCurrentIndex(std::max(0, what->findData(st.value(QStringLiteral("output/layout"), int(deck::PrintLayout::Slides)).toInt())));
    order->setCurrentIndex(st.value(QStringLiteral("output/horizontal"), true).toBool() ? 0 : 1);
    hidden->setChecked(st.value(QStringLiteral("output/hidden"), false).toBool());
    frame->setChecked(st.value(QStringLiteral("output/frame"), false).toBool());
    scaleFit->setChecked(st.value(QStringLiteral("output/scaleToFit"), true).toBool());
    color->setCurrentIndex(std::max(0, color->findData(st.value(QStringLiteral("output/color"), st.value(QStringLiteral("output/grayscale"), false).toBool() ? int(PrintColor::Grayscale) : int(PrintColor::Color)).toInt())));
    all->setChecked(true);

    auto options = [=]() {
        deck::OutputOptions o;
        o.layout = deck::PrintLayout(what->currentData().toInt());
        o.hiddenSlides = hidden->isChecked();
        o.frameSlides = frame->isChecked();
        o.grayscale = color->currentData().toInt() != int(PrintColor::Color);
        o.blackAndWhite = color->currentData().toInt() == int(PrintColor::BlackAndWhite);
        o.scaleToFit = scaleFit->isChecked();
        o.horizontalOrder = order->currentData().toBool();
        if (currentOnly->isChecked()) o.slides = {current};
        else if (selection->isChecked()) o.slides = m_win->selectedSlides().toVector();
        else if (some->isChecked()) o.slides = deck::parseSlideRange(text->text(), slides);
        const PrinterChoice choice = printerChoice(printer);
        o.toFile = choice.toFile;
        o.printer = choice.printer;
        o.copies = copies->value();
        o.collate = collate->isChecked();
        o.paper = paperChoiceSize(paper);
        o.comments = comments->isChecked();
        o.highQuality = highQuality->isChecked();
        o.openAfter = openPdf->isChecked();
        o.reverse = reverse->isChecked() && !sidesByHand(sides);   // (both sides by hand sets the order itself)
        return o;
    };
    auto page = std::make_shared<int>(0);
    // The words, the arrows, and the Print button for what is chosen.
    auto refresh = [=]() {
        const deck::OutputOptions o = options();
        const int per = deck::slidesPerPage(o.layout);
        order->setEnabled(per >= 4);
        frame->setEnabled(o.layout == deck::PrintLayout::Slides);
        comments->setEnabled(o.layout == deck::PrintLayout::Slides || o.layout == deck::PrintLayout::NotesPages);
        const bool tooBig = highQuality->isChecked() && !deck::highQualityFits(deck::printSheetSize(*ed->doc(), o));
        highQualityNote->setText(tooBig ? tr("These pages are too large for High quality, so they are made at 300 to the inch.") : QString());
        highQualityNote->setVisible(tooBig);
        updatePrinterPropertiesButton(properties, printer);
        openPdf->setVisible(printerChoice(printer).toFile);
        scaleFit->setEnabled(o.layout == deck::PrintLayout::Slides);
        headerFooter->setEnabled(o.layout == deck::PrintLayout::NotesPages || deck::slidesPerPage(o.layout) > 0);
        text->setEnabled(some->isChecked());
        // A range that names no slide can't be printed.
        const bool badRange = some->isChecked() && o.slides.isEmpty();
        text->setStyleSheet(badRange && !text->text().isEmpty() ? QStringLiteral("QLineEdit { border: 1px solid #c42b1c; }") : QString());
        const int count = badRange ? 0 : deck::outputPageCount(*ed->doc(), o);
        *page = std::clamp(*page, 0, std::max(0, count - 1));
        // (the pages come out last first when asked, so the first one shown is the last page)
        const int shown = o.reverse ? count - 1 - *page : *page;
        pageLabel->setText(badRange ? DeckWindow::tr("Enter slide numbers, such as 1-3, 5.")
                           : count  ? tr("Page %1 of %2").arg(shown + 1).arg(count)
                                    : DeckWindow::tr("There are no slides to print."));
        previous->setEnabled(*page > 0);
        next->setEnabled(*page + 1 < count);
        printBtn->setEnabled(count > 0);
        collate->setEnabled(copies->value() > 1);
        // (a file has no sides; by hand, the order of the pages is set by the program)
        const PrinterChoice chosen = printerChoice(printer);
        updateSidesBox(sides, chosen.printer, chosen.toFile);
        reverse->setEnabled(!sidesByHand(sides));
        return count;
    };
    auto paint = [=]() {
        const deck::OutputOptions o = options();
        const int count = refresh();
        if (count == 0) {
            preview->clear();
            return;
        }
        // The page that fits the window, or larger (the preview scrolls then).
        const QSizeF sheet = deck::printSheetSize(*ed->doc(), o);
        const qreal dpr = preview->devicePixelRatioF();
        const double zoomBy = zoomFactor(zoom);
        // (what fits is worked out from the scroll area's window, which the picture never changes)
        const int availWidth = std::max(280, scroll->viewport()->width() - 4);
        const int availHeight = std::max(280, scroll->viewport()->height() - 4);
        const double scale = std::min((availWidth - 16) / sheet.width(), (availHeight - 18) / sheet.height()) * zoomBy;
        if (scale <= 0) return;
        const QSize box(int(std::ceil(sheet.width() * scale)) + 16, int(std::ceil(sheet.height() * scale)) + 18);
        // (no picture of more than kMaxPreviewSide pixels on a side: a large zoom of a large page is drawn from fewer, which the box then stretches)
        constexpr double kMaxPreviewSide = 4096;
        const double side = std::max(sheet.width(), sheet.height()) * scale * dpr;
        const double drawn = side > kMaxPreviewSide ? scale * kMaxPreviewSide / side : scale;
        QImage img((sheet * drawn * dpr).toSize().expandedTo(QSize(1, 1)), QImage::Format_ARGB32_Premultiplied);
        img.setDevicePixelRatio(dpr);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.scale(drawn, drawn);
        deck::paintOutputPage(&p, *ed->doc(), &ed->cache(), o, o.reverse ? count - 1 - *page : *page, sheet);
        p.end();
        preview->setPixmap(paperPixmap(img, box, dpr));
    };
    // A different kind of output or range starts again at its first page.
    auto restart = [=]() {
        *page = 0;
        paint();
    };
    auto remember = [=]() {
        const deck::OutputOptions o = options();
        Settings &s = Settings::get();
        s.setValue(QStringLiteral("output/layout"), int(o.layout));
        s.setValue(QStringLiteral("output/horizontal"), o.horizontalOrder);
        s.setValue(QStringLiteral("output/hidden"), o.hiddenSlides);
        s.setValue(QStringLiteral("output/frame"), o.frameSlides);
        s.setValue(QStringLiteral("output/grayscale"), o.grayscale);
        s.setValue(QStringLiteral("output/color"), color->currentData().toInt());
        s.setValue(QStringLiteral("output/scaleToFit"), o.scaleToFit);
    };
    connect(what, &QComboBox::currentIndexChanged, w, [=] { remember(); restart(); });
    connect(order, &QComboBox::currentIndexChanged, w, [=] { remember(); paint(); });
    connect(sides, &QComboBox::currentIndexChanged, w, [=] {
        Settings::get().setValue(QStringLiteral("print/sides"), sidesWanted(sides));
        restart();
    });
    connect(printer, &QComboBox::currentIndexChanged, w, [=] { restart(); });
    connect(paper, &QComboBox::currentIndexChanged, w, [=] { restart(); });
    // The system's print window, on the printer the job is printed on, so that what is chosen there (paper, tray, quality) is kept; what the page also has comes back to it.
    connect(properties, &QPushButton::clicked, this, [=] {
        PrinterControls controls;
        controls.printer = printer;
        controls.copies = copies;
        controls.collate = collate;
        controls.color = color;
        controls.paper = paper;
        showPrinterProperties(this, m_win->printer(), controls);
    });
    connect(comments, &QCheckBox::toggled, w, [=] { restart(); });
    connect(highQuality, &QCheckBox::toggled, w, [=] { paint(); });
    connect(copies, &QSpinBox::valueChanged, w, [=] { refresh(); });
    connect(collate, &QCheckBox::toggled, w, [=] { refresh(); });
    connect(reverse, &QCheckBox::toggled, w, [=] { restart(); });
    connect(zoom, &QComboBox::currentIndexChanged, w, [=] { paint(); });
    connect(group, &QButtonGroup::buttonToggled, w, [=] { restart(); });
    connect(text, &QLineEdit::textChanged, w, [=] {
        // Typing a range is choosing one.
        if (!some->isChecked()) some->setChecked(true);
        restart();
    });
    for (QCheckBox *c : {hidden, frame, scaleFit}) connect(c, &QCheckBox::toggled, w, [=] { remember(); paint(); });
    connect(color, &QComboBox::currentIndexChanged, w, [=] { remember(); paint(); });
    connect(headerFooter, &QPushButton::clicked, w, [=] {
        m_win->showPrintHeaderFooter();
        paint();
    });
    connect(previous, &QToolButton::clicked, w, [=] { --*page; paint(); });
    connect(next, &QToolButton::clicked, w, [=] { ++*page; paint(); });
    connect(printBtn, &QPushButton::clicked, this, [=] {
        if (m_win->printOutput(options(), sidesDuplex(sides), sidesByHand(sides))) Q_EMIT closeRequested();
    });
    refresh();
    // The preview is drawn once the page has its size, and again when that changes.
    auto *redraw = new QTimer(w);
    redraw->setSingleShot(true);
    redraw->setInterval(30);
    connect(redraw, &QTimer::timeout, w, paint);
    preview->resized = [redraw] { redraw->start(); };
    QTimer::singleShot(0, w, paint);
    return w;
}

// ---------------- Export ----------------
QWidget *DeckFileScreen::buildExport()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("Export"), w));
    v->setSpacing(10);
    auto row = [&](const QString &ic, const QString &title, const QString &text, const std::function<bool()> &fn) {
        auto *b = new ActionCard(ic, title, text, w);
        connect(b, &QAbstractButton::clicked, this, [this, fn] {
            if (fn()) Q_EMIT closeRequested();
        });
        v->addWidget(b);
    };
    row(QStringLiteral("file-output"), tr("Export to PDF"), tr("Slides, notes pages, handouts, or the outline, as a PDF that looks the same everywhere. Choose what to include first."),
        [this] { return m_win->exportPdf(); });
    row(QStringLiteral("image"), tr("Export Slide as Picture"), tr("The slide you're on, as a PNG or JPEG picture 1920 pixels across."), [this] { return m_win->exportPicture(); });
    auto *slides = new ActionCard(QStringLiteral("images"), tr("Save Slides as Pictures"), tr("Every slide as a PNG or JPEG picture in a folder, named Slide 1, Slide 2, and so on. You choose the width, and whether hidden slides go too."), w);
    slides->setObjectName(QStringLiteral("exportSlidePictures"));
    connect(slides, &QAbstractButton::clicked, this, [this] {
        if (m_win->exportSlidePicturesAsk()) Q_EMIT closeRequested();
    });
    v->addWidget(slides);
    auto *mail = new ActionCard(QStringLiteral("mail"), tr("Make an Email"), tr("A message file with the presentation, or a PDF of it, in it. Open it in your email program, add who it is for, and send it."), w);
    mail->setObjectName(QStringLiteral("exportEmail"));
    connect(mail, &QAbstractButton::clicked, this, [this] {
        if (m_win->exportEmailAsk()) Q_EMIT closeRequested();
    });
    v->addWidget(mail);
    // Pack and Go: the file with its fonts and linked files (libs/shell/app/packandgo.h).
    addPackAndGoCards(v, w, [this](bool toFolder) {
        if (m_win->packAndGo(toFolder)) Q_EMIT closeRequested();
    });
    v->addStretch(1);
    return w;
}

// ---------------- About ----------------
QWidget *DeckFileScreen::buildAbout()
{
    AboutHost host;
    host.tagline = tr("Free, open-source presentations for Windows, macOS, and Linux.");
    host.nameNote = tr("Saved as the author of a presentation that has none, as who last changed it, and as the name on your comments.");
    // FFmpeg, which plays the movies and sounds (third_party/ffmpeg).
    host.extras = {{QStringLiteral("FFmpeg"), QStringLiteral("https://ffmpeg.org/"), tr("Playing movies and sounds"), QStringLiteral("LGPL-2.1+"),
                    QStringLiteral("https://github.com/JeffOffice/jeffoffice/blob/main/third_party/ffmpeg/licenses/ffmpeg.txt")}};
    return buildAboutPage(nullptr, this, host);
}

} // namespace jp
