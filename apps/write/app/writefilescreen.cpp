#include "app/writefilescreen.h"

#include "app/filescreenparts.h"
#include "app/icons.h"
#include "app/pagepaint.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/printing.h"

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
#include <QPrinterInfo>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStyleHints>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>
#include <memory>

namespace jp {

namespace {

// A way to start a document: its first page over its name, a rounded highlight under the pointer, and an accent edge with the keyboard.
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

// The page preview, which says when its size changed so the page is drawn again to fit.
class PreviewLabel : public QLabel {
public:
    using QLabel::QLabel;
    std::function<void()> resized;

protected:
    void resizeEvent(QResizeEvent *e) override
    {
        QLabel::resizeEvent(e);
        if (resized) resized();
    }
};

} // namespace

WriteFileScreen::WriteFileScreen(WriteWindow *win) : FileScreen(win), m_win(win)
{
    setBackButton(tr("Back to document"), tr("Return to your document (Esc)"));
    setPageBuilder([this](const QString &key) { return buildPage(key); });
    addSection(tr("Start"));
    addPage(QStringLiteral("new"), QStringLiteral("file-plus"), tr("New"));
    addPage(QStringLiteral("open"), QStringLiteral("folder-open"), tr("Open"));
    addSection(tr("This document"));
    addCommand(QStringLiteral("save"), QStringLiteral("save"), tr("Save"), [this] { if (m_win->save()) Q_EMIT closeRequested(); });
    addCommand(QStringLiteral("saveas"), QStringLiteral("copy"), tr("Save As"), [this] { if (m_win->saveAs()) Q_EMIT closeRequested(); });
    addPage(QStringLiteral("print"), QStringLiteral("printer"), tr("Print"));
    addPage(QStringLiteral("export"), QStringLiteral("file-output"), tr("Export"));
    addCommand(QStringLiteral("close"), QStringLiteral("circle-x"), tr("Close"), [this] { m_win->act("file.close")->trigger(); Q_EMIT closeRequested(); });
    addSection(appInfo().name);
    addCommand(QStringLiteral("options"), QStringLiteral("settings"), tr("Options"), [this] { m_win->act("file.options")->trigger(); });
    addPage(QStringLiteral("about"), QStringLiteral("info"), tr("About"));
}

// The pages are built when they are shown, so they show what is now.
QWidget *WriteFileScreen::buildPage(const QString &key)
{
    if (key == "new") return buildNew();
    if (key == "open") return buildOpen();
    if (key == "print") return buildPrint();
    if (key == "export") return buildExport();
    if (key == "about") return buildAbout();
    return nullptr;
}

// ---------------- New ----------------
void WriteFileScreen::startBlank()
{
    // A new document nobody has touched gives way; any other stays, and the new one opens in a window of its own (as Open does).
    if (m_win->isModified() || !m_win->filePath().isEmpty()) m_win->act("file.new")->trigger();
    else m_win->setDocument(write::WriteDoc::blank(), QString());
    Q_EMIT closeRequested();
}

QWidget *WriteFileScreen::buildNew()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("New Document"), w));
    // The tile shows the blank document's first page.
    const qreal dpr = devicePixelRatioF();
    const QSize box(200, 260);
    auto blank = write::WriteDoc::blank();
    QImage first(QSize(qRound((box.width() - 16) * dpr), qRound((box.width() - 16) * dpr * 11 / 8.5)), QImage::Format_RGB32);
    first.fill(Qt::white);
    first.setDevicePixelRatio(dpr);
    if (const auto page = blank->pager().materialize(0)) {
        QPainter p(&first);
        p.scale((box.width() - 16) / page->size.width(), (box.width() - 16) / page->size.width());
        LayoutCache cache;
        write::paintPage(&p, *page, &cache, write::PageDrawing());
    }
    auto *tile = new StartTile(tr("Blank Document"), paperPixmap(first, box, dpr), w);
    connect(tile, &QAbstractButton::clicked, this, [this] { startBlank(); });
    auto *row = new QHBoxLayout();
    row->addWidget(tile);
    row->addStretch(1);
    v->addLayout(row);
    v->addStretch(1);
    return w;
}

// ---------------- Open ----------------
QWidget *WriteFileScreen::buildOpen()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("Open"), w));
    auto *cards = new QHBoxLayout();
    cards->setSpacing(12);
    auto *browse = new ActionCard(QStringLiteral("folder-open"), tr("Browse"), tr("Open a document from your computer."), w);
    connect(browse, &QAbstractButton::clicked, this, [this] { m_win->act("file.open")->trigger(); });
    auto *recover = new ActionCard(QStringLiteral("life-buoy"), tr("Recover Unsaved Work"), tr("Open copies of unsaved work %1 kept when it closed unexpectedly.").arg(appInfo().name), w);
    connect(recover, &QAbstractButton::clicked, this, [this] { m_win->offerRecovery(true); });
    cards->addWidget(browse, 1);
    cards->addWidget(recover, 1);
    v->addLayout(cards);
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
            auto *it = new QListWidgetItem(icon(QStringLiteral("file-text")), QStringLiteral("%1\n%2").arg(fi.fileName(), fi.absolutePath()));
            it->setData(Qt::UserRole, f);
            if (!fi.exists()) it->setForeground(QColor(150, 150, 150));
            list->addItem(it);
        }
        if (list->count() == 0) {
            delete list;
            auto *empty = mutedLabel(tr("Documents you open or save will appear here."), w, 0.6);
            empty->setContentsMargins(2, 4, 0, 0);
            v->addWidget(empty);
            return;
        }
        // As tall as its rows, so a short list leaves no empty space; the page scrolls.
        list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list->setFixedHeight(list->sizeHintForRow(0) * list->count() + 2 * list->frameWidth() + 12);
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
            m.exec(list->viewport()->mapToGlobal(pos));
        });
        v->addWidget(list);
    };
    if (!pinned.isEmpty()) {
        section(tr("Pinned"));
        addList(pinned, true);
    }
    section(tr("Recent Documents"));
    addList(Settings::get().recentFiles(), false);
    v->addStretch(1);
    return w;
}

// ---------------- Print ----------------
// What to print on the left, and the pages that will come out on the right, one at a time.
QWidget *WriteFileScreen::buildPrint()
{
    write::PageView *view = m_win->view();
    view->paginateAll();
    const int pageCount = view->pageCount();
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
    copies->setAccessibleName(tr("Copies"));
    auto *top = new QHBoxLayout();
    top->addWidget(printBtn);
    top->addSpacing(12);
    top->addWidget(new QLabel(tr("Copies:"), w));
    top->addWidget(copies);
    top->addStretch(1);
    left->addLayout(top);
    left->addSpacing(8);
    // The printer (a PDF file is one more), whether the copies come out a set at a time, and the paper the document's sections set.
    auto *form = new QFormLayout();
    auto *printer = new QComboBox(w);
    printer->setObjectName(QStringLiteral("printPrinter"));
    printer->setAccessibleName(tr("Printer"));
    for (const QPrinterInfo &info : QPrinterInfo::availablePrinters()) printer->addItem(info.printerName(), info.printerName());
    printer->addItem(tr("Save as PDF"), QStringLiteral("pdf"));
    if (!QPrinterInfo::defaultPrinter().isNull()) printer->setCurrentIndex(std::max(0, printer->findData(QPrinterInfo::defaultPrinter().printerName())));
    form->addRow(tr("Printer:"), printer);
    auto *collate = new QCheckBox(tr("Collated"), w);
    collate->setObjectName(QStringLiteral("printCollate"));
    collate->setChecked(true);
    collate->setToolTip(tr("Print each copy of the whole document in turn (1, 2, 3, 1, 2, 3), not each page's copies together (1, 1, 2, 2, 3, 3)."));
    form->addRow(QString(), collate);
    auto *paper = new QLabel(w);
    paper->setObjectName(QStringLiteral("printPaper"));
    paper->setWordWrap(true);
    form->addRow(tr("Paper:"), paper);
    left->addLayout(form);
    left->addSpacing(8);
    auto *all = new QRadioButton(tr("All pages"), w);
    all->setObjectName(QStringLiteral("printRangeAll"));
    auto *current = new QRadioButton(tr("Current page"), w);
    current->setObjectName(QStringLiteral("printRangeCurrent"));
    auto *some = new QRadioButton(tr("Pages:"), w);
    some->setObjectName(QStringLiteral("printRangePages"));
    auto *text = new QLineEdit(w);
    text->setObjectName(QStringLiteral("printRangeText"));
    text->setAccessibleName(tr("Pages to print"));
    text->setPlaceholderText(tr("1-3, 5"));
    text->setToolTip(tr("Page numbers and ranges, separated by commas."));
    auto *group = new QButtonGroup(w);
    for (QRadioButton *r : {all, current, some}) {
        group->addButton(r);
        left->addWidget(r);
    }
    left->addWidget(text);
    left->addStretch(1);
    h->addLayout(left, 1);

    auto *right = new QVBoxLayout();
    auto *preview = new PreviewLabel(w);
    preview->setObjectName(QStringLiteral("printPreview"));
    preview->setMinimumSize(300, 300);
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
    right->addWidget(preview, 1);
    right->addLayout(nav);
    h->addLayout(right, 2);
    all->setChecked(true);

    auto options = [=]() {
        WriteOutputOptions o;
        if (current->isChecked()) o.range = WriteOutputOptions::Range::Current;
        else if (some->isChecked()) {
            o.range = WriteOutputOptions::Range::Pages;
            o.pages = text->text();
        }
        o.toFile = printer->currentData().toString() == QLatin1String("pdf");
        if (!o.toFile) o.printer = printer->currentText();
        o.copies = copies->value();
        o.collate = collate->isChecked();
        return o;
    };
    auto page = std::make_shared<int>(0);
    // The words, the arrows, and the Print button for what is chosen; the pages it covers.
    auto covered = [=]() {
        const QVector<int> pages = m_win->outputPages(options());
        text->setEnabled(some->isChecked());
        const bool bad = some->isChecked() && pages.isEmpty();
        text->setStyleSheet(bad && !text->text().isEmpty() ? QStringLiteral("QLineEdit { border: 1px solid #c42b1c; }") : QString());
        *page = std::clamp(*page, 0, std::max(0, int(pages.size()) - 1));
        pageLabel->setText(bad ? tr("Enter page numbers, such as 1-3, 5.") : pages.isEmpty() ? tr("There are no pages to print.") : tr("Page %1 of %2").arg(pages[*page] + 1).arg(pageCount));
        previous->setEnabled(*page > 0);
        next->setEnabled(*page + 1 < pages.size());
        printBtn->setEnabled(!pages.isEmpty());
        collate->setEnabled(copies->value() > 1);
        // The paper of the page shown; where the sections of the document set others for the pages to print, that is said.
        if (pages.isEmpty()) {
            paper->clear();
        } else {
            const QSizeF shown = m_win->document()->pager().page(pages[*page]).size;
            bool differ = false;
            for (int index : pages) {
                const QSizeF size = m_win->document()->pager().page(index).size;
                differ |= std::abs(size.width() - shown.width()) > 1 || std::abs(size.height() - shown.height()) > 1;
            }
            paper->setText(write::paperDescription(shown) + (differ ? tr(" (the document's sections set other paper for some pages)") : QString()));
        }
        return pages;
    };
    auto paint = [=]() {
        const QVector<int> pages = covered();
        if (pages.isEmpty()) {
            preview->clear();
            return;
        }
        const qreal dpr = preview->devicePixelRatioF();
        const int room = std::max(preview->minimumHeight(), w->window()->height() - 200);
        const QSizeF paper = m_win->document()->pager().page(pages[*page]).size;
        const double scale = std::min((preview->width() - 16) / paper.width(), (room - 18) / paper.height());
        if (scale <= 0) return;
        const int widthPx = std::max(1, int(std::lround(paper.width() * scale * dpr)));
        QImage img = view->pageImage(pages[*page], widthPx);
        img.setDevicePixelRatio(dpr);
        preview->setPixmap(paperPixmap(img, QSize(int(std::ceil(paper.width() * scale)) + 16, int(std::ceil(paper.height() * scale)) + 18), dpr));
    };
    auto restart = [=]() {
        *page = 0;
        paint();
    };
    connect(group, &QButtonGroup::buttonToggled, w, [=] { restart(); });
    connect(text, &QLineEdit::textChanged, w, [=] {
        // Typing a range is choosing one.
        if (!some->isChecked()) some->setChecked(true);
        restart();
    });
    connect(copies, &QSpinBox::valueChanged, w, [=] { covered(); });
    connect(previous, &QToolButton::clicked, w, [=] { --*page; paint(); });
    connect(next, &QToolButton::clicked, w, [=] { ++*page; paint(); });
    connect(printBtn, &QPushButton::clicked, this, [=] {
        if (m_win->printOutput(options())) Q_EMIT closeRequested();
    });
    covered();
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
QWidget *WriteFileScreen::buildExport()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("Export"), w));
    v->setSpacing(10);
    auto *b = new ActionCard(QStringLiteral("file-output"), tr("Export to PDF"), tr("The pages as a PDF that looks the same everywhere."), w);
    connect(b, &QAbstractButton::clicked, this, [this] {
        if (m_win->exportPdf()) Q_EMIT closeRequested();
    });
    v->addWidget(b);
    v->addStretch(1);
    return w;
}

// ---------------- About ----------------
QWidget *WriteFileScreen::buildAbout()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->setSpacing(14);
    v->addWidget(heading(tr("About"), w));
    v->addWidget(suiteCard(w, this));

    // Identity card: badge, name, version, the one-line description, and where to find it.
    auto *id = cardFrame(w);
    id->setMaximumWidth(820);
    auto *ih = new QHBoxLayout(id);
    ih->setContentsMargins(20, 18, 20, 18);
    ih->setSpacing(16);
    auto *badge = new QLabel(id);
    badge->setPixmap(appBadge(this, 56, 24, 14));
    ih->addWidget(badge, 0, Qt::AlignTop);
    auto *iv = new QVBoxLayout();
    iv->setSpacing(3);
    auto *name = new QLabel(appInfo().name, id);
    QFont nf = name->font();
    nf.setWeight(QFont::DemiBold);
    nf.setPointSizeF(nf.pointSizeF() * 1.35);
    name->setFont(nf);
    iv->addWidget(name);
    iv->addWidget(mutedLabel(tr("Version %1 · Built with Qt %2").arg(QStringLiteral(JP_VERSION), QString::fromLatin1(qVersion())), id));
    auto *tag = new QLabel(tr("Free, open-source documents for Windows, macOS, and Linux. This is a Preview."), id);
    tag->setWordWrap(true);
    fitHeight(tag);
    iv->addWidget(tag);
    const QString repo = QStringLiteral("https://github.com/") + appInfo().repository;
    auto *links = new QLabel(QStringLiteral("<a href=\"%1\">%2</a> &nbsp;·&nbsp; <a href=\"%1/releases\">%3</a> &nbsp;·&nbsp; <a href=\"%1/issues\">%4</a>")
                                 .arg(repo, tr("Source code"), tr("Downloads"), tr("Report a problem")),
                             id);
    links->setOpenExternalLinks(true);
    iv->addWidget(links);
    ih->addLayout(iv, 1);
    v->addWidget(id);

    // License and legal notices.
    auto *legal = cardFrame(w);
    legal->setMaximumWidth(820);
    auto *lv = new QVBoxLayout(legal);
    lv->setContentsMargins(20, 16, 20, 16);
    lv->setSpacing(8);
    auto *lh = new QLabel(tr("License and notices"), legal);
    QFont lf = lh->font();
    lf.setWeight(QFont::DemiBold);
    lh->setFont(lf);
    lv->addWidget(lh);
    auto *lt = mutedLabel(tr("Copyright © 2026 JeffOffice LLC.<br><br>"
                             "%1 is free software under the "
                             "<a href=\"%2\">GNU General Public License v3.0</a>. "
                             "It comes with ABSOLUTELY NO WARRANTY; see the license for details. "
                             "<a href=\"%3\">Read the license on gnu.org</a>.<br><br>"
                             "%1 is an independent open-source project. It is not affiliated with, endorsed by or supported by any other "
                             "software company. Product names are trademarks of their owners.")
                              .arg(appInfo().name, repo + QStringLiteral("/blob/main/LICENSE"), QStringLiteral("https://www.gnu.org/licenses/gpl-3.0.html")),
                          legal, 0.75);
    lt->setWordWrap(true);
    fitHeight(lt);
    lt->setTextFormat(Qt::RichText);
    lt->setOpenExternalLinks(true);
    lv->addWidget(lt);
    v->addWidget(legal);
    v->addStretch(1);
    return w;
}

} // namespace jp
