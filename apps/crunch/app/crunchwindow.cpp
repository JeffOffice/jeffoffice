#include "app/crunchwindow.h"
#include "app/plaintext.h"

#include "core/printjob.h"
#include <QPrinter>

#include "app/crunchfilescreen.h"
#include "app/crunchinfo.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/keytips.h"
#include "app/optionsdialog.h"
#include "app/otherformats.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/sheettabs.h"
#include "app/telemetry.h"
#include "app/widgets.h"
#include "core/sheetobjects.h"
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
#include <QUndoStack>
#include <QThread>
#include <QVBoxLayout>

#include <cmath>

namespace jp {

namespace {
int s_untitledCount = 0;
}

namespace {
QList<CrunchWindow *> &openWindows()
{
    static QList<CrunchWindow *> windows;
    return windows;
}
} // namespace

CrunchWindow *CrunchWindow::windowWith(const QString &path)
{
    const QString wanted = QFileInfo(path).canonicalFilePath();
    if (wanted.isEmpty()) return nullptr;
    for (CrunchWindow *w : openWindows())
        if (!w->m_path.isEmpty() && (w->m_loaded || w->m_loading) && QFileInfo(w->m_path).canonicalFilePath() == wanted) return w;
    return nullptr;
}

CrunchWindow::CrunchWindow(QWidget *parent) : AppWindow(parent)
{
    openWindows().append(this);
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
    m_printChoices = std::make_unique<crunch::PrintChoices>();
    m_autoTimer.setSingleShot(true);
    m_autoTimer.setInterval(0);
    connect(&m_autoTimer, &QTimer::timeout, this, &CrunchWindow::stepAutoHeights);
    m_breaksTimer.setSingleShot(true);
    m_breaksTimer.setInterval(150);
    connect(&m_breaksTimer, &QTimer::timeout, this, &CrunchWindow::updatePageBreaks);
    m_breaksStepTimer.setSingleShot(true);
    m_breaksStepTimer.setInterval(0);
    connect(&m_breaksStepTimer, &QTimer::timeout, this, &CrunchWindow::stepPageBreaks);
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
    connect(m_view, &grid::GridView::breakMoveRequested, this, &CrunchWindow::breakLineMoved);
    connect(m_view, &grid::GridView::printEdgeRequested, this, &CrunchWindow::printEdgeMoved);
    connect(m_view, &grid::GridView::zoomChanged, this, [this](double z) {
        // (the zoom of the cells is kept with the sheet: in the view of the cells it is the sheet's zoom, in the page breaks view the zoom of that view)
        if (m_viewKind != 2 && !m_showingSheet && m_session && m_sheet >= 0) m_session->setSheetView(m_sheet, m_viewKind, int(std::lround(z * 100)));
    });
    connect(m_view, &grid::GridView::selectionChanged, this, [this] {
        if (!m_finishing) m_tabOriginCol = m_tabRow = -1;
    });
    // editing: a key that types starts an entry in the cell, F2 and a double click edit what it holds, Delete clears
    connect(m_view, &grid::GridView::typingStarted, this, [this](const QString &text) { beginEdit(text, CellEditor::Mode::Typing, false); });
    connect(m_view, &grid::GridView::editRequested, this, [this] {
        if (m_edit.active || !editingAllowed()) return;
        const grid::Selection &sel = m_view->selection();
        beginEdit(m_session->editText(m_sheet, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol))), CellEditor::Mode::Editing, false);
    });
    connect(m_view, &grid::GridView::clearRequested, this, &CrunchWindow::clearSelection);
    connect(m_view, &grid::GridView::viewChanged, this, &CrunchWindow::placeEditor);
    connect(m_view, &grid::GridView::viewChanged, this, &CrunchWindow::scheduleAutoHeights);
    m_view->setClickGuard([this] { return commitEdit(CellEditor::Way::Stay); });
    connect(m_cellEditor, &CellEditor::finished, this, [this](CellEditor::Way way) { commitEdit(way); });
    connect(m_cellEditor, &CellEditor::cancelled, this, &CrunchWindow::cancelEdit);
    connect(m_cellEditor, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_syncingEditors) return;
        m_syncingEditors = true;
        m_formulaBar->setText(text);
        m_syncingEditors = false;
        placeEditor();
    });
    connect(m_formulaBar, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_syncingEditors) return;
        if (!m_edit.active) {
            beginEdit(text, CellEditor::Mode::Editing, true);
            return;
        }
        m_syncingEditors = true;
        m_cellEditor->setText(text);
        m_syncingEditors = false;
        placeEditor();
    });
    connect(m_view, &grid::GridView::statsChanged, this, [this](const QString &text) { m_stats->setText(text); });
    connect(m_view, &grid::GridView::noticeRaised, this, [this](const QString &text) { setStatus(text); });   // (a limit that stopped a conditional format: said in the status bar)
    connect(m_view, &grid::GridView::zoomChanged, this, [this](double z) {
        placeEditor();
        m_zoomLabel->setText(QStringLiteral("%1%").arg(std::lround(z * 100)));
        const QSignalBlocker block(m_zoomSlider);
        m_zoomSlider->setValue(int(std::lround(std::log(z) / std::log(1.05))));
    });
    connect(m_view, &grid::GridView::contextMenuRequested, this, [this](const QPoint &globalPos) { showContextMenu(globalPos); });
    connect(&m_loader, &crunch::WorkbookLoader::firstScreen, this, &CrunchWindow::onFirstScreen);
    connect(&m_loader, &crunch::WorkbookLoader::progress, this, &CrunchWindow::onProgress);
    connect(&m_loader, &crunch::WorkbookLoader::finished, this, &CrunchWindow::onFinished);
    connect(&m_loader, &crunch::WorkbookLoader::calcBatch, this, &CrunchWindow::onCalcBatch);
    m_copyTimer.setSingleShot(true);
    connect(&m_copyTimer, &QTimer::timeout, this, &CrunchWindow::copyStep);
    connect(m_tabs, &crunch::SheetTabBar::contextRequested, this, [this](int, const QPoint &global) { showTabMenu(global); });
    connect(m_tabs, &crunch::SheetTabBar::renamed, this, [this](int tab, const QString &name) {
        if (tab >= 0 && tab < m_tabSheets.size()) renameSheetTo(m_tabSheets[tab], name);
    });
    connect(m_tabs, &crunch::SheetTabBar::tabDragged, this, [this](int from, int to) { onTabMoved(from, to); });
    connect(m_tabs, &QTabBar::currentChanged, this, [this](int tab) {
        if (m_switchingTabs || m_tabs->isDragging() || tab < 0 || tab >= m_tabSheets.size()) return;
        setActiveSheet(m_tabSheets[tab]);
    });
    showBlank();
    setupObjects();
    resize(1280, 800);
    updateTitle();
    updateBars();
}

CrunchWindow::~CrunchWindow()
{
    stopPasswordJob();   // (a password being worked on is given up with the window)
    if (m_tabs) m_tabs->cancelRename();   // (a name being typed is given up with the window, not given to a window that is going)
    if (m_view) m_view->setOverlay(nullptr);   // (the objects over the cells go before the grid does)
    if (m_pageView) m_pageView->setSource(nullptr);
    openWindows().removeAll(this);
    // (the session's undo stack tells the window as its steps go: it is let go of first, and the window is not told)
    if (m_session) {
        m_session->undoStack()->disconnect(this);
        m_session->disconnect(this);
        m_session.reset();
    }
    m_sortCancel = true;
    if (m_sortThread) m_sortThread->wait();   // (the order a thread is working out is given up)
    m_loader.cancel();
    if (m_saveThread) m_saveThread->wait();   // (a save that is going on is finished: it ends with the file whole)
}

QAction *CrunchWindow::mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn, bool checkable, bool enabled)
{
    auto *a = new QAction(text, this);
    a->setObjectName(id);
    if (!iconName.isEmpty()) a->setIcon(icon(iconName));
    if (!key.isEmpty()) {
        a->setShortcut(key);
        a->setShortcutContext(Qt::WindowShortcut);
        a->setToolTip(jp::plainToolTip(tr("%1 (%2)").arg(QString(text).remove('&'), key.toString(QKeySequence::NativeText))));
        addAction(a);
    }
    a->setCheckable(checkable);
    a->setEnabled(enabled);
    if (!enabled) a->setToolTip(jp::plainToolTip(tr("%1: not available in JeffCrunch yet").arg(QString(text).remove('&'))));
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
    m_formulaBar->setMaxLength(1 << 20);
    m_formulaBar->setAccessibleName(tr("Formula bar"));
    m_formulaBar->setAccessibleDescription(tr("What the selected cell holds: its formula, or its value. Type here to change it, and press Enter."));
    row->addWidget(m_nameBox);
    row->addWidget(fx);
    row->addWidget(m_formulaBar, 1);
    v->addWidget(m_formulaRow);
    m_view = new grid::GridView(central);
    m_view->setAccessibleName(tr("Sheet"));
    m_view->setAccessibleDescription(tr("The cells of the sheet. The arrow keys move the selection; Ctrl and an arrow go to the edge of the data."));
    v->addWidget(m_view, 1);
    buildPageLayoutView(central, v);
    m_cellEditor = new CellEditor(m_view->viewport());
    m_cellEditor->hide();
    m_cellEditor->setAccessibleName(tr("Cell editor"));
    m_cellEditor->setAccessibleDescription(tr("The cell being typed into. Enter and Tab finish the entry; Escape gives it up."));
    m_sheetTabRow = new QWidget(central);
    auto *tabLayout = new QHBoxLayout(m_sheetTabRow);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(0);
    m_tabs = new crunch::SheetTabBar(m_sheetTabRow);
    tabLayout->addWidget(m_tabs, 1);
    v->addWidget(m_sheetTabRow);
    setCentralWidget(central);
    connect(m_nameBox, &QLineEdit::returnPressed, this, &CrunchWindow::goToName);
    m_formulaBar->installEventFilter(this);
    createFormulaHelper();
}

void CrunchWindow::buildStatusBar()
{
    QStatusBar *sb = statusBar();
    m_status = plainLabel(new QLabel(tr("Ready"), this));
    m_status->setObjectName(QStringLiteral("statusText"));
    m_status->setMinimumWidth(220);
    sb->addWidget(m_status);
    m_stats = plainLabel(new QLabel(this));
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
    addViewButtons(sb);
    button("zoom.out");
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setObjectName(QStringLiteral("zoomSlider"));
    m_zoomSlider->setRange(-47, 28);   // 1.05^n: 10% .. 400%
    m_zoomSlider->setFixedWidth(140);
    m_zoomSlider->setAccessibleName(tr("Zoom"));
    m_zoomSlider->setAccessibleDescription(tr("Left and Right arrows zoom out and in."));
    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int v) { setViewZoom(std::pow(1.05, v)); });
    sb->addPermanentWidget(m_zoomSlider);
    button("zoom.in");
    m_zoomLabel = plainLabel(new QLabel(QStringLiteral("100%")));
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
                                                       crunch::openFilter() + QStringLiteral(";;") + tr("All Files (*)"));
        if (p.isEmpty()) return;
        Settings::get().setValue("dirs/open", QFileInfo(p).absolutePath());
        openFile(p);
    });
    mk("file.openText", tr("Open Text File…"), "file-text", QKeySequence(), [this] {
        const QString p = QFileDialog::getOpenFileName(this, tr("Open Text File"), Settings::get().value("dirs/open", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                       tr("Text files (*.csv *.tsv *.txt)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (p.isEmpty()) return;
        Settings::get().setValue("dirs/open", QFileInfo(p).absolutePath());
        m_forceTextPrompt = true;
        openFile(p);
        m_forceTextPrompt = false;
    });
    mk("file.save", tr("Save"), "save", QKeySequence::Save, [this] { save(); }, false, false);
    mk("file.saveAs", tr("Save As…"), "save", QKeySequence::SaveAs, [this] {
        if (!m_loaded || !m_session || m_saving || !commitEdit()) return;
        // (a workbook read from the older binary form: the same question as Save, which tells the person it is saved as .xlsx)
        if (savesAsANewFile()) {
            const QString path = pathToSave();
            if (!path.isEmpty()) saveAs(path);
            return;
        }
        const QString kind = saveKind();
        const QString start = m_path.isEmpty() ? displayName() + QLatin1Char('.') + kind : m_path;
        const QString filter = (kind == QLatin1String("xlsm") ? tr("Macro-enabled workbooks (*.xlsm)") : tr("Workbooks (*.xlsx)")) + QStringLiteral(";;") + crunch::csvFilter();
        const QString p = askForSavePath(tr("Save Workbook As"), start, filter);
        if (!p.isEmpty()) saveAs(p);
    }, false, false);
    mk("file.close", tr("Close"), "circle-x", QKeySequence::Close, [this] { close(); });
    mk("edit.undo", tr("Undo"), "undo-2", QKeySequence::Undo, [this] { undoRedo(false); }, false, false);
    mk("edit.redo", tr("Redo"), "redo-2", QKeySequence::Redo, [this] { undoRedo(true); }, false, false);
    mk("file.options", tr("Options"), "settings", QKeySequence(), [this] {
        // The shared pages (General, Proofing); JeffCrunch has no pages of its own yet.
        Editor stand;
        optionsDialog(this, &stand, {});
    });
    mk("edit.selectAll", tr("Select All"), "", QKeySequence(), [this] { m_view->selectAll(); });
    // ---- view
    mk("view.gridlines", tr("Gridlines"), "grid-3x3", QKeySequence(), [this] { m_view->setShowGridlines(act("view.gridlines")->isChecked()); }, true)->setChecked(true);
    mk("view.headings", tr("Headings"), "table-properties", QKeySequence(), [this] { m_view->setShowHeadings(act("view.headings")->isChecked()); }, true)->setChecked(true);
    mk("view.formulaBar", tr("Formula Bar"), "text-cursor-input", QKeySequence(), [this] { m_formulaRow->setVisible(act("view.formulaBar")->isChecked()); }, true)->setChecked(true);
    mk("view.pageBreaks", tr("Page Lines"), "separator-horizontal", QKeySequence(), [this] { updatePageBreaks(); }, true)->setChecked(true);
    mk("view.formulas", tr("Show Formulas"), "square-function", crunch::showFormulasKey(), [this] {
        m_view->sheet().setShowFormulas(act("view.formulas")->isChecked());
        m_view->invalidateValues();
    }, true);
    mk("zoom.in", tr("Zoom In"), "zoom-in", QKeySequence(), [this] { setViewZoom(std::round(viewZoom() * 10 + 1) / 10); });
    mk("zoom.out", tr("Zoom Out"), "zoom-out", QKeySequence(), [this] { setViewZoom(std::round(viewZoom() * 10 - 1) / 10); });
    for (int pct : {200, 150, 100, 75, 50, 25, 10}) {
        mk(QStringLiteral("zoom.%1").arg(pct), QStringLiteral("%1%").arg(pct), pct == 100 ? "zoom-in" : "", QKeySequence(), [this, pct] { setViewZoom(pct / 100.0); });
    }
    act("zoom.100")->setText(tr("100%"));
    mk("view.freeze", tr("Freeze Panes"), "snowflake", QKeySequence(), [this] { freezeAtSelection(); });
    mk("view.freezeRow", tr("Freeze Top Row"), "panel-top", QKeySequence(), [this] { m_view->freezeTopRow(); });
    mk("view.freezeCol", tr("Freeze First Column"), "panel-left", QKeySequence(), [this] { m_view->freezeFirstColumn(); });
    mk("view.unfreeze", tr("Unfreeze Panes"), "panels-top-left", QKeySequence(), [this] { m_view->unfreeze(); });
    // ---- formulas
    mk("formulas.calculate", tr("Recalculate All"), "calculator", QKeySequence(Qt::Key_F9), [this] {
        // (the calculation is the worker's: it is asked to work everything out again, and does)
        if (m_loading || !m_haveCalc || !m_loader.recalculate()) return;
        m_calculating = true;
        setStatus(tr("Calculating…"));
    });
    // ---- help
    mk("help.show", tr("Help"), "circle-help", QKeySequence::HelpContents, [this] { showHelp(helpContext()); });
    connect(m_ribbon, &Ribbon::helpClicked, this, [this] { act("help.show")->trigger(); });
    createOpsActions();
    createCellsActions();
    createClipboardActions();
    createFindActions();
    createSortActions();
    createFilterActions();
    createFunctionActions();
    createNameActions();
    createCalcActions();
    createAuditActions();
    createFormatActions();
    createSheetActions();
    createPageActions();
    createViewActions();
    createPageLayoutActions();
    createChartActions();
    createNoteActions();
    createProtectActions();
    createDisabledActions();
}

// The commands of the ribbon that come with editing (and the later parts of JeffCrunch): they are on the ribbon, and disabled, until they work.
void CrunchWindow::createDisabledActions()
{
    struct Command {
        const char *id, *text, *icon;
    };
    static const Command commands[] = {
        
        {"insert.table", QT_TR_NOOP("Table"), "table"}, {"insert.picture", QT_TR_NOOP("Picture"), "image"}, {"insert.shape", QT_TR_NOOP("Shapes"), "shapes"},
        {"insert.pivot", QT_TR_NOOP("PivotTable"), "table-2"},
        {"insert.text", QT_TR_NOOP("Text Box"), "text-cursor"},
    };
    for (const Command &c : commands) mk(QString::fromLatin1(c.id), QCoreApplication::translate("jp::CrunchWindow", c.text), c.icon, QKeySequence(), nullptr, false, false);
}

void CrunchWindow::freezeAtSelection()
{
    // the rows above the cell with the cursor that are in view, and the columns to its left that are (what was scrolled out of sight stays out of sight)
    const grid::Selection &s = m_view->selection();
    m_view->freezeAt(s.activeRow, s.activeCol);
}

void CrunchWindow::notice(const QString &title, const QString &text, bool warning)
{
    if (m_notifier) {
        m_notifier(title, text);
        return;
    }
    plainBox(this, warning ? QMessageBox::Warning : QMessageBox::Information, title, text);
}

// ---- keeping the bars in step with the selection

void CrunchWindow::updateBars()
{
    updateDropDown();
    showNameInBox();
    if (m_edit.active) return;   // (the formula bar holds what is being typed)
    // The formula bar takes about 20 milliseconds to be given 32,000 characters, whenever the cursor moves on to a cell that has them: it is given a part of a long text (with "…"
    // after it) until it is clicked into or tabbed to, and the whole of it then.
    constexpr int kShown = 2000;
    m_formulaFull = m_view->activeCellText();
    if (m_session && m_loaded) m_formulaFull = m_session->barText(m_sheet, quint32(std::max<qint64>(0, m_view->selection().activeRow)), quint32(std::max<qint64>(0, m_view->selection().activeCol)), m_formulaFull);
    if (m_formulaFull.size() <= kShown || m_formulaBar->hasFocus()) m_formulaBar->setText(m_formulaFull);
    else m_formulaBar->setText(m_formulaFull.left(kShown) + QChar(0x2026));
    m_formulaBar->setCursorPosition(0);
    updateFormatState(false);
}

// The name box: the address of the selection, or the name of the object that is picked.
void CrunchWindow::showNameInBox()
{
    if (m_nameBox->hasFocus() || m_chartSheetShown) return;
    const int id = pickedObject();
    m_nameBox->setText(id >= 0 ? m_objects->nameOf(m_sheet, id) : m_view->selectionName());
}

void CrunchWindow::goToName()
{
    if (!commitEdit()) return;
    const QString text = m_nameBox->text().trimmed();
    if (const int id = pickedObject(); id >= 0 && m_session && m_objects) {
        // with an object picked, what is typed is its name
        if (!editingAllowed()) {
            m_nameBox->setText(m_objects->nameOf(m_sheet, id));
            return;
        }
        if (text.isEmpty() || text.size() > 255) {
            setStatus(tr("An object's name is one to 255 characters."));
            m_nameBox->setText(m_objects->nameOf(m_sheet, id));
            return;
        }
        if (text != m_objects->nameOf(m_sheet, id)) {
            crunch::ObjectEdit e;
            e.sheet = m_sheet;
            e.id = id;
            e.before = m_objects->stateOf(m_sheet, id);
            e.after = e.before;
            e.after.name = text;
            const crunch::EditSession::Outcome o = m_session->changeObjects({e}, tr("Rename Object"));
            if (!o.ok) setStatus(o.problem);
        }
        m_nameBox->setText(m_objects->nameOf(m_sheet, id));
        m_view->setFocus(Qt::OtherFocusReason);
        return;
    }
    bool done = false;
    if (m_session && m_loaded) {
        // (an address, a cell of another sheet, whole columns or rows, or the name of a range)
        const crunch::GoToTarget t = crunch::resolveGoTo(*m_session, text, m_sheet);
        if (t.ok) {
            if (t.sheet != m_sheet) setActiveSheet(t.sheet);
            if (t.sheet == m_sheet) {
                if (t.wholeRows) m_view->selectRows(t.range.row1, t.range.row2);
                else if (t.wholeColumns) m_view->selectColumns(t.range.col1, t.range.col2);
                else m_view->selectRange(t.range.row1, t.range.col1, t.range.row2, t.range.col2);
                done = true;
            }
        }
    } else {
        jp::office::sheet::CellRange range;
        if (jp::office::sheet::parseRangeRef(text, &range)) {
            m_view->selectRange(range.row1, range.col1, range.row2, range.col2);
            done = true;
        }
    }
    if (!done) setStatus(tr("\"%1\" is not an address JeffCrunch can go to.").arg(text));
    m_nameBox->setText(m_view->selectionName());
    m_view->setFocus(Qt::OtherFocusReason);
}

void CrunchWindow::updateTitle()
{
    const QString name = m_path.isEmpty() ? tr("Book%1").arg(s_untitledCount) : QFileInfo(m_path).fileName();
    setWindowTitle(QStringLiteral("%1[*] - %2").arg(name, appInfo().name));
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
    // the formula bar has the whole of a long text once it has the focus
    if (o == m_formulaBar && e->type() == QEvent::FocusIn && m_formulaBar->text() != m_formulaFull) {
        m_formulaBar->setText(m_formulaFull);
        m_formulaBar->setCursorPosition(0);
    }
    // Enter and Escape in the formula bar finish or give up the entry in it (and take the focus back to the grid)
    if (o == m_formulaBar && e->type() == QEvent::KeyPress) {
        auto *k = static_cast<QKeyEvent *>(e);
        if (k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter) {
            if (m_edit.active) commitEdit((k->modifiers() & Qt::ShiftModifier) ? CellEditor::Way::ShiftEnter : CellEditor::Way::Enter);
            else m_view->setFocus(Qt::OtherFocusReason);
            return true;
        }
        if (k->key() == Qt::Key_Escape && !m_fileScreen->isVisible()) {
            if (m_edit.active) cancelEdit();
            else m_view->setFocus(Qt::OtherFocusReason);
            return true;
        }
    }
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
    if (e->key() == Qt::Key_Escape && m_painter.mode != PainterMode::Off && !m_edit.active) {
        cancelFormatPainter();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && (m_sortPlan || m_findRun || m_filterPlan || m_replacePlan) && !m_edit.active) {
        if (m_sortPlan) cancelSort();
        if (m_filterPlan) cancelFilter();
        if (m_findRun || m_replacePlan) cancelSearch();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && m_calculating && !m_fileScreen->isVisible()) {
        if (!stopCalculation()) setStatus(tr("Calculation stopped"));   // (a save that was waiting says it was cancelled)
        e->accept();
        return;
    }
    AppWindow::keyPressEvent(e);
}

void CrunchWindow::closeEvent(QCloseEvent *e)
{
    // an entry that is being made is finished (or the window stays, with the reason shown), and changes that are not saved are asked about
    if (!commitEdit() || !askAboutChanges()) {
        e->ignore();
        return;
    }
    stopCalculation();
    m_loader.cancel();
    e->accept();
}

} // namespace jp
