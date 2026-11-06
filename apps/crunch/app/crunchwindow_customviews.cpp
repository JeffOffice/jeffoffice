// Custom views: the View tab's command that keeps how the workbook looks (the sheet in front, the rows and columns hidden, the zoom, the frozen panes, the cell chosen in each sheet, the gridlines
// and headings, and the size of the window) under a name, and shows it again. The views are kept with the workbook (libs/office/sheet/customviews.h); making and deleting one is not a step of Undo,
// and showing one hides and shows rows and columns as one step.

#include "app/crunchwindow.h"

#include "app/customviewsdialog.h"

#include "core/editsession.h"
#include "grid/gridview.h"
#include "sheet/calculation.h"
#include "sheet/customviews.h"

#include <QAction>
#include <QUndoStack>

#include <algorithm>
#include <cmath>

namespace jp {

using jp::office::sheet::CustomView;
using jp::office::sheet::CustomViewSheet;
using Spans = QVector<QPair<quint32, quint32>>;

namespace {

constexpr int kTwipsPerPixel = 15;   // the file counts the window's size in twips (a 96-dot-per-inch pixel is 15)

}

void CrunchWindow::createCustomViewActions()
{
    mk(QStringLiteral("view.customViews"), tr("Custom Views"), "layout-list", QKeySequence(), [this] { customViews(); }, false, false);
    m_opsActions << QStringLiteral("view.customViews");
}

// How the workbook looks now, under a name.
CustomView CrunchWindow::captureCustomView(const QString &name)
{
    CustomView v;
    v.name = name;
    v.guid = jp::office::sheet::newCustomViewGuid();
    v.hasState = true;
    const jp::office::sheet::Calculation *calc = calculation();
    if (!m_session || !m_session->workbook() || !calc) return v;
    rememberView();
    const jp::office::sheet::WorkbookInfo &info = m_session->workbook()->info();
    if (m_sheet >= 0 && m_sheet < info.sheets.size()) {
        v.activeSheet = info.sheets[m_sheet].name;
        v.activeSheetId = info.sheets[m_sheet].sheetId;
    }
    v.windowWidth = width() * kTwipsPerPixel;
    v.windowHeight = height() * kTwipsPerPixel;
    v.xWindow = x() * kTwipsPerPixel;
    v.yWindow = y() * kTwipsPerPixel;
    v.gridlines = m_view->showGridlines();
    v.headings = m_view->showHeadings();
    v.formulaBar = act(QStringLiteral("view.formulaBar"))->isChecked();
    for (const int place : info.order) {
        if (place < 0 || place >= info.sheets.size() || info.sheets[place].deleted || info.sheets[place].kind != jp::office::sheet::SheetKind::Worksheet || place >= calc->sheetCount()) continue;
        CustomViewSheet s;
        s.name = info.sheets[place].name;
        const grid::GridView::ViewState st = place == m_sheet ? m_view->viewState() : m_views.value(place);
        const jp::office::sheet::SheetView &pane = m_session->pages().pane(place);
        if (st.valid) {
            s.zoom = std::clamp(int(std::lround(st.zoom * 100)), 10, 400);
            s.frozenRows = quint32(std::max<qint64>(0, st.frozenRows - st.skipRows));
            s.frozenCols = quint32(std::max<qint64>(0, st.frozenCols - st.skipCols));
            s.activeRow = quint32(std::max<qint64>(0, st.selection.activeRow));
            s.activeCol = quint32(std::max<qint64>(0, st.selection.activeCol));
        } else {
            s.zoom = std::clamp(m_session->pages().normalZoom(place), 10, 400);
            s.frozenRows = pane.frozenRows;
            s.frozenCols = pane.frozenCols;
            s.activeRow = pane.activeRow;
            s.activeCol = pane.activeCol;
        }
        s.frozen = s.frozenRows || s.frozenCols;
        Spans rows, cols;
        for (const jp::office::sheet::RowSpec &r : calc->rowSpecsIn(place, 0, jp::office::sheet::kMaxRows - 1))
            if (r.hidden) rows.append({r.row, r.row});
        for (const jp::office::sheet::ColumnSpan &c : calc->columnRuns(place, 0, jp::office::sheet::kMaxColumns - 1))
            if (c.hidden) cols.append({c.first, c.last});
        s.hiddenRows = jp::office::sheet::normalizedSpans(rows);
        s.hiddenCols = jp::office::sheet::normalizedSpans(cols);
        v.sheets.append(s);
    }
    return v;
}

void CrunchWindow::customViews()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || !m_haveCalc || m_sheet < 0 || !m_session->workbook()) return;
    CustomViewsRequest req;
    req.views = m_session->customViews();
    req.capture = [this](const QString &name) { return captureCustomView(name); };
    if (m_anchorHooks.customViewsAsker) {
        if (!m_anchorHooks.customViewsAsker(&req)) return;
    } else {
        CustomViewsDialog dialog(req, this);
        dialog.exec();
        req = dialog.result();
    }
    if (req.views != m_session->customViews()) {
        m_session->setCustomViews(req.views);
        setWindowModified(true);
    }
    if (!req.show.isEmpty()) showCustomView(req.show);
}

// Shows the view with this name: the sheet it had in front, the rows and columns hidden or shown in each of its sheets (one step of Undo), the zoom, panes, and cell chosen of each, and the
// window's size. A sheet the view does not know is left as it is; a sheet that cannot be changed (protected) keeps its rows and columns.
bool CrunchWindow::showCustomView(const QString &name)
{
    if (!m_session || !m_haveCalc || !m_session->workbook() || !calculation()) return false;
    const CustomView *found = nullptr;
    for (const CustomView &v : m_session->customViews())
        if (v.name.compare(name, Qt::CaseInsensitive) == 0) found = &v;
    if (!found) return false;
    const CustomView view = *found;
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return false;
    const jp::office::sheet::WorkbookInfo &info = m_session->workbook()->info();
    auto placeOf = [&](const QString &sheet) {
        for (const int place : info.order)
            if (place >= 0 && place < info.sheets.size() && !info.sheets[place].deleted && info.sheets[place].kind == jp::office::sheet::SheetKind::Worksheet && info.sheets[place].name == sheet) return place;
        return -1;
    };
    // the sheet in front first (leaving a sheet keeps the state it had; the states below are put in after)
    int target = placeOf(view.activeSheet);
    if (target < 0)
        for (const int place : info.order)
            if (place >= 0 && place < info.sheets.size() && !info.sheets[place].deleted && info.sheets[place].sheetId == view.activeSheetId) target = place;
    if (target >= 0 && target != m_sheet) setActiveSheet(target);
    rememberView();
    // rows and columns
    struct Change {
        int sheet;
        bool column, hide;
        quint32 first, last;
    };
    std::vector<Change> changes;
    const jp::office::sheet::Calculation *calc = calculation();
    for (const CustomViewSheet &s : view.sheets) {
        const int place = placeOf(s.name);
        if (place < 0 || place >= calc->sheetCount() || !m_session->canEditSheet(place, nullptr)) continue;
        Spans nowRows, nowCols;
        for (const jp::office::sheet::RowSpec &r : calc->rowSpecsIn(place, 0, jp::office::sheet::kMaxRows - 1))
            if (r.hidden) nowRows.append({r.row, r.row});
        for (const jp::office::sheet::ColumnSpan &c : calc->columnRuns(place, 0, jp::office::sheet::kMaxColumns - 1))
            if (c.hidden) nowCols.append({c.first, c.last});
        nowRows = jp::office::sheet::normalizedSpans(nowRows);
        nowCols = jp::office::sheet::normalizedSpans(nowCols);
        for (const auto &sp : jp::office::sheet::spansWithout(nowRows, s.hiddenRows)) changes.push_back({place, false, false, sp.first, sp.second});
        for (const auto &sp : jp::office::sheet::spansWithout(nowCols, s.hiddenCols)) changes.push_back({place, true, false, sp.first, sp.second});
        for (const auto &sp : jp::office::sheet::spansWithout(s.hiddenRows, nowRows)) changes.push_back({place, false, true, std::min<quint32>(sp.first, jp::office::sheet::kMaxRows - 1), std::min<quint32>(sp.second, jp::office::sheet::kMaxRows - 1)});
        for (const auto &sp : jp::office::sheet::spansWithout(s.hiddenCols, nowCols)) changes.push_back({place, true, true, std::min<quint32>(sp.first, jp::office::sheet::kMaxColumns - 1), std::min<quint32>(sp.second, jp::office::sheet::kMaxColumns - 1)});
    }
    if (!changes.empty()) {
        QUndoStack *stack = m_session->undoStack();
        stack->beginMacro(tr("Show Custom View"));
        for (const Change &c : changes) {
            crunch::EditSession::SizeChange by;
            by.setHidden = true;
            by.hidden = c.hide;
            if (c.column) m_session->resizeColumns(c.sheet, c.first, c.last, by, tr("Show Custom View"));
            else m_session->resizeRows(c.sheet, c.first, c.last, by, tr("Show Custom View"));
        }
        stack->endMacro();
    }
    // the look of each sheet
    for (const CustomViewSheet &s : view.sheets) {
        const int place = placeOf(s.name);
        if (place < 0 || place >= calc->sheetCount()) continue;
        grid::GridView::ViewState st = place == m_sheet ? m_view->viewState() : m_views.value(place);
        if (!st.valid) {
            st.valid = true;
            st.showZeros = calc->sheet(place).view.showZeros;
        }
        st.zoom = s.zoom / 100.0;
        st.skipRows = st.skipCols = 0;
        st.frozenRows = s.frozen ? qint64(s.frozenRows) : 0;
        st.frozenCols = s.frozen ? qint64(s.frozenCols) : 0;
        st.scrollX = st.scrollY = 0;
        st.gridlines = view.gridlines;
        st.headings = view.headings;
        st.selection.activeRow = st.selection.headRow = st.selection.range.row1 = st.selection.range.row2 = qint64(s.activeRow);
        st.selection.activeCol = st.selection.headCol = st.selection.range.col1 = st.selection.range.col2 = qint64(s.activeCol);
        st.selection.whole = grid::Selection::Whole::None;
        if (place == m_sheet) {
            m_view->setViewState(st);
            syncPanes();
            act(QStringLiteral("view.gridlines"))->setChecked(view.gridlines);
            act(QStringLiteral("view.headings"))->setChecked(view.headings);
        } else {
            m_views.insert(place, st);
            jp::office::sheet::SheetView pane = m_session->pages().pane(place);
            pane.frozen = s.frozen;
            pane.frozenRows = s.frozen ? s.frozenRows : 0;
            pane.frozenCols = s.frozen ? s.frozenCols : 0;
            pane.paneRow = pane.frozenRows;
            pane.paneCol = pane.frozenCols;
            pane.topRow = pane.leftCol = 0;
            pane.activeRow = s.activeRow;
            pane.activeCol = s.activeCol;
            m_session->setSheetPane(place, pane);
            m_session->setSheetView(place, m_session->pages().viewKind(place), s.zoom);
        }
    }
    act(QStringLiteral("view.formulaBar"))->setChecked(view.formulaBar);
    m_formulaRow->setVisible(view.formulaBar);
    if (view.windowWidth > 0 && view.windowHeight > 0 && !isMaximized() && !isFullScreen())
        resize(std::max(400, view.windowWidth / kTwipsPerPixel), std::max(300, view.windowHeight / kTwipsPerPixel));
    if (m_session->viewsChanged()) setWindowModified(true);
    return true;
}

} // namespace jp
