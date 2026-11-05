// Opening a workbook in JeffCrunch's window: the first rows at once, the sheets, and the calculation done in slices.

#include "app/crunchwindow.h"
#include "app/functionargumentsdialog.h"

#include "io/crypto.h"
#include "sheet/csv.h"
#include "sheet/sheetcrypt.h"

#include "app/crunchfilescreen.h"
#include "app/oldformat.h"
#include "app/otherformats.h"
#include "office/package.h"
#include "office/repair.h"
#include "sheet/xls.h"
#include "app/settings.h"
#include "app/sheettabs.h"
#include "app/telemetry.h"
#include "core/sheetobjects.h"
#include "core/appinfo.h"
#include "core/threadtime.h"

#include <QPointer>
#include <QTimer>
#include <QApplication>
#include <QAbstractButton>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QInputDialog>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>

#include <algorithm>
#include <utility>

namespace jp {

using jp::office::sheet::SheetKind;
using jp::office::sheet::SheetState;

void CrunchWindow::showBlank()
{
    grid::GridData d;
    d.styles = &m_emptyStyles;
    d.strings = &m_emptyStrings;
    d.sheet = &m_emptySheet;
    m_view->setData(d);
    // (a blank grid has cells: what a chart sheet had turned off is back)
    m_chartSheetShown = false;
    if (m_formulaBar) m_formulaBar->setReadOnly(false);
    if (m_nameBox) m_nameBox->setEnabled(true);
    if (!m_actions.isEmpty()) updateEditActions();
}

// The window holds no workbook: the grid is empty and nothing of the one before is left.
void CrunchWindow::dropWorkbook()
{
    forgetFilePassword();
    cancelPageBreaks();
    dropLayoutJob();
    m_loader.cancel();
    // (what was waiting for the old workbook is not done to the next: a save that waited for its calculation, a close that waited for that save, what was typed before it was ready)
    m_saveAfterCalculation.clear();
    m_copyAfterCalculation = false;
    m_closeAfterSave = false;
    m_pendingTyping.clear();
    m_findRun.reset();
    m_replacePlan.reset();
    m_sortPlan.reset();
    m_values.clear();
    m_path.clear();
    m_originalPath.clear();
    m_blank = false;
    if (m_argsDialog) m_argsDialog->reject();   // (its call has nowhere to go once the workbook is gone)
    m_session.reset();
    m_loaded.reset();
    m_haveFirstScreen = m_haveCalc = false;
    m_calculating = false;
    m_views.clear();
    m_sheet = -1;
    if (m_objects) m_objects->open(nullptr);   // (nothing opened: no object is left over the blank grid)
    m_view->setCellsEnabled(true);
    showBlank();
    fillTabs();
    setWindowModified(false);
}

bool CrunchWindow::newBlank()
{
    if (m_loaded || m_loading) return false;
    if (m_untitled.isEmpty()) m_untitled = nextUntitledName();
    m_blank = true;
    m_loading = true;
    m_haveFirstScreen = m_haveCalc = false;
    m_calculating = false;
    m_values.clear();
    m_longestBatchMicros = m_batches = m_longestBatchItems = 0;
    m_loadProblem.clear();
    crunch::BlankDefaults defaults;
    defaults.font = Settings::get().value(QStringLiteral("crunch/newFont"), defaults.font).toString();
    defaults.size = Settings::get().value(QStringLiteral("crunch/newFontSize"), defaults.size).toDouble();
    defaults.sheets = Settings::get().value(QStringLiteral("crunch/newSheets"), defaults.sheets).toInt();
    defaults.paperSize = jp::usesLetterPaper(QLocale()) ? 1 : 9;   // Letter where Letter is used, else A4
    m_loader.startBlank(defaults);
    setStatus(tr("Ready"));
    updateTitle();
    return true;
}

bool CrunchWindow::isUntouchedBlank() const
{
    if (!m_blank || !m_path.isEmpty() || m_saving || m_edit.active || m_applying || m_findRun || m_replacePlan || m_sortPlan) return false;
    return !m_session || (!m_session->isModified() && !m_session->viewsChanged());
}

void CrunchWindow::browseAndOpen(OpenHow how)
{
    const QString p = QFileDialog::getOpenFileName(this, openHowText(how), Settings::get().value("dirs/open", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                   crunch::openFilter() + QStringLiteral(";;") + tr("All Files (*)"));
    if (p.isEmpty()) return;
    Settings::get().setValue("dirs/open", QFileInfo(p).absolutePath());
    openFile(p, how);
}

void CrunchWindow::setReadOnly(bool on)
{
    AppWindow::setReadOnly(on);
    if (!m_actions.isEmpty()) {
        updateEditActions();
        updateFinalNote();
    }
    updateTitle();
}

bool CrunchWindow::openFile(const QString &path, OpenHow how)
{
    const QFileInfo info(path);
    const QString name = info.fileName();
    if (!info.exists() || !info.isFile()) {
        notice(tr("Open"), tr("%1 can't open \"%2\".\nThe file isn't there.").arg(appInfo().name, name));
        return false;
    }
    // A file that is open in a window is not opened again: that window is brought to the front
    if (CrunchWindow *have = how == OpenHow::Copy || m_reopening ? nullptr : windowWith(path)) {
        have->setWindowState(have->windowState() & ~Qt::WindowMinimized);
        have->show();
        have->raise();
        have->activateWindow();
        Q_EMIT have->broughtForward();
        return true;
    }
    // A new workbook nobody has changed gives way to the file (and comes back if the file can't be read).
    const bool replacing = (isUntouchedBlank() || m_reopening) && how != OpenHow::NewWindow;
    // A window that shows a workbook, or is reading one, stays; the new one opens in a window of its own.
    if (how == OpenHow::NewWindow || ((m_loaded || m_loading) && !replacing)) {
        auto *w = new CrunchWindow();
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->m_closeIfOpenFails = true;
        w->setNotifier(m_notifier);
        w->m_promptForText = m_promptForText;
        w->m_forceTextPrompt = m_forceTextPrompt;
        w->m_anchorHooks = m_anchorHooks;
        m_forceTextPrompt = false;
        w->show();
        if (w->openFile(path, how == OpenHow::NewWindow ? OpenHow::Normal : how)) return true;
        w->close();   // (it was made for this file: a window with nothing in it is not left)
        return false;
    }
    // a text file of values: how it is read is asked first (the window that opens it asks)
    std::shared_ptr<const jp::office::sheet::CsvImport> csv;
    if ((m_promptForText && jp::office::sheet::isDelimitedName(path)) || m_forceTextPrompt) {
        m_forceTextPrompt = false;
        if (!askTextImport(path, &csv)) return false;
    }
    // Open and Repair: what can be read of a damaged package is made into a package (kept in a file of its own for the reader), and what was left out is said when it is open.
    QString loadPath = path;
    if (how == OpenHow::Repair) {
        QByteArray bytes;
        QString why;
        office::RepairReport report;
        QByteArray fixed;
        const bool binary = QFileInfo(path).suffix().compare(QLatin1String("xls"), Qt::CaseInsensitive) == 0;
        QStringList notes;
        if (office::readWholeFile(path, &bytes, &why)) {
            if (binary) fixed = office::sheet::salvageXlsStream(bytes, office::sheet::Limits(), &notes, &why);
            else fixed = office::repairPackage(bytes, &why, &report);
        }
        // (the file is removed when this object goes, on every path)
        auto temp = std::make_unique<QTemporaryFile>(QDir::temp().filePath(binary ? QStringLiteral("jeffcrunch-repair-XXXXXX.xls") : QStringLiteral("jeffcrunch-repair-XXXXXX.xlsx")));
        if (fixed.isEmpty() || !temp->open() || temp->write(fixed) != fixed.size()) {
            if (!fixed.isEmpty()) why = temp->errorString();
            notice(tr("Open and Repair"), tr("%1 can't repair \"%2\".\n%3").arg(appInfo().name, name, why));
            return false;
        }
        temp->close();
        loadPath = temp->fileName();
        m_repairTemp = std::move(temp);
        m_lastRepairFile = loadPath;
        m_repairFile = loadPath;
        m_repairName = name;
        m_repairNote = binary ? notes.join(QLatin1Char('\n')) : office::repairSummary(report);
    }
    if (replacing) dropWorkbook();
    setReadOnly(how == OpenHow::ReadOnly);
    m_openHow = how;
    m_personalInfoRemoved = false;
    m_editedProps.reset();
    m_openAsCopy = how == OpenHow::Copy;
    m_path = loadPath;
    m_loading = true;
    m_haveFirstScreen = m_haveCalc = false;
    m_calculating = false;
    m_values.clear();
    m_longestBatchMicros = m_batches = m_longestBatchItems = 0;
    m_loadProblem.clear();
    // (a file that is protected with a password is refused by the read, which says so: the window asks for the password then, and reads again)
    m_loader.start(loadPath, csv);
    setStatus(tr("Opening %1…").arg(name));
    updateTitle();
    hideFileScreen();
    Settings::get().addRecentFile(path);
    telemetry::count(QStringLiteral("file.open.") + crunch::fileKind(path));
    return true;
}

bool CrunchWindow::askFilePasswordAgain(bool wrong)
{
    FilePasswordRequest req;
    req.file = QFileInfo(m_path).fileName();
    req.wrong = wrong;
    if (m_anchorHooks.filePasswordAsker) {
        if (!m_anchorHooks.filePasswordAsker(&req)) return false;
    } else {
        QInputDialog dialog(this);
        dialog.setWindowTitle(tr("Password"));
        dialog.setLabelText(wrong ? tr("The password is not correct.\n\nThis file is protected with a password. Password:") : tr("This file is protected with a password. Password:"));
        dialog.setTextEchoMode(QLineEdit::Password);
        dialog.setOkButtonText(tr("Open"));
        dialog.setCancelButtonText(tr("Cancel"));
        if (dialog.exec() != QDialog::Accepted) return false;
        req.password = dialog.textValue();
        dialog.setTextValue(QString());
    }
    jp::crypto::wipe(m_pendingPassword);
    m_pendingPassword = req.password;
    jp::crypto::wipe(req.password);
    m_loading = true;
    setStatus(tr("Checking the password…"));
    m_loader.start(m_path, nullptr, m_pendingPassword);
    return true;
}

void CrunchWindow::forgetFilePassword()
{
    jp::crypto::wipe(m_filePassword);
    jp::crypto::wipe(m_pendingPassword);
    m_fileProtected = m_diskProtected = m_protectionChanged = false;
    m_diskDigest.clear();
}

void CrunchWindow::onFirstScreen()
{
    cancelPageBreaks();
    dropLayoutJob();
    m_loaded = m_loader.result();
    if (!m_loaded || !m_loaded->book || !m_loaded->preview) return;
    m_haveFirstScreen = true;
    openObjects();
    m_views.clear();
    m_sheet = m_loaded->previewSheet;
    fillTabs();
    showSheet(m_sheet, false);
    // (the keys go to the grid: a window that opens with the focus on its File button, the first in the tab order, would take a person's typing as presses of it)
    if (!m_fileScreen->isVisible() && (!focusWidget() || qobject_cast<QAbstractButton *>(focusWidget()))) m_view->setFocus(Qt::OtherFocusReason);
    if (!m_blank) setStatus(tr("Loading the rest of %1…").arg(QFileInfo(m_path).fileName()));
    updateTitle();
    updateBars();
    Q_EMIT firstScreenShown();
}

void CrunchWindow::onProgress(int sheet, qint64 cells)
{
    if (m_blank || !m_loaded || !m_loaded->book || sheet < 0 || sheet >= m_loaded->book->info().sheets.size()) return;
    setStatus(tr("Reading %1 (%2 cells)…").arg(m_loaded->book->info().sheets[sheet].name, QLocale().toString(cells)));
}

void CrunchWindow::onFinished(bool ok, const QString &message)
{
    m_loading = false;
    cancelPageBreaks();
    dropLayoutJob();
    bool quietly = false;
    if (!ok) {
        // a file that needs a password: the person is asked (and asked again if it is not the one); one who cancels leaves nothing open and is told nothing
        const auto result = m_loader.result();
        if (result && result->passwordNeed != jp::office::sheet::PasswordNeed::None) {
            if (askFilePasswordAgain(result->passwordNeed == jp::office::sheet::PasswordNeed::Wrong)) return;
            quietly = true;
        }
        const QString name = m_repairName.isEmpty() ? QFileInfo(m_path).fileName() : m_repairName;
        m_openAsCopy = false;
        m_repairTemp.reset();
        m_repairFile.clear();
        m_repairName.clear();
        m_repairNote.clear();
        dropWorkbook();
        // (a window made for the file goes; any other has a new workbook: no window is left that cannot be changed)
        const bool goes = m_closeIfOpenFails;
        if (!goes) newBlank();
        else setStatus(tr("Ready"));
        updateTitle();
        if (!quietly) notice(tr("Open"), tr("%1 can't open \"%2\".\n%3").arg(appInfo().name, name, message));
        Q_EMIT loadFinished(false);
        if (goes) close();
        return;
    }
    m_loaded = m_loader.result();
    // (a file that was protected is saved protected again, with the password that opened it, which is kept only while the workbook is open)
    jp::crypto::wipe(m_filePassword);
    m_fileProtected = m_diskProtected = m_loaded->protectedFile;
    m_filePassword = m_fileProtected && !m_loaded->defaultPasswordOnly ? m_pendingPassword : QString();
    m_diskDigest = m_loaded->diskDigest;
    jp::crypto::wipe(m_pendingPassword);
    m_protectionChanged = false;
    m_haveCalc = true;
    m_refreshOnOpenPending = true;
    // (a workbook read from a file of another form: that file is the one never written, whatever the workbook is saved as)
    m_originalPath = m_loaded->book->isConverted() ? m_path : QString();
    // (opened as a copy: the workbook has no file of its own, so Save asks where; a workbook read from another form never writes its file, and has no need of this)
    if (std::exchange(m_openAsCopy, false) && !m_loaded->book->isConverted()) {
        m_untitled = tr("Copy of %1").arg(QFileInfo(m_path).completeBaseName());
        m_path.clear();
        updateTitle();
    }
    // (a repaired workbook is not the file it was repaired from, and its package was only kept for the reader: Save asks for a name)
    QString repairNote, repairedName;
    if (!m_repairFile.isEmpty()) {
        m_repairTemp.reset();
        m_repairFile.clear();
        repairedName = std::exchange(m_repairName, QString());
        repairNote = std::exchange(m_repairNote, QString());
        m_untitled = tr("Repaired %1").arg(QFileInfo(repairedName).completeBaseName());
        m_path.clear();
        m_originalPath.clear();
        updateTitle();
    }
    // A workbook marked as final opens for reading only (the Info page lets it be edited again).
    if (m_openHow == OpenHow::Normal && !m_blank && repairedName.isEmpty() && !m_path.isEmpty() && m_loaded->book && office::isMarkedFinal(office::readPackageProps(m_loaded->book->package()).custom)) {
        setReadOnly(true);
    }
    createEditing();
    if (m_objects && m_loaded->calc) m_objects->setCells(m_loaded->calc.get(), &m_values);
    for (int i = 0; i < m_tabs->count(); ++i) m_tabs->setTabEnabled(i, true);
    // the stripes the sheets ask for, now that every sheet is read
    refreshStripes();
    // the sheet read whole takes the place of its first rows (what the person did with the view stays)
    showSheet(m_sheet, true);
    m_loaded->preview.reset();
    m_loadProblem = m_loaded->problem;
    // (the worker goes on to work the formulas out, and tells what it has as it has it: onCalcBatch)
    m_calculating = true;
    setStatus(m_blank ? tr("Ready") : tr("Calculating…"));
    if (!m_loadProblem.isEmpty()) notice(tr("Open"), m_loadProblem, true);
    // (what had to be changed is known once every sheet is read: sheet names the .xlsx form does not allow, and formulas that could not be converted)
    const QStringList changed = m_loaded->book->changedFeatures();
    if (!m_loaded->notRead.isEmpty() || !changed.isEmpty()) notice(tr("Open"), crunch::notReadNotice(m_loaded->notRead, changed));
    if (!repairNote.isEmpty()) notice(tr("Open and Repair"), tr("\"%1\" was opened as a repaired copy. Save it under a name of its own.\n\n%2").arg(repairedName, repairNote), false);
    Q_EMIT loadFinished(true);
    // (what was typed while the new workbook was getting ready)
    if (!m_pendingTyping.isEmpty() && m_session) {
        const QString typed = std::exchange(m_pendingTyping, QString());
        beginEdit(typed, CellEditor::Mode::Typing, false);
    }
}

// ---- the sheets

void CrunchWindow::fillTabs()
{
    m_switchingTabs = true;
    m_tabs->clearTabs();
    m_tabSheets.clear();
    if (m_loaded && m_loaded->book) {
        const auto &sheets = m_loaded->book->info().sheets;
        for (const int i : m_loaded->book->info().order) {
            if (i < 0 || i >= sheets.size() || sheets[i].deleted || sheets[i].state != SheetState::Visible) continue;   // (hidden and very hidden sheets are kept out)
            // (the name is shown as it is: a "&" in a tab's text makes the next letter a key, and a name that has one is shown without it, unless it is doubled)
            const int tab = m_tabs->addTab(QString(sheets[i].name).replace(QLatin1Char('&'), QStringLiteral("&&")));
            m_tabs->setTabData(tab, sheets[i].name);
            m_tabSheets.append(i);
            m_tabs->setTabEnabled(tab, m_haveCalc || i == m_sheet);
            if (i == m_sheet) m_tabs->setCurrentIndex(tab);
        }
        m_tabs->selectOnly(m_tabs->currentIndex());
        refreshStripes();
    }
    m_sheetTabRow->setVisible(!m_tabSheets.isEmpty());
    m_switchingTabs = false;
}

void CrunchWindow::showSheet(int sheetIndex, bool keepView)
{
    ++m_sheetShows;
    if (!m_loaded || !m_loaded->book) return;
    if (!keepView) m_copy.reset();   // (a copy being made is of the sheet that was shown)
    if (m_layoutSheet != sheetIndex) dropLayoutJob();   // (the pages held are another sheet's)
    grid::GridData d;
    d.styles = &m_loaded->book->styles();
    d.strings = &m_loaded->book->sharedStrings();
    d.dateSystem = m_loaded->book->info().dateSystem;
    d.sheetIndex = sheetIndex;
    if (m_haveCalc && m_loaded->calc) {   // (the worker puts the calculation there while it reads: not looked at before the read is over)
        d.sheet = &m_loaded->calc->sheet(sheetIndex);
        // (the other sheets and the names of all: the rules of conditional formatting have formulas that read them)
        const int count = std::min<int>(m_loaded->calc->sheetCount(), int(m_loaded->book->info().sheets.size()));
        for (int i = 0; i < count; ++i) {
            d.sheets.push_back(&m_loaded->calc->sheet(i));
            d.sheetNames.append(m_loaded->book->info().sheets[i].name);
        }
        d.values = &m_values;
        if (size_t(sheetIndex) < m_loaded->geometry.size()) d.geometry = m_loaded->geometry[size_t(sheetIndex)];
    } else if (m_loaded->preview && sheetIndex == m_loaded->previewSheet) {
        d.sheet = m_loaded->preview.get();
        d.geometry = m_loaded->previewGeometry;
    } else {
        d.sheet = &m_emptySheet;
    }
    m_showingSheet = true;
    m_view->setData(d, keepView);
    if (!keepView && m_views.contains(sheetIndex)) m_view->setViewState(m_views.value(sheetIndex));
    m_showingSheet = false;
    afterSheetShown(sheetIndex, false);
}

// What the window shows beside the grid for the sheet in view. `inPlace`: the grid kept its settings and was moved by an edit of rows or columns, not given the sheet again (the formulas
// it shows and the colors of the buttons are as they were).
void CrunchWindow::afterSheetShown(int sheetIndex, bool inPlace)
{
    const jp::office::sheet::Sheet *shown = m_view->sheet().data().sheet;
    // (which way the sheet reads is what the person left it at, not the file's alone)
    if (m_session && sheetIndex >= 0 && sheetIndex < m_session->pages().sheetCount()) m_view->setRightToLeft(m_session->pages().pane(sheetIndex).rightToLeft);
    if (m_session && sheetIndex >= 0 && sheetIndex < m_session->pages().sheetCount()) m_view->setShowZeros(m_session->pages().pane(sheetIndex).showZeros);
    if (QAction *rtl = act("view.rightToLeft")) rtl->setChecked(m_view->sheet().rightToLeft());
    updateCondActions();
    if (!inPlace) {
        m_view->setShowFormulas(act("view.formulas")->isChecked());
        // (the colors the buttons of the ribbon offer are the workbook's theme)
        if (m_fillButton) m_fillButton->setScheme(workbookColors());
        if (m_fontColorButton) m_fontColorButton->setScheme(workbookColors());
    }
    prepareObjects(sheetIndex);
    refreshOutline();
    act("view.gridlines")->setChecked(m_view->showGridlines());
    act("view.headings")->setChecked(m_view->showHeadings());
    updateFilterButtons();
    showViewKind(shown ? (m_session ? m_session->pages().viewKind(sheetIndex) : int(shown->view.kind)) : 0);
    updatePageActions();
    refreshAnchors();
    scheduleAutoHeights();
    updateBars();
}

void CrunchWindow::rememberView()
{
    if (m_sheet >= 0) m_views.insert(m_sheet, m_view->viewState());
    syncPanes();   // (a sheet that is left keeps the scroll and the cursor it had)
}

void CrunchWindow::setActiveSheet(int sheetIndex)
{
    if (!m_loaded || !m_loaded->book || sheetIndex == m_sheet || sheetIndex < 0 || sheetIndex >= m_loaded->book->info().sheets.size()) return;
    if (!m_haveCalc && sheetIndex != m_loaded->previewSheet) return;   // (the other sheets are not read yet)
    hideFillOptions();
    const bool keepEdit = keepEditOnSheetChange(sheetIndex);
    if (!keepEdit && !commitEdit()) {
        // (the entry could not be finished: the tab goes back to the sheet it is for)
        const int back = m_tabSheets.indexOf(m_sheet);
        if (back >= 0 && m_tabs->currentIndex() != back) {
            m_switchingTabs = true;
            m_tabs->setCurrentIndex(back);
            m_switchingTabs = false;
        }
        return;
    }
    rememberView();
    m_sheet = sheetIndex;
    const int tab = m_tabSheets.indexOf(sheetIndex);
    if (tab >= 0 && m_tabs->currentIndex() != tab) {
        m_switchingTabs = true;
        m_tabs->setCurrentIndex(tab);
        m_switchingTabs = false;
    }
    showSheet(sheetIndex, false);
    refreshCheckMarks();
    if (keepEdit && m_edit.active) {
        // (the entry goes on: its cell's line is over its own sheet only, the formula bar is where it is typed while another sheet is shown)
        if (m_sheet != m_edit.sheet) {
            if (m_point.line == m_cellEditor) m_point.homeLine = m_cellEditor;
            m_point.line = m_formulaBar;
        } else if (m_point.homeLine) {
            m_point.line = m_point.homeLine;
            m_point.homeLine.clear();
        }
        placeEditor();
        editLine()->setFocus(Qt::OtherFocusReason);
        updatePointViews();
        return;
    }
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
}

// ---- the calculation, on the worker

// Esc: the worker stops (what it has worked out is told, and is shown; the rest keeps the result the file stored).
bool CrunchWindow::stopCalculation()
{
    m_loader.stopCalculation();
    // a save that was waiting for the calculation is not made later, when something else finishes one: it is given up, and the person is told
    const bool saveWaiting = !m_saveAfterCalculation.isNull();
    m_saveAfterCalculation.clear();
    m_copyAfterCalculation = false;
    if (m_calculating || saveWaiting) {
        m_calculating = false;
        setStatus(saveWaiting ? tr("The calculation was stopped, so the save was cancelled. Save again when you want to.") : tr("Ready"));
    }
    // (a close that was waiting for that save asks again)
    if (saveWaiting && m_closeAfterSave) {
        m_closeAfterSave = false;
        close();
    }
    return saveWaiting;
}

// A batch of what the calculation has worked out: put in the store, and what of it is on the screen is drawn again.
void CrunchWindow::onCalcBatch(const std::shared_ptr<crunch::CalcBatch> &batch)
{
    if (!m_loaded || !batch) return;
    const qint64 cpu0 = jp::threadCpuMicros();
    m_longestBatchItems = std::max<qint64>(m_longestBatchItems, qint64(batch->items.size()));
    // (the items name cells by the ids of their rows and columns, which edits of rows and columns leave as they are: what was in flight when one was made lands on the right cell; those for cells that
    // an edit moved one by one after they were worked out are let go of, and the formulas are worked out again)
    if (m_loaded->calc) m_loaded->calc->valuesTaken(qint64(batch->items.size()));   // (what was told and is not taken is in flight: Calculation::resultsInFlight)
    if (m_session) m_session->dropStale(&batch->items);
    if (m_session) m_session->dropRemovedSheets(&batch->items);   // (results for a sheet that Undo took away while it was being worked out)
    if (m_session) m_session->noteResults(batch->items);   // (before the store has them: it says what the engine had before)
    m_values.apply(batch->items);
    // (the grid and the objects want places, as the cells are now)
    const std::vector<jp::grid::ValueStore::Item> where = m_session ? m_session->placed(batch->items) : std::vector<jp::grid::ValueStore::Item>();
    const std::vector<jp::grid::ValueStore::Item> &shown = m_session ? where : batch->items;
    if (m_objects) m_objects->cellsChanged(shown);
    m_workerCpuMicros = batch->workerCpuMicros;
    const bool active = m_view->valuesChanged(m_sheet, shown);
    if (batch->last && m_session) m_session->pruneStamps(batch->epochs);
    if (m_viewKind == 2 && m_pageView && m_layoutPages) m_pageView->contentsChanged();   // (the pages show the values the calculation gave)
    if (active) updateBars();
    ++m_batches;
    if (batch->last) {
        followCalcBatch(*batch);
        // (a calculation that was stopped has been said to be: only one that finished says so now; and one that has not worked out all the changes made since is not over)
        if (!batch->stopped && m_calculating && batch->epoch >= m_loader.changesMade()) {
            m_calculating = false;
            if (m_session) m_session->calculationFinished();
            if (!m_saving) setStatus(tr("Ready"));
            updateBars();
            m_longestBatchMicros = std::max(m_longestBatchMicros, jp::threadCpuMicros() - cpu0);
            Q_EMIT calculationFinished();
            if (m_refreshOnOpenPending && m_session) {
                // (a table whose data says to be read again when the file opens is: one step, which the person can undo; it waits for the first screen to be drawn)
                m_refreshOnOpenPending = false;
                QPointer<CrunchWindow> self(this);
                QTimer::singleShot(0, this, [self] {
                    if (!self || !self->m_session) return;
                    const auto refreshed = self->m_session->refreshPivotsOnOpen();
                    if (!refreshed.problem.isEmpty()) self->setStatus(refreshed.problem.section(QLatin1Char('\n'), 0, 0));
                });
            }
            if (!m_saveAfterCalculation.isNull()) {
                const QString path = m_saveAfterCalculation;
                m_saveAfterCalculation.clear();
                if (std::exchange(m_copyAfterCalculation, false)) startCopy(path);
                else startSave(path);
            }
            return;
        }
    } else if (m_calculating) {
        setStatus(tr("Calculating… %1%").arg(batch->percent));
    }
    m_longestBatchMicros = std::max(m_longestBatchMicros, jp::threadCpuMicros() - cpu0);
}

} // namespace jp
