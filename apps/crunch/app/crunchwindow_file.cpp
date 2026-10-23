// Opening a workbook in JeffCrunch's window: the first rows at once, the sheets, and the calculation done in slices.

#include "app/crunchwindow.h"

#include "app/settings.h"
#include "app/sheettabs.h"
#include "app/telemetry.h"
#include "core/appinfo.h"

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
}

bool CrunchWindow::openFile(const QString &path)
{
    const QFileInfo info(path);
    const QString name = info.fileName();
    if (!info.exists() || !info.isFile()) {
        notice(tr("Open"), tr("%1 can't open \"%2\".\nThe file isn't there.").arg(appInfo().name, name));
        return false;
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
    m_loadProblem.clear();
    m_loader.start(path);
    setStatus(tr("Opening %1…").arg(name));
    updateTitle();
    hideFileScreen();
    Settings::get().addRecentFile(path);
    telemetry::count(QStringLiteral("file.open.") + info.suffix().toLower());
    return true;
}

void CrunchWindow::onFirstScreen()
{
    m_loaded = m_loader.result();
    if (!m_loaded || !m_loaded->book || !m_loaded->preview) return;
    m_haveFirstScreen = true;
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
        m_loaded.reset();
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
    for (int i = 0; i < m_tabs->count(); ++i) m_tabs->setTabEnabled(i, true);
    // the stripes the sheets ask for, now that every sheet is read
    const jp::office::sheet::Styles &styles = m_loaded->book->styles();
    for (int tab = 0; tab < m_tabSheets.size(); ++tab) {
        const auto &color = m_loaded->calc->sheet(m_tabSheets[tab]).tabColor;
        if (color.isSet()) m_tabs->setStripe(tab, QColor::fromRgb(styles.resolve(color, 0x808080)));
    }
    // the sheet read whole takes the place of its first rows (what the person did with the view stays)
    showSheet(m_sheet, true);
    m_loaded->preview.reset();
    m_loadProblem = m_loaded->problem;
    if (!m_loadProblem.isEmpty()) notice(tr("Open"), m_loadProblem, true);
    Q_EMIT loadFinished(true);
    startCalculation();
}

// ---- the sheets

void CrunchWindow::fillTabs()
{
    m_switchingTabs = true;
    m_tabs->clearTabs();
    m_tabSheets.clear();
    if (m_loaded && m_loaded->book) {
        const auto &sheets = m_loaded->book->info().sheets;
        for (int i = 0; i < sheets.size(); ++i) {
            if (sheets[i].state != SheetState::Visible) continue;   // (hidden and very hidden sheets are kept out)
            const int tab = m_tabs->addTab(sheets[i].name);
            m_tabSheets.append(i);
            m_tabs->setTabEnabled(tab, i == m_sheet);
            if (i == m_sheet) m_tabs->setCurrentIndex(tab);
        }
        if (m_loaded->preview && m_loaded->preview->tabColor.isSet() && m_sheet >= 0) {
            const int tab = m_tabSheets.indexOf(m_sheet);
            if (tab >= 0) m_tabs->setStripe(tab, QColor::fromRgb(m_loaded->book->styles().resolve(m_loaded->preview->tabColor, 0x808080)));
        }
    }
    m_tabs->setVisible(!m_tabSheets.isEmpty());
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
    if (m_loaded->calc && m_haveCalc) {
        d.sheet = &m_loaded->calc->sheet(sheetIndex);
        d.calc = m_loaded->calc.get();
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
    act("view.gridlines")->setChecked(m_view->showGridlines());
    act("view.headings")->setChecked(m_view->showHeadings());
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
    rememberView();
    m_sheet = sheetIndex;
    const int tab = m_tabSheets.indexOf(sheetIndex);
    if (tab >= 0 && m_tabs->currentIndex() != tab) {
        m_switchingTabs = true;
        m_tabs->setCurrentIndex(tab);
        m_switchingTabs = false;
    }
    showSheet(sheetIndex, false);
    if (m_loaded->book->info().sheets[sheetIndex].kind == SheetKind::Chartsheet) setStatus(tr("This is a chart sheet; JeffCrunch doesn't show charts yet."));
    else if (!m_calculating && !m_loading) setStatus(tr("Ready"));
}

// ---- the calculation, a few milliseconds at a time

void CrunchWindow::startCalculation()
{
    if (!m_loaded || !m_loaded->calc) return;
    m_calculating = true;
    m_stopCalc = false;
    m_budget = 20000;
    m_sinceRepaint.start();
    setStatus(tr("Calculating…"));
    m_calcTimer.start(0);
}

void CrunchWindow::stopCalculation()
{
    m_stopCalc = true;
    m_calcTimer.stop();
    if (m_calculating) {
        m_calculating = false;
        setStatus(tr("Ready"));
    }
}

void CrunchWindow::calculationStep()
{
    if (!m_loaded || !m_loaded->calc || !m_calculating) return;
    jp::office::sheet::Calculation *calc = m_loaded->calc.get();
    QElapsedTimer t;
    t.start();
    const jp::office::sheet::Calculation::Progress p = calc->recalculate(m_budget, &m_stopCalc);
    const qint64 ms = t.elapsed();
    // a slice of about six milliseconds: the work of the next is more when this one was quick, less when it was long
    if (ms < 3) m_budget = std::min<qint64>(m_budget * 2, 4000000);
    else if (ms > 10) m_budget = std::max<qint64>(m_budget / 2, 2000);
    const bool done = p.finished || p.cancelled;
    if (done || m_sinceRepaint.elapsed() > 250) {
        m_view->invalidateValues();
        updateBars();
        m_sinceRepaint.restart();
    }
    if (!done) {
        const qint64 total = std::max<qint64>(1, calc->formulaCount());
        setStatus(tr("Calculating… %1%").arg(std::max<qint64>(0, 100 - 100 * calc->dirtyCount() / total)));
        m_calcTimer.start(0);
        return;
    }
    m_calculating = false;
    setStatus(tr("Ready"));
    Q_EMIT calculationFinished();
}

} // namespace jp
