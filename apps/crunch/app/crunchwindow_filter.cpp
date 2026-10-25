// The filter on the window: the buttons on the header row, Filter (on and off), Clear and Reapply, the drop-down of a button with the values of its column, and the questions the filters
// by condition ask. The conditions are worked out by the edit session a slice at a time; the rows they hide are kept apart from the rows a person hid.
#include "app/crunchwindow.h"

#include "app/crunchinfo.h"
#include "app/filterdialogs.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QGuiApplication>
#include <QScreen>

namespace jp {

using jp::office::sheet::AutoFilter;
using jp::office::sheet::CellRange;
using jp::office::sheet::FilterColumn;

namespace {
// The cells a filter is made at once without a timer.
constexpr qint64 kSmallFilter = 4000;
}

void CrunchWindow::createFilterActions()
{
    mk("filter.toggle", tr("Filter"), "filter", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L), [this] { toggleFilter(); }, true);
    mk("filter.clear", tr("Clear"), "filter-x", QKeySequence(), [this] {
        if (m_session && m_loaded && currentFilter().valid()) applyFilter(m_session->filterCleared(m_sheet), tr("Clearing the filter"));
    });
    mk("filter.reapply", tr("Reapply"), "refresh-cw", QKeySequence(), [this] {
        if (m_session && m_loaded && currentFilter().valid()) applyFilter(currentFilter(), tr("Applying the filter again"));
    });
    for (const char *id : {"filter.toggle", "filter.clear", "filter.reapply"}) m_opsActions << QString::fromLatin1(id);
    m_filterTimer.setInterval(0);
    m_listTimer.setInterval(0);
    connect(&m_filterTimer, &QTimer::timeout, this, [this] { stepFilter(); });
    connect(&m_listTimer, &QTimer::timeout, this, [this] { stepList(); });
    connect(m_view, &grid::GridView::headerButtonClicked, this, [this](qint64 row, qint64 col) { showFilterPopup(row, col); });
    updateEditActions();
}

AutoFilter CrunchWindow::currentFilter() const
{
    if (m_session && m_loaded && m_loaded->calc && m_haveCalc) return m_loaded->calc->autoFilterOf(m_sheet);
    if (m_loaded && m_loaded->preview && m_sheet == m_loaded->previewSheet) return m_loaded->preview->autoFilter;
    return AutoFilter();
}

// The buttons of the filter are on its header row (those whose column has no button of its own turned off), and the command shows whether the sheet has a filter.
void CrunchWindow::updateFilterButtons()
{
    const AutoFilter f = currentFilter();
    std::vector<grid::GridView::HeaderButton> buttons;
    if (f.valid() && !m_chartSheetShown) {
        const quint32 width = std::min<quint32>(f.ref.col2 - f.ref.col1 + 1, grid::GridView::kMaxHeaderButtons);
        for (quint32 c = 0; c < width; ++c) {
            const FilterColumn *fc = f.columnAt(c);
            if (fc && fc->hideButton) continue;
            grid::GridView::HeaderButton b;
            b.row = f.ref.row1;
            b.col = f.ref.col1 + c;
            b.filtered = fc && fc->kind != FilterColumn::Kind::None;
            buttons.push_back(b);
        }
    }
    m_view->setHeaderButtons(std::move(buttons));
    if (QAction *a = act("filter.toggle")) {
        const QSignalBlocker block(a);
        a->setChecked(f.valid());
    }
    if (QAction *a = act("filter.clear")) a->setToolTip(tr("Take the conditions off every column (the buttons stay)"));
}

// Filter: on over the cells (the block around the cell with the cursor if one is selected), or off.
void CrunchWindow::toggleFilter()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || m_filterPlan) return;
    if (currentFilter().valid()) {
        applyFilter(AutoFilter(), tr("Turning the filter off"));
        return;
    }
    const grid::Selection &sel = m_view->selection();
    CellRange range = selectedRange();
    const bool oneCell = range.row1 == range.row2 && range.col1 == range.col2;
    if (oneCell && sel.whole == grid::Selection::Whole::None) {
        bool truncated = false;
        range = crunch::currentRegion(*m_session, m_sheet, range.row1, range.col1, &truncated);
        if (truncated) {
            notice(tr("Filter"), tr("The block of data around this cell is too big to find its edges quickly. Select the cells to filter."));
            return;
        }
        if (range.cellCount() <= 1) {
            notice(tr("Filter"), tr("There is nothing to filter here. Select a cell in the data, or select the cells to filter."));
            return;
        }
    } else if (sel.whole != grid::Selection::Whole::None) {
        const jp::office::sheet::Sheet &s = m_session->calculation()->sheet(m_sheet);
        if (s.extentRows == 0) {
            notice(tr("Filter"), tr("There is nothing to filter here."));
            return;
        }
        range.row2 = std::min(range.row2, s.extentRows - 1);
        range.col2 = std::min(range.col2, s.extentCols - 1);
    }
    AutoFilter f;
    f.ref = range;
    applyFilter(f, tr("Turning the filter on"));
}

bool CrunchWindow::applyFilter(const AutoFilter &filter, const QString &text)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || m_filterPlan) return false;
    QString why;
    m_filterPlan = m_session->planFilter(m_sheet, filter, &why);
    if (!m_filterPlan) {
        notice(tr("Filter"), why);
        Q_EMIT filterFinished(false);
        return false;
    }
    m_filterText = text;
    setStatus(tr("Filtering…"));
    const qint64 rows = filter.ref.valid ? qint64(filter.ref.row2) - filter.ref.row1 : 0;
    const qint64 cost = rows * qint64(filter.conditionColumns().size());   // (a filter with no conditions has nothing to work out)
    if (cost <= kSmallFilter) {
        m_session->prepareFilter(*m_filterPlan, 0);
        finishFilter(true);
        return true;
    }
    m_filterTimer.start();
    return true;
}

void CrunchWindow::stepFilter()
{
    if (!m_filterPlan) {
        m_filterTimer.stop();
        return;
    }
    if (!m_session) {
        cancelFilter();
        return;
    }
    m_filterTuner.begin();
    const qint64 budget = m_filterTuner.budget();
    const bool done = m_session->prepareFilter(*m_filterPlan, budget);
    m_filterTuner.end(budget);
    if (done) finishFilter(true);
}

void CrunchWindow::cancelFilter()
{
    if (!m_filterPlan) return;
    m_filterTimer.stop();
    m_filterPlan.reset();
    setStatus(tr("The filter was stopped."));
    Q_EMIT filterFinished(false);
}

void CrunchWindow::finishFilter(bool done)
{
    m_filterTimer.stop();
    const auto plan = std::move(m_filterPlan);
    m_filterPlan.reset();
    bool made = false;
    if (plan && m_session && done) {
        const qint64 before = m_loader.changesMade();
        const crunch::EditSession::Outcome o = doStructural([&] { return m_session->applyFilter(*plan, m_filterText); });
        if (!o.ok) {
            if (!o.problem.isEmpty()) notice(tr("Filter"), o.problem);
        } else {
            made = true;
            if (m_loader.changesMade() != before) m_calculating = true;
        }
    }
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
    Q_EMIT filterFinished(made);
}

// ---- the drop-down

void CrunchWindow::showFilterPopup(qint64 row, qint64 col)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !m_session || !m_loaded || !m_haveCalc || m_filterPlan) return;
    const AutoFilter f = currentFilter();
    if (!f.valid() || row != qint64(f.ref.row1) || col < qint64(f.ref.col1) || col > qint64(f.ref.col2)) return;
    if (m_filterPopup) m_filterPopup->close();
    const quint32 column = quint32(col - f.ref.col1);
    FilterPopup::Input input;
    const QString head = m_session->displayText(m_sheet, f.ref.row1, quint32(col)).trimmed().left(60);
    input.columnName = head.isEmpty() ? tr("Column %1").arg(jp::office::sheet::columnName(quint32(col))) : head;
    if (const FilterColumn *fc = f.columnAt(column)) {
        input.filtered = fc->kind != FilterColumn::Kind::None;
        input.current = *fc;
        if (input.current.kind == FilterColumn::Kind::Color && !input.current.color.isSet()) input.current.color = m_session->colorOfCondition(input.current);
    }
    const jp::office::sheet::Styles &st = m_loaded->book->styles();
    input.toColor = [&st](const jp::office::sheet::ColorSpec &c) { return QColor::fromRgb(st.resolve(c, 0x000000)); };
    auto *popup = new FilterPopup(input, this);
    popup->setAttribute(Qt::WA_DeleteOnClose);
    m_filterPopup = popup;
    popup->setLoading(true);
    // the column's values are listed a slice at a time
    m_listRun = std::make_unique<crunch::FilterListRun>(m_session.get(), m_sheet, f, column);
    m_listChanges = m_loader.changesMade();
    m_listSteps = m_session->undoStack()->index();
    m_listTimer.start();
    // (a popup that is gone takes the listing with it, unless another has been made since: the listing is the other's then)
    const QObject *raw = popup;
    connect(popup, &QObject::destroyed, this, [this, raw] {
        if (m_filterPopup && m_filterPopup.data() != raw) return;
        m_listTimer.stop();
        m_listRun.reset();
    });
    const int sheet = m_sheet;
    connect(popup, &FilterPopup::sortRequested, this, [this, f, column](bool descending) {
        crunch::SortSpec spec;
        crunch::SortLevel level;
        level.key = int(column);
        level.descending = descending;
        spec.levels.append(level);
        spec.hasHeader = true;
        sortRange(f.ref, spec);
    });
    connect(popup, &FilterPopup::sortByColorRequested, this, [this, f, column](bool font, const jp::office::sheet::ColorSpec &color) {
        crunch::SortSpec spec;
        crunch::SortLevel level;
        level.key = int(column);
        level.by = font ? crunch::SortLevel::By::FontColor : crunch::SortLevel::By::CellColor;
        level.color = color;
        level.onTop = true;
        spec.levels.append(level);
        spec.hasHeader = true;
        sortRange(f.ref, spec);
    });
    connect(popup, &FilterPopup::clearRequested, this, [this, sheet, column] {
        if (m_session && m_sheet == sheet) applyFilter(m_session->filterWith(sheet, column, FilterColumn()), tr("Clearing the filter from a column"));
    });
    connect(popup, &FilterPopup::conditionChosen, this, [this, sheet, column](const FilterColumn &c) {
        if (m_session && m_sheet == sheet) applyFilter(m_session->filterWith(sheet, column, c), tr("Filtering"));
    });
    connect(popup, &FilterPopup::askRequested, this, [this, column, popup](FilterAsk what) {
        popup->close();
        askFilter(what, column);
    });
    const QRect button = m_view->headerButtonRect(row, col);
    const QPoint at = m_view->viewport()->mapToGlobal(QPoint(button.isEmpty() ? m_view->cellRect(row, col).left() : button.left(), (button.isEmpty() ? m_view->cellRect(row, col).bottom() : button.bottom()) + 1));
    popup->popup(at);
}

void CrunchWindow::stepList()
{
    if (!m_listRun || !m_filterPopup) {
        m_listTimer.stop();
        return;
    }
    if (!m_session || m_loader.changesMade() != m_listChanges || m_session->undoStack()->index() != m_listSteps) {
        // (the cells were changed while the values were being listed: the list is of the cells as they were, so it is given up)
        m_listTimer.stop();
        m_listRun.reset();
        m_filterPopup->close();
        return;
    }
    m_listTuner.begin();
    const qint64 budget = m_listTuner.budget();
    const bool done = m_listRun->step(budget);
    m_listTuner.end(budget);
    if (done) listFinished();
}

void CrunchWindow::listFinished()
{
    m_listTimer.stop();
    if (m_filterPopup && m_listRun) {
        m_filterPopup->setList(m_listRun->list());
        m_filterPopup->setLoading(false);
    }
    m_listRun.reset();
}

// Equals…, Between…, Top 10…, Custom Filter…: the person is asked, and the condition is put on the column.
void CrunchWindow::askFilter(FilterAsk what, quint32 column)
{
    if (!m_session || !m_loaded) return;
    const AutoFilter f = currentFilter();
    if (!f.valid() || column > f.ref.col2 - f.ref.col1) return;
    const quint32 col = f.ref.col1 + column;
    const QString head = m_session->displayText(m_sheet, f.ref.row1, col).trimmed().left(60);
    const QString name = head.isEmpty() ? tr("Column %1").arg(jp::office::sheet::columnName(col)) : head;
    const FilterColumn *current = f.columnAt(column);
    // a column of dates asks for days
    bool dates = false;
    {
        crunch::FilterListRun probe(m_session.get(), m_sheet, f, column);
        probe.step(0);
        dates = probe.list().kind == crunch::FilterList::Kind::Date;
    }
    FilterColumn chosen;
    if (m_filterAsker) {
        if (!m_filterAsker(what, name, dates, current, &chosen)) return;
    } else if (what == FilterAsk::Top10) {
        TopFilterDialog dialog(name, current, this);
        if (dialog.exec() != QDialog::Accepted) return;
        chosen = dialog.result();
    } else {
        CustomFilterDialog dialog(name, dates, m_loaded->book->info().dateSystem, what, current, this);
        if (dialog.exec() != QDialog::Accepted) return;
        chosen = dialog.result();
    }
    applyFilter(m_session->filterWith(m_sheet, column, chosen), tr("Filtering"));
}

} // namespace jp
