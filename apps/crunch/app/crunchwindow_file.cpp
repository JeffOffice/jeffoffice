// Opening a workbook in JeffCrunch's window: the first rows at once, the sheets, and the calculation done in slices.

#include "app/crunchwindow.h"

#include "app/oldformat.h"
#include "app/settings.h"
#include "app/sheettabs.h"
#include "app/telemetry.h"
#include "core/sheetobjects.h"
#include "core/appinfo.h"
#include "core/threadtime.h"

#include <QApplication>
#include <QFileInfo>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>

#include <algorithm>

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

bool CrunchWindow::openFile(const QString &path)
{
    const QFileInfo info(path);
    const QString name = info.fileName();
    if (!info.exists() || !info.isFile()) {
        notice(tr("Open"), tr("%1 can't open \"%2\".\nThe file isn't there.").arg(appInfo().name, name));
        return false;
    }
    // A file that is open in a window is not opened again: that window is brought to the front
    if (CrunchWindow *have = windowWith(path)) {
        have->setWindowState(have->windowState() & ~Qt::WindowMinimized);
        have->show();
        have->raise();
        have->activateWindow();
        Q_EMIT have->broughtForward();
        return true;
    }
    // A window that shows a workbook, or is reading one, stays; the new one opens in a window of its own.
    if (m_loaded || m_loading) {
        auto *w = new CrunchWindow();
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->setNotifier(m_notifier);
        w->show();
        return w->openFile(path);
    }
    m_path = path;
    m_loading = true;
    m_haveFirstScreen = m_haveCalc = false;
    m_calculating = false;
    m_values.clear();
    m_longestBatchMicros = m_batches = m_longestBatchItems = 0;
    m_loadProblem.clear();
    m_loader.start(path);
    setStatus(tr("Opening %1…").arg(name));
    updateTitle();
    hideFileScreen();
    Settings::get().addRecentFile(path);
    telemetry::count(QStringLiteral("file.open.") + crunch::fileKind(path));
    return true;
}

void CrunchWindow::onFirstScreen()
{
    m_loaded = m_loader.result();
    if (!m_loaded || !m_loaded->book || !m_loaded->preview) return;
    m_haveFirstScreen = true;
    openObjects();
    m_views.clear();
    m_sheet = m_loaded->previewSheet;
    fillTabs();
    showSheet(m_sheet, false);
    setStatus(tr("Loading the rest of %1…").arg(QFileInfo(m_path).fileName()));
    updateTitle();
    updateBars();
    Q_EMIT firstScreenShown();
}

void CrunchWindow::onProgress(int sheet, qint64 cells)
{
    if (!m_loaded || !m_loaded->book || sheet < 0 || sheet >= m_loaded->book->info().sheets.size()) return;
    setStatus(tr("Reading %1 (%2 cells)…").arg(m_loaded->book->info().sheets[sheet].name, QLocale().toString(cells)));
}

void CrunchWindow::onFinished(bool ok, const QString &message)
{
    m_loading = false;
    if (!ok) {
        const QString name = QFileInfo(m_path).fileName();
        m_path.clear();
        m_session.reset();
        m_loaded.reset();
        if (m_objects) m_objects->open(nullptr);   // (nothing opened: no object is left over the blank grid)
        m_view->setCellsEnabled(true);
        showBlank();
        fillTabs();
        setStatus(tr("Ready"));
        updateTitle();
        notice(tr("Open"), tr("%1 can't open \"%2\".\n%3").arg(appInfo().name, name, message));
        Q_EMIT loadFinished(false);
        return;
    }
    m_loaded = m_loader.result();
    m_haveCalc = true;
    // (a workbook read from a file of another form: that file is the one never written, whatever the workbook is saved as)
    m_originalPath = m_loaded->book->isConverted() ? m_path : QString();
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
    setStatus(tr("Calculating…"));
    if (!m_loadProblem.isEmpty()) notice(tr("Open"), m_loadProblem, true);
    // (what had to be changed is known once every sheet is read: sheet names the .xlsx form does not allow, and formulas that could not be converted)
    const QStringList changed = m_loaded->book->changedFeatures();
    if (!m_loaded->notRead.isEmpty() || !changed.isEmpty()) notice(tr("Open"), crunch::notReadNotice(m_loaded->notRead, changed));
    Q_EMIT loadFinished(true);
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
    if (!m_loaded || !m_loaded->book) return;
    if (!keepView) m_copy.reset();   // (a copy being made is of the sheet that was shown)
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
    m_view->setData(d, keepView);
    if (!keepView && m_views.contains(sheetIndex)) m_view->setViewState(m_views.value(sheetIndex));
    m_view->sheet().setShowFormulas(act("view.formulas")->isChecked());
    // (the colors the buttons of the ribbon offer are the workbook's theme)
    if (m_fillButton) m_fillButton->setScheme(workbookColors());
    if (m_fontColorButton) m_fontColorButton->setScheme(workbookColors());
    prepareObjects(sheetIndex);
    act("view.gridlines")->setChecked(m_view->showGridlines());
    act("view.headings")->setChecked(m_view->showHeadings());
    updatePageActions();
    updateBars();
}

void CrunchWindow::rememberView()
{
    if (m_sheet >= 0) m_views.insert(m_sheet, m_view->viewState());
}

void CrunchWindow::setActiveSheet(int sheetIndex)
{
    if (!m_loaded || !m_loaded->book || sheetIndex == m_sheet || sheetIndex < 0 || sheetIndex >= m_loaded->book->info().sheets.size()) return;
    if (!m_haveCalc && sheetIndex != m_loaded->previewSheet) return;   // (the other sheets are not read yet)
    if (!commitEdit()) {
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
    if (m_session && batch->generation < m_session->generation()) m_session->moveToNow(&batch->items, batch->generation);   // (made before cells were moved: they are named by where they were)
    if (m_session) m_session->dropRemovedSheets(&batch->items);   // (results for a sheet that Undo took away while it was being worked out)
    if (m_session) m_session->noteResults(batch->items);   // (before the store has them: it says what the engine had before)
    m_values.apply(batch->items);
    if (m_objects) m_objects->cellsChanged(batch->items);
    m_workerCpuMicros = batch->workerCpuMicros;
    const bool active = m_view->valuesChanged(m_sheet, batch->items);
    if (active) updateBars();
    ++m_batches;
    if (batch->last) {
        // (a calculation that was stopped has been said to be: only one that finished says so now; and one that has not worked out all the changes made since is not over)
        if (!batch->stopped && m_calculating && batch->epoch >= m_loader.changesMade()) {
            m_calculating = false;
            if (m_session) m_session->calculationFinished();
            if (!m_saving) setStatus(tr("Ready"));
            updateBars();
            m_longestBatchMicros = std::max(m_longestBatchMicros, jp::threadCpuMicros() - cpu0);
            Q_EMIT calculationFinished();
            if (!m_saveAfterCalculation.isNull()) {
                const QString path = m_saveAfterCalculation;
                m_saveAfterCalculation.clear();
                startSave(path);
            }
            return;
        }
    } else if (m_calculating) {
        setStatus(tr("Calculating… %1%").arg(batch->percent));
    }
    m_longestBatchMicros = std::max(m_longestBatchMicros, jp::threadCpuMicros() - cpu0);
}

} // namespace jp
