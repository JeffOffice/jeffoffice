#include "app/crunchwindow.h"

#include "app/crunchfilescreen.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/keytips.h"
#include "app/optionsdialog.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/sheettabs.h"
#include "app/telemetry.h"
#include "app/widgets.h"
#include "core/appinfo.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QRegularExpression>
#include <QSlider>
#include <QStandardPaths>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <cmath>

namespace jp {

namespace {
int s_untitledCount = 0;
}

CrunchWindow::CrunchWindow(QWidget *parent) : AppWindow(parent)
{
    setObjectName(QStringLiteral("crunchWindow"));
    setAcceptDrops(true);
    setWindowIcon(drawnIcon([](QPainter *p, const QRectF &r) {
        p->setPen(Qt::NoPen);
        p->setBrush(appInfo().accent);
        p->drawRoundedRect(r.adjusted(1, 1, -1, -1), r.width() * 0.24, r.width() * 0.24);
        QFont f;
        f.setPixelSize(int(r.height() * 0.42));
        f.setWeight(QFont::DemiBold);
        p->setFont(f);
        p->setPen(Qt::white);
        p->drawText(r, Qt::AlignCenter, appInfo().badge);
    }));
    ++s_untitledCount;
    m_emptyStyles.fonts = {jp::office::sheet::FontSpec()};
    m_emptyStyles.fills = {jp::office::sheet::FillSpec()};
    m_emptyStyles.borders = {jp::office::sheet::BorderSpec()};
    m_emptyStyles.formats = {jp::office::sheet::CellFormat()};
    m_emptyStyles.cellXfs = {0};
    m_ribbon = new Ribbon(this);
    setMenuWidget(m_ribbon);
    buildCentral();
    createActions();
    buildRibbon();
    buildStatusBar();
    describeForScreenReaders(m_view);
    describeForScreenReaders(statusBar());
    m_fileScreen = new CrunchFileScreen(this);
    m_fileScreen->hide();
    connect(m_fileScreen, &CrunchFileScreen::closeRequested, this, &CrunchWindow::hideFileScreen);
    new KeyTips(this, m_ribbon, [this] { showFileScreen(); });   // Alt, F
    connect(m_ribbon, &Ribbon::fileClicked, this, [this] { showFileScreen(); });
    connect(m_ribbon, &Ribbon::leaveRequested, this, [this] { m_view->setFocus(Qt::OtherFocusReason); });
    connect(m_view, &grid::GridView::selectionChanged, this, &CrunchWindow::updateBars);
    connect(m_view, &grid::GridView::statsChanged, this, [this](const QString &text) { m_stats->setText(text); });
    connect(m_view, &grid::GridView::zoomChanged, this, [this](double z) {
        m_zoomLabel->setText(QStringLiteral("%1%").arg(std::lround(z * 100)));
        const QSignalBlocker block(m_zoomSlider);
        m_zoomSlider->setValue(int(std::lround(std::log(z) / std::log(1.05))));
    });
    connect(m_view, &grid::GridView::contextMenuRequested, this, [this](const QPoint &globalPos) {
        // the commands of a right click: copying works now, and the others are dimmed until editing comes
        auto *menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->setObjectName(QStringLiteral("cellMenu"));
        for (const char *id : {"edit.cut", "edit.copy", "edit.paste"}) menu->addAction(act(QString::fromLatin1(id)));
        menu->addSeparator();
        menu->addAction(act("edit.selectAll"));
        menu->popup(globalPos);
    });
    connect(&m_loader, &crunch::WorkbookLoader::firstScreen, this, &CrunchWindow::onFirstScreen);
    connect(&m_loader, &crunch::WorkbookLoader::progress, this, &CrunchWindow::onProgress);
    connect(&m_loader, &crunch::WorkbookLoader::finished, this, &CrunchWindow::onFinished);
    m_calcTimer.setSingleShot(true);
    connect(&m_calcTimer, &QTimer::timeout, this, &CrunchWindow::calculationStep);
    m_copyTimer.setSingleShot(true);
    connect(&m_copyTimer, &QTimer::timeout, this, &CrunchWindow::copyStep);
    connect(m_tabs, &QTabBar::currentChanged, this, [this](int tab) {
        if (m_switchingTabs || tab < 0 || tab >= m_tabSheets.size()) return;
        setActiveSheet(m_tabSheets[tab]);
    });
    showBlank();
    resize(1280, 800);
    updateTitle();
    updateBars();
}

CrunchWindow::~CrunchWindow()
{
    m_stopCalc = true;
    m_loader.cancel();
}

QAction *CrunchWindow::mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn, bool checkable, bool enabled)
{
    auto *a = new QAction(text, this);
    a->setObjectName(id);
    if (!iconName.isEmpty()) a->setIcon(icon(iconName));
    if (!key.isEmpty()) {
        a->setShortcut(key);
        a->setShortcutContext(Qt::WindowShortcut);
        a->setToolTip(tr("%1 (%2)").arg(QString(text).remove('&'), key.toString(QKeySequence::NativeText)));
        addAction(a);
    }
    a->setCheckable(checkable);
    a->setEnabled(enabled);
    if (!enabled) a->setToolTip(tr("%1: not available in JeffCrunch yet").arg(QString(text).remove('&')));
    // Each command counts by its id for the usage statistics (when they're on).
    connect(a, &QAction::triggered, this, [fn, id] {
        telemetry::count(id);
        if (fn) fn();
    });
    m_actions.insert(id, a);
    return a;
}

QAction *CrunchWindow::act(const QString &id) const { return m_actions.value(id); }

QStringList CrunchWindow::actionIds() const
{
    QStringList ids = m_actions.keys();
    ids.sort();
    return ids;
}

// ---- the parts of the window

void CrunchWindow::buildCentral()
{
    auto *central = new QWidget(this);
    auto *v = new QVBoxLayout(central);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    // the name box and the formula bar
    m_formulaRow = new QWidget(central);
    auto *row = new QHBoxLayout(m_formulaRow);
    row->setContentsMargins(6, 4, 6, 4);
    row->setSpacing(6);
    m_nameBox = new QLineEdit(m_formulaRow);
    m_nameBox->setObjectName(QStringLiteral("nameBox"));
    m_nameBox->setFixedWidth(110);
    m_nameBox->setAccessibleName(tr("Name box"));
    m_nameBox->setAccessibleDescription(tr("The address of the selected cell. Type an address, such as C5 or A1:D10, and press Enter to go there."));
    auto *fx = new QLabel(QStringLiteral("fx"), m_formulaRow);
    fx->setObjectName(QStringLiteral("formulaLabel"));
    QFont fxFont = fx->font();
    fxFont.setItalic(true);
    fxFont.setWeight(QFont::DemiBold);
    fx->setFont(fxFont);
    m_formulaBar = new QLineEdit(m_formulaRow);
    m_formulaBar->setObjectName(QStringLiteral("formulaBar"));
    m_formulaBar->setReadOnly(true);
    m_formulaBar->setAccessibleName(tr("Formula bar"));
    m_formulaBar->setAccessibleDescription(tr("What the selected cell holds: its formula, or its value. It can't be changed yet."));
    row->addWidget(m_nameBox);
    row->addWidget(fx);
    row->addWidget(m_formulaBar, 1);
    v->addWidget(m_formulaRow);
    m_view = new grid::GridView(central);
    m_view->setAccessibleName(tr("Sheet"));
    m_view->setAccessibleDescription(tr("The cells of the sheet. The arrow keys move the selection; Ctrl and an arrow go to the edge of the data."));
    v->addWidget(m_view, 1);
    m_tabs = new crunch::SheetTabBar(central);
    v->addWidget(m_tabs);
    setCentralWidget(central);
    connect(m_nameBox, &QLineEdit::returnPressed, this, &CrunchWindow::goToName);
}

void CrunchWindow::buildStatusBar()
{
    QStatusBar *sb = statusBar();
    m_status = new QLabel(tr("Ready"), this);
    m_status->setObjectName(QStringLiteral("statusText"));
    m_status->setMinimumWidth(220);
    sb->addWidget(m_status);
    m_stats = new QLabel(this);
    m_stats->setObjectName(QStringLiteral("selectionStats"));
    sb->addPermanentWidget(m_stats);
    auto button = [&](const char *id) {
        auto *b = new QToolButton();
        b->setDefaultAction(act(QString::fromLatin1(id)));
        b->setAutoRaise(true);
        b->setIconSize(QSize(16, 16));
        b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        sb->addPermanentWidget(b);
        return b;
    };
    button("zoom.out");
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setObjectName(QStringLiteral("zoomSlider"));
    m_zoomSlider->setRange(-47, 28);   // 1.05^n: 10% .. 400%
    m_zoomSlider->setFixedWidth(140);
    m_zoomSlider->setAccessibleName(tr("Zoom"));
    m_zoomSlider->setAccessibleDescription(tr("Left and Right arrows zoom out and in."));
    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int v) { m_view->setZoom(std::pow(1.05, v)); });
    sb->addPermanentWidget(m_zoomSlider);
    button("zoom.in");
    m_zoomLabel = new QLabel(QStringLiteral("100%"));
    m_zoomLabel->setMinimumWidth(44);
    m_zoomLabel->setObjectName(QStringLiteral("zoomPercent"));
    sb->addPermanentWidget(m_zoomLabel);
}

void CrunchWindow::setStatus(const QString &text) { m_status->setText(text); }

QString CrunchWindow::statusText() const { return m_status->text(); }

// ---- the commands

void CrunchWindow::createActions()
{
    mk("file.open", tr("Open"), "folder-open", QKeySequence::Open, [this] { act("file.openDialog")->trigger(); });
    mk("file.openDialog", tr("Open Workbook…"), "folder-open", QKeySequence(), [this] {
        const QString p = QFileDialog::getOpenFileName(this, tr("Open Workbook"), Settings::get().value("dirs/open", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                       tr("Workbooks (*.xlsx *.xlsm)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (p.isEmpty()) return;
        Settings::get().setValue("dirs/open", QFileInfo(p).absolutePath());
        openFile(p);
    });
    mk("file.close", tr("Close"), "circle-x", QKeySequence::Close, [this] { close(); });
    mk("file.options", tr("Options"), "settings", QKeySequence(), [this] {
        // The shared pages (General, Proofing); JeffCrunch has no pages of its own yet.
        Editor stand;
        optionsDialog(this, &stand, {});
    });
    mk("edit.copy", tr("Copy"), "copy", QKeySequence::Copy, [this] { copySelection(); });
    mk("edit.selectAll", tr("Select All"), "", QKeySequence(), [this] { m_view->selectAll(); });
    // ---- view
    mk("view.gridlines", tr("Gridlines"), "grid-3x3", QKeySequence(), [this] { m_view->setShowGridlines(act("view.gridlines")->isChecked()); }, true)->setChecked(true);
    mk("view.headings", tr("Headings"), "table-properties", QKeySequence(), [this] { m_view->setShowHeadings(act("view.headings")->isChecked()); }, true)->setChecked(true);
    mk("view.formulaBar", tr("Formula Bar"), "text-cursor-input", QKeySequence(), [this] { m_formulaRow->setVisible(act("view.formulaBar")->isChecked()); }, true)->setChecked(true);
    mk("view.formulas", tr("Show Formulas"), "square-function", QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft), [this] {
        m_view->sheet().setShowFormulas(act("view.formulas")->isChecked());
        m_view->invalidateValues();
    }, true);
    mk("zoom.in", tr("Zoom In"), "zoom-in", QKeySequence(), [this] { m_view->setZoom(std::round(m_view->zoom() * 10 + 1) / 10); });
    mk("zoom.out", tr("Zoom Out"), "zoom-out", QKeySequence(), [this] { m_view->setZoom(std::round(m_view->zoom() * 10 - 1) / 10); });
    for (int pct : {200, 150, 100, 75, 50, 25, 10}) {
        mk(QStringLiteral("zoom.%1").arg(pct), QStringLiteral("%1%").arg(pct), pct == 100 ? "zoom-in" : "", QKeySequence(), [this, pct] { m_view->setZoom(pct / 100.0); });
    }
    act("zoom.100")->setText(tr("100%"));
    mk("view.freeze", tr("Freeze Panes"), "snowflake", QKeySequence(), [this] { freezeAtSelection(); });
    mk("view.freezeRow", tr("Freeze Top Row"), "panel-top", QKeySequence(), [this] { m_view->setFrozen(1, m_view->sheet().layout().frozenCols()); });
    mk("view.freezeCol", tr("Freeze First Column"), "panel-left", QKeySequence(), [this] { m_view->setFrozen(m_view->sheet().layout().frozenRows(), 1); });
    mk("view.unfreeze", tr("Unfreeze Panes"), "panels-top-left", QKeySequence(), [this] { m_view->setFrozen(0, 0); });
    // ---- formulas
    mk("formulas.calculate", tr("Calculate Now"), "calculator", QKeySequence(Qt::Key_F9), [this] {
        if (!m_loaded || !m_loaded->calc || m_loading) return;
        m_loaded->calc->invalidateAll();
        startCalculation();
    });
    // ---- help
    mk("help.show", tr("Help"), "circle-help", QKeySequence::HelpContents, [this] { showHelp(helpContext()); });
    connect(m_ribbon, &Ribbon::helpClicked, this, [this] { act("help.show")->trigger(); });
    createDisabledActions();
}

// The commands of the ribbon that come with editing (and the later parts of JeffCrunch): they are on the ribbon, and disabled, until they work.
void CrunchWindow::createDisabledActions()
{
    struct Command {
        const char *id, *text, *icon;
    };
    static const Command commands[] = {
        {"edit.cut", QT_TR_NOOP("Cut"), "scissors"}, {"edit.paste", QT_TR_NOOP("Paste"), "clipboard-paste"}, {"edit.undo", QT_TR_NOOP("Undo"), "undo-2"},
        {"edit.redo", QT_TR_NOOP("Redo"), "redo-2"}, {"edit.fill", QT_TR_NOOP("Fill"), "arrow-down-to-line"}, {"edit.clear", QT_TR_NOOP("Clear"), "eraser"},
        {"edit.sort", QT_TR_NOOP("Sort and Filter"), "arrow-down-a-z"}, {"edit.find", QT_TR_NOOP("Find and Select"), "search"}, {"edit.autosum", QT_TR_NOOP("AutoSum"), "sigma"},
        {"fmt.bold", QT_TR_NOOP("Bold"), "bold"}, {"fmt.italic", QT_TR_NOOP("Italic"), "italic"}, {"fmt.underline", QT_TR_NOOP("Underline"), "underline"},
        {"fmt.strike", QT_TR_NOOP("Strikethrough"), "strikethrough"}, {"fmt.fillColor", QT_TR_NOOP("Fill Color"), "paint-bucket"}, {"fmt.fontColor", QT_TR_NOOP("Font Color"), "baseline"},
        {"fmt.alignLeft", QT_TR_NOOP("Align Left"), "align-left"}, {"fmt.alignCenter", QT_TR_NOOP("Center"), "align-center"}, {"fmt.alignRight", QT_TR_NOOP("Align Right"), "align-right"},
        {"fmt.wrap", QT_TR_NOOP("Wrap Text"), "wrap-text"}, {"fmt.merge", QT_TR_NOOP("Merge and Center"), "table-cells-merge"}, {"fmt.indentInc", QT_TR_NOOP("Increase Indent"), "indent-increase"},
        {"fmt.indentDec", QT_TR_NOOP("Decrease Indent"), "indent-decrease"}, {"fmt.currency", QT_TR_NOOP("Currency"), "dollar-sign"}, {"fmt.percent", QT_TR_NOOP("Percent"), "percent"},
        {"fmt.comma", QT_TR_NOOP("Comma Style"), "hash"}, {"fmt.decimalInc", QT_TR_NOOP("Increase Decimal"), "arrow-up-0-1"}, {"fmt.decimalDec", QT_TR_NOOP("Decrease Decimal"), "arrow-down-1-0"},
        {"cells.insert", QT_TR_NOOP("Insert"), "square-plus"}, {"cells.delete", QT_TR_NOOP("Delete"), "square-minus"}, {"cells.format", QT_TR_NOOP("Format"), "table-properties"},
        {"insert.table", QT_TR_NOOP("Table"), "table"}, {"insert.picture", QT_TR_NOOP("Picture"), "image"}, {"insert.shape", QT_TR_NOOP("Shapes"), "shapes"},
        {"insert.chart", QT_TR_NOOP("Chart"), "chart-column"}, {"insert.pivot", QT_TR_NOOP("PivotTable"), "table-2"}, {"insert.link", QT_TR_NOOP("Link"), "link"},
        {"insert.text", QT_TR_NOOP("Text Box"), "text-cursor"}, {"formulas.insertFunction", QT_TR_NOOP("Insert Function"), "square-function"},
        {"formulas.names", QT_TR_NOOP("Name Manager"), "variable"}, {"formulas.options", QT_TR_NOOP("Calculation Options"), "settings-2"},
    };
    for (const Command &c : commands) mk(QString::fromLatin1(c.id), QCoreApplication::translate("jp::CrunchWindow", c.text), c.icon, QKeySequence(), nullptr, false, false);
}

void CrunchWindow::copySelection()
{
    m_copy = m_view->copyWalk();
    if (!m_copy) {
        QApplication::clipboard()->clear();
        return;
    }
    if (m_copy->tooMany()) {
        m_copy.reset();
        notice(tr("Copy"), tr("That is more than a million cells, which JeffCrunch doesn't copy as text. Select fewer cells."), false);
        return;
    }
    // (a large selection is turned to text a few milliseconds at a time, and put on the clipboard when it is all text)
    setStatus(tr("Copying…"));
    m_copyTimer.start(0);
}

void CrunchWindow::copyStep()
{
    if (!m_copy) return;
    QElapsedTimer t;
    t.start();
    bool done = false;
    while (!done && t.elapsed() < 4) done = m_copy->step(20000);
    if (!done) {
        m_copyTimer.start(0);
        return;
    }
    QApplication::clipboard()->setText(m_copy->text());
    m_copy.reset();
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
}

void CrunchWindow::freezeAtSelection()
{
    const grid::Selection &s = m_view->selection();
    m_view->setFrozen(s.activeRow, s.activeCol);
}

void CrunchWindow::notice(const QString &title, const QString &text, bool warning)
{
    if (m_notifier) {
        m_notifier(title, text);
        return;
    }
    if (warning) QMessageBox::warning(this, title, text);
    else QMessageBox::information(this, title, text);
}

// ---- keeping the bars in step with the selection

void CrunchWindow::updateBars()
{
    if (!m_nameBox->hasFocus()) m_nameBox->setText(m_view->selectionName());
    m_formulaBar->setText(m_view->activeCellText());
    m_formulaBar->setCursorPosition(0);
}

void CrunchWindow::goToName()
{
    const QString text = m_nameBox->text().trimmed();
    static const QRegularExpression columns(QStringLiteral("^([A-Za-z]{1,3}):([A-Za-z]{1,3})$")), rows(QStringLiteral("^(\\d{1,7}):(\\d{1,7})$"));
    jp::office::sheet::CellRange range;
    bool done = false;
    if (const auto m = columns.match(text); m.hasMatch()) {
        quint32 r = 0, c1 = 0, c2 = 0;
        if (jp::office::sheet::parseCellRef(m.captured(1) + QLatin1Char('1'), &r, &c1) && jp::office::sheet::parseCellRef(m.captured(2) + QLatin1Char('1'), &r, &c2)) {
            m_view->selectColumns(c1, c2);
            done = true;
        }
    } else if (const auto m2 = rows.match(text); m2.hasMatch()) {
        const qint64 a = m2.captured(1).toLongLong() - 1, b = m2.captured(2).toLongLong() - 1;
        if (a >= 0 && b >= 0 && a < qint64(jp::office::sheet::kMaxRows) && b < qint64(jp::office::sheet::kMaxRows)) {
            m_view->selectRows(a, b);
            done = true;
        }
    } else if (jp::office::sheet::parseRangeRef(text, &range)) {
        m_view->selectRange(range.row1, range.col1, range.row2, range.col2);
        done = true;
    }
    if (!done) setStatus(tr("\"%1\" is not an address JeffCrunch can go to.").arg(text));
    m_nameBox->setText(m_view->selectionName());
    m_view->setFocus(Qt::OtherFocusReason);
}

void CrunchWindow::updateTitle()
{
    const QString name = m_path.isEmpty() ? tr("Book%1").arg(s_untitledCount) : QFileInfo(m_path).fileName();
    setWindowTitle(QStringLiteral("%1 - %2").arg(name, appInfo().name));
}

QString CrunchWindow::displayName() const { return m_path.isEmpty() ? tr("Book%1").arg(s_untitledCount) : QFileInfo(m_path).completeBaseName(); }

// ---- the File screen

void CrunchWindow::showFileScreen(const QString &page)
{
    m_view->setFocus();
    m_fileScreen->setGeometry(rect());
    m_fileScreen->showPage(page.isEmpty() ? QStringLiteral("open") : page);
    m_fileScreen->show();
    m_fileScreen->raise();
    m_fileScreen->setFocus();
    qApp->installEventFilter(this);
}

void CrunchWindow::hideFileScreen()
{
    if (!m_fileScreen->isVisible()) return;
    qApp->removeEventFilter(this);
    m_fileScreen->hide();
    m_view->setFocus();
}

void CrunchWindow::resizeEvent(QResizeEvent *e)
{
    AppWindow::resizeEvent(e);
    if (m_fileScreen) m_fileScreen->setGeometry(rect());
}

bool CrunchWindow::eventFilter(QObject *o, QEvent *e)
{
    // Escape closes the File screen from anywhere in the window.
    if (e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape && m_fileScreen->isVisible()) {
        auto *w = qobject_cast<QWidget *>(o);
        if (w && w->window() == this) {
            hideFileScreen();
            return true;
        }
    }
    return AppWindow::eventFilter(o, e);
}

void CrunchWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (!e->mimeData()->hasUrls()) return;
    for (const QUrl &u : e->mimeData()->urls())
        if (u.isLocalFile()) {
            e->acceptProposedAction();
            return;
        }
}

void CrunchWindow::dropEvent(QDropEvent *e)
{
    for (const QUrl &u : e->mimeData()->urls())
        if (u.isLocalFile()) openFile(u.toLocalFile());
}

// Esc stops the calculation (what is not worked out keeps the result the file stored); with the File screen over the window it closes the screen instead.
void CrunchWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape && m_calculating && !m_fileScreen->isVisible()) {
        stopCalculation();
        setStatus(tr("Calculation stopped"));
        e->accept();
        return;
    }
    AppWindow::keyPressEvent(e);
}

void CrunchWindow::closeEvent(QCloseEvent *e)
{
    stopCalculation();
    m_loader.cancel();
    e->accept();
}

} // namespace jp
