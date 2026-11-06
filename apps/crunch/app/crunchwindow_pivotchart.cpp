// Insert > Pivot Chart (a chart of the pivot table the cursor is in): in a file of its own, because the chart code and the pivot code each name cell ranges in their own way.
#include "app/crunchwindow.h"

#include "app/pivotpane.h"
#include "core/sheetobjects.h"
#include "sheet/chartmake.h"
#include "sheet/pivot.h"
#include "sheet/pivotbook.h"

#include <QFileInfo>
#include <QGuiApplication>

namespace jp {

using namespace jp::office;
using namespace jp::office::sheet;

// Insert > Pivot Chart: a chart of the pivot table the cursor is in (its values, without the grand totals), marked as the table's chart. It reads the table's cells, so the numbers follow a refresh.
bool CrunchWindow::insertPivotChart()
{
    if (!m_session || !m_objects || !m_loaded || m_chartSheetShown) return false;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return false;
    const int t = selectedPivot();
    if (t < 0) {
        notice(tr("Pivot Chart"), tr("Click a cell in a pivot table first."), false);
        return false;
    }
    if (!m_objects->cells()) {
        notice(tr("Pivot Chart"), tr("The workbook is still being read. A chart can be made when it is open."));
        return false;
    }
    const PivotTableEntry &entry = m_session->workbook()->pivots().table(t);
    const jp::office::sheet::CellRange where = m_session->workbook()->pivots().locationOf(t);
    if (!entry.table || !where.valid || entry.table->dataFields.isEmpty() || entry.table->firstDataRow < 1) {
        notice(tr("Pivot Chart"), tr("This pivot table has no values to chart yet. Add a field to Values first."), false);
        return false;
    }
    const PivotTable &pt = *entry.table;
    jp::office::sheet::CellRange block = where;
    block.row1 = where.row1 + quint32(pt.firstDataRow) - 1;   // (the line of the items' names, under the captions)
    const bool bottomTotal = !pt.rowItems.isEmpty() && pt.rowItems.last().t == QLatin1String("grand");
    const bool rightTotal = !pt.colItems.isEmpty() && pt.colItems.last().t == QLatin1String("grand");
    if (bottomTotal && block.row2 > block.row1) --block.row2;
    if (rightTotal && block.col2 > block.col1) --block.col2;
    if (block.row2 <= block.row1 || block.col2 <= block.col1) {
        notice(tr("Pivot Chart"), tr("This pivot table has too little in it to chart."), false);
        return false;
    }
    const QString sheetName = m_loaded->book->info().sheets[m_sheet].name;
    ChartPlan plan = planChartFrom(sheetName, block, true, true, false, ChartKind::Column);
    if (plan.series.isEmpty()) {
        notice(tr("Pivot Chart"), tr("This pivot table has no numbers to chart."), false);
        return false;
    }
    Chart chart = makeChart(ChartKind::Column, plan, m_objects->theme(), plan.points);
    const QString book = m_path.isEmpty() ? QStringLiteral("Book1.xlsx") : QFileInfo(m_path).fileName();
    chart.pivotName = QStringLiteral("[%1]%2!%3").arg(book, sheetName, pt.name);
    chart.pivotButtons.set = true;   // (a new pivot chart shows its field buttons)
    chart.pivotTable = t;   // (named again from the table's own name when the file is saved)
    QString why;
    const DrawingAnchor anchor = newObjectAnchor(480, 288);
    const int id = m_objects->makeChart(m_sheet, chart, anchor, &why);
    if (id < 0) {
        notice(tr("Pivot Chart"), why);
        return false;
    }
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = e.before;
    e.after.deleted = false;
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, tr("New Pivot Chart"));
    if (!o.ok) {
        m_objects->discardMade(m_sheet, id);
        notice(tr("Pivot Chart"), o.problem);
        return false;
    }
    m_objects->layer()->select(id);
    setStatus(tr("Pivot chart inserted"));
    return true;
}

} // namespace jp
