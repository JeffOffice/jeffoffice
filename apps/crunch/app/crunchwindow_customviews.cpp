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
#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <QUndoCommand>
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

namespace {

// The most changes of hidden rows and columns a view may make when it is shown (the runs it holds, and the runs of the sheets as they are now that it makes show again).
constexpr int kMaxViewChanges = 2 * jp::office::sheet::kMaxCustomViewSpans;

// Showing a view as one step of Undo: what is not rows or columns (the sheet in front, the zoom, the panes, the cell, the gridlines, the headings, the formula bar, the window's size) goes back to how
// it was, and Redo puts the view's look again. It sits after the changes of rows and columns inside the same macro; the window has already made it look as `after` when it is made.
class ShowLook : public QUndoCommand {
public:
    ShowLook(CrunchWindow *window, CustomView before, CustomView after) : QUndoCommand(QCoreApplication::translate("jp::CrunchWindow", "Show Custom View")), m_window(window), m_before(std::move(before)), m_after(std::move(after)) {}
    void undo() override { m_window->applyCustomViewLook(m_before); }
    void redo() override
    {
        if (m_first) m_first = false;
        else m_window->applyCustomViewLook(m_after);
    }

private:
    CrunchWindow *m_window;
    CustomView m_before, m_after;
    bool m_first = true;
};

} // namespace

// How the workbook looks now. `hidden`: with each sheet's hidden rows and columns too (the rows a filter hides are not held: the filter shows them, and a view that held them would hide them by hand);
// false, with `tooMany` set, when the runs of hidden rows and columns come to more than a view keeps.
CustomView CrunchWindow::lookOfWorkbook(bool hidden, bool *tooMany)
{
    CustomView v;
    v.hasState = true;
    if (tooMany) *tooMany = false;
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
    int spans = 0;
    for (const int place : info.order) {
        if (place < 0 || place >= info.sheets.size() || info.sheets[place].deleted || info.sheets[place].kind != jp::office::sheet::SheetKind::Worksheet || place >= calc->sheetCount()) continue;
        if (v.sheets.size() >= jp::office::sheet::kMaxCustomViewSheets) {
            if (tooMany) *tooMany = true;
            break;
        }
        CustomViewSheet s;
        s.name = info.sheets[place].name;
        const grid::GridView::ViewState st = place == m_sheet ? m_view->viewState() : m_views.value(place);
        const jp::office::sheet::SheetView &pane = m_session->pages().pane(place);
        if (st.valid) {
            s.zoom = std::clamp(int(std::lround(st.zoom * 100)), 10, 400);
            s.frozenRows = quint32(std::max<qint64>(0, st.frozenRows - st.skipRows));
            s.frozenCols = quint32(std::max<qint64>(0, st.frozenCols - st.skipCols));
            s.activeRow = quint32(std::clamp<qint64>(st.selection.activeRow, 0, jp::office::sheet::kMaxRows - 1));
            s.activeCol = quint32(std::clamp<qint64>(st.selection.activeCol, 0, jp::office::sheet::kMaxColumns - 1));
        } else {
            s.zoom = std::clamp(m_session->pages().normalZoom(place), 10, 400);
            s.frozenRows = pane.frozenRows;
            s.frozenCols = pane.frozenCols;
            s.activeRow = pane.activeRow;
            s.activeCol = pane.activeCol;
        }
        if (s.frozenRows > jp::office::sheet::kCustomViewMaxFrozenRows || s.frozenCols > jp::office::sheet::kCustomViewMaxFrozenCols) s.frozenRows = s.frozenCols = 0;
        s.frozen = s.frozenRows || s.frozenCols;
        if (hidden) {
            Spans rows, cols;
            for (const jp::office::sheet::RowSpec &r : calc->rowSpecsIn(place, 0, jp::office::sheet::kMaxRows - 1))
                if (r.hidden && !r.filtered) rows.append({r.row, r.row});
            for (const jp::office::sheet::ColumnSpan &c : calc->columnRuns(place, 0, jp::office::sheet::kMaxColumns - 1))
                if (c.hidden) cols.append({c.first, c.last});
            s.hiddenRows = jp::office::sheet::normalizedSpans(rows);
            s.hiddenCols = jp::office::sheet::normalizedSpans(cols);
            spans += int(s.hiddenRows.size() + s.hiddenCols.size());
            if (spans > jp::office::sheet::kMaxCustomViewSpans && tooMany) *tooMany = true;
        }
        v.sheets.append(s);
    }
    return v;
}

// How the workbook looks now, under a name; false, with what is the matter, when what it holds is more than a view keeps (it would not read back the same).
bool CrunchWindow::captureCustomView(const QString &name, CustomView *out, QString *problem)
{
    bool tooMany = false;
    CustomView v = lookOfWorkbook(true, &tooMany);
    if (tooMany || jp::office::sheet::customViewSpanCount(v) > jp::office::sheet::kMaxCustomViewSpans) {
        if (problem) *problem = tr("This view can't be kept: the workbook has more runs of hidden rows and columns, or more sheets, than a view holds (%1 runs, %2 sheets). Unhide some, and add the view again.").arg(jp::office::sheet::kMaxCustomViewSpans).arg(jp::office::sheet::kMaxCustomViewSheets);
        return false;
    }
    v.name = name;
    v.guid = jp::office::sheet::newCustomViewGuid();
    *out = v;
    return true;
}

void CrunchWindow::customViews()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || !m_haveCalc || m_sheet < 0 || !m_session->workbook()) return;
    CustomViewsRequest req;
    req.views = m_session->customViews();
    req.capture = [this](const QString &name, CustomView *view, QString *problem) { return captureCustomView(name, view, problem); };
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
    if (req.show >= 0) showCustomViewAt(req.show);
}

// Puts the look of a view on the window: the sheet in front, and for each sheet the zoom, the panes, and the cell chosen; the gridlines, headings, formula bar, and the window's size. (The rows and
// columns are the session's, and not changed here.)
void CrunchWindow::applyCustomViewLook(const CustomView &view)
{
    if (!m_session || !m_haveCalc || !m_session->workbook() || !calculation()) return;
    const jp::office::sheet::WorkbookInfo &info = m_session->workbook()->info();
    const jp::office::sheet::Calculation *calc = calculation();
    QHash<QString, int> places;
    for (const int place : info.order)
        if (place >= 0 && place < info.sheets.size() && !info.sheets[place].deleted && info.sheets[place].kind == jp::office::sheet::SheetKind::Worksheet && place < calc->sheetCount())
            places.insert(info.sheets[place].name.toCaseFolded(), place);
    int target = places.value(view.activeSheet.toCaseFolded(), -1);
    if (target < 0)
        for (const int place : info.order)
            if (place >= 0 && place < info.sheets.size() && !info.sheets[place].deleted && info.sheets[place].sheetId == view.activeSheetId && places.contains(info.sheets[place].name.toCaseFolded())) target = place;
    if (target >= 0 && target != m_sheet) setActiveSheet(target);
    rememberView();
    QSet<int> done;
    for (const CustomViewSheet &s : view.sheets) {
        const int place = places.value(s.name.toCaseFolded(), -1);
        if (place < 0 || done.contains(place)) continue;   // (a sheet is looked at once)
        done.insert(place);
        grid::GridView::ViewState st = place == m_sheet ? m_view->viewState() : m_views.value(place);
        if (!st.valid) {
            st.valid = true;
            st.showZeros = calc->sheet(place).view.showZeros;
        }
        const bool frozen = s.frozen && s.frozenRows <= jp::office::sheet::kCustomViewMaxFrozenRows && s.frozenCols <= jp::office::sheet::kCustomViewMaxFrozenCols;
        const quint32 row = std::min<quint32>(s.activeRow, jp::office::sheet::kMaxRows - 1), col = std::min<quint32>(s.activeCol, jp::office::sheet::kMaxColumns - 1);
        st.zoom = std::clamp(s.zoom, 10, 400) / 100.0;
        st.skipRows = st.skipCols = 0;
        st.frozenRows = frozen ? qint64(s.frozenRows) : 0;
        st.frozenCols = frozen ? qint64(s.frozenCols) : 0;
        st.scrollX = st.scrollY = 0;
        st.gridlines = view.gridlines;
        st.headings = view.headings;
        st.selection.activeRow = st.selection.headRow = st.selection.range.row1 = st.selection.range.row2 = qint64(row);
        st.selection.activeCol = st.selection.headCol = st.selection.range.col1 = st.selection.range.col2 = qint64(col);
        st.selection.whole = grid::Selection::Whole::None;
        if (place == m_sheet) {
            m_view->setViewState(st);
            syncPanes();
            act(QStringLiteral("view.gridlines"))->setChecked(view.gridlines);
            act(QStringLiteral("view.headings"))->setChecked(view.headings);
        } else {
            m_views.insert(place, st);
            jp::office::sheet::SheetView pane = m_session->pages().pane(place);
            pane.frozen = frozen;
            pane.frozenRows = frozen ? s.frozenRows : 0;
            pane.frozenCols = frozen ? s.frozenCols : 0;
            pane.paneRow = pane.frozenRows;
            pane.paneCol = pane.frozenCols;
            pane.topRow = pane.leftCol = 0;
            pane.activeRow = row;
            pane.activeCol = col;
            m_session->setSheetPane(place, pane);
            m_session->setSheetView(place, m_session->pages().viewKind(place), std::clamp(s.zoom, 10, 400));
        }
    }
    act(QStringLiteral("view.formulaBar"))->setChecked(view.formulaBar);
    m_formulaRow->setVisible(view.formulaBar);
    if (view.windowWidth > 0 && view.windowHeight > 0 && !isMaximized() && !isFullScreen())
        resize(std::max(400, view.windowWidth / kTwipsPerPixel), std::max(300, view.windowHeight / kTwipsPerPixel));
}

// Shows the view at this place in the list: the rows and columns of each of its sheets hidden or shown (a sheet once; a run of rows is one change; what a filter hides is left to the filter), and its
// look; all of it one step of Undo. A sheet the view does not know is left as it is; false, with a message, when the view asks for more changes than a view may.
bool CrunchWindow::showCustomViewAt(int index)
{
    if (!m_session || !m_haveCalc || !m_session->workbook() || !calculation() || index < 0 || index >= m_session->customViews().size()) return false;
    const CustomView view = m_session->customViews()[index];
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return false;
    const jp::office::sheet::WorkbookInfo &info = m_session->workbook()->info();
    const jp::office::sheet::Calculation *calc = calculation();
    QHash<QString, int> places;
    for (const int place : info.order)
        if (place >= 0 && place < info.sheets.size() && !info.sheets[place].deleted && info.sheets[place].kind == jp::office::sheet::SheetKind::Worksheet && place < calc->sheetCount())
            places.insert(info.sheets[place].name.toCaseFolded(), place);
    struct Change {
        int sheet;
        bool column, hide;
        quint32 first, last;
    };
    std::vector<Change> changes;
    QSet<int> done;
    for (const CustomViewSheet &s : view.sheets) {
        const int place = places.value(s.name.toCaseFolded(), -1);
        if (place < 0 || done.contains(place) || !m_session->canEditSheet(place, nullptr)) continue;
        done.insert(place);
        Spans nowRows, nowCols, filtered;
        for (const jp::office::sheet::RowSpec &r : calc->rowSpecsIn(place, 0, jp::office::sheet::kMaxRows - 1)) {
            if (!r.hidden) continue;
            (r.filtered ? filtered : nowRows).append({r.row, r.row});
        }
        for (const jp::office::sheet::ColumnSpan &c : calc->columnRuns(place, 0, jp::office::sheet::kMaxColumns - 1))
            if (c.hidden) nowCols.append({c.first, c.last});
        nowRows = jp::office::sheet::normalizedSpans(nowRows);
        nowCols = jp::office::sheet::normalizedSpans(nowCols);
        filtered = jp::office::sheet::normalizedSpans(filtered);
        const Spans wantRows = jp::office::sheet::normalizedSpans(s.hiddenRows), wantCols = jp::office::sheet::normalizedSpans(s.hiddenCols);
        auto add = [&](const Spans &spans, bool column, bool hide) {
            const quint32 last = column ? jp::office::sheet::kMaxColumns - 1 : jp::office::sheet::kMaxRows - 1;
            for (const auto &sp : spans)
                if (sp.first <= last) changes.push_back({place, column, hide, sp.first, std::min(sp.second, last)});
        };
        add(jp::office::sheet::spansWithout(nowRows, wantRows), false, false);
        add(jp::office::sheet::spansWithout(nowCols, wantCols), true, false);
        add(jp::office::sheet::spansWithout(jp::office::sheet::spansWithout(wantRows, nowRows), filtered), false, true);
        add(jp::office::sheet::spansWithout(wantCols, nowCols), true, true);
        if (int(changes.size()) > kMaxViewChanges) {
            notice(tr("Custom Views"), tr("This view would make more changes to hidden rows and columns than a view may (%1). Nothing was changed.").arg(kMaxViewChanges));
            return false;
        }
    }
    const CustomView before = lookOfWorkbook(false);
    QUndoStack *stack = m_session->undoStack();
    stack->beginMacro(tr("Show Custom View"));
    for (const Change &c : changes) {
        crunch::EditSession::SizeChange by;
        by.setHidden = true;
        by.hidden = c.hide;
        if (c.column) m_session->resizeColumns(c.sheet, c.first, c.last, by, tr("Show Custom View"));
        else m_session->resizeRows(c.sheet, c.first, c.last, by, tr("Show Custom View"));
    }
    applyCustomViewLook(view);
    stack->push(new ShowLook(this, before, lookOfWorkbook(false)));
    stack->endMacro();
    if (m_session->viewsChanged()) setWindowModified(true);
    return true;
}

} // namespace jp
