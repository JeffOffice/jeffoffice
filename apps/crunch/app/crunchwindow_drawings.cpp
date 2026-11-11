// The charts, pictures, and shapes over the cells in JeffCrunch's window: made for the sheet in view when it is shown, drawn again when a cell a chart reads changes, moved when rows or
// columns are inserted or deleted. The work is core/sheetobjects.h's; this joins it to the window (the sheet in view, the editing session's signals, the calculation).

#include "app/crunchwindow.h"

#include "app/objectpicker.h"
#include "core/sheetobjects.h"

#include <QFileInfo>
#include <QMetaObject>
#include <QUndoStack>
#include <QUuid>

namespace jp {

void CrunchWindow::setupObjects()
{
    m_objects = std::make_unique<crunch::SheetObjects>();
    // a pivot chart made here is named from its table's name, its sheet's, and the file's, as they are when a save is made
    m_objects->setPivotNamer([this](const jp::office::Chart &c) {
        if (!m_session || !m_loaded || c.pivotTable < 0 || c.pivotTable >= m_session->workbook()->pivots().tableCount()) return c.pivotName;
        const auto &pb = m_session->workbook()->pivots();
        const int sheet = pb.sheetOfTable(c.pivotTable);
        const auto &sheets = m_loaded->book->info().sheets;
        if (!pb.table(c.pivotTable).table || sheet < 0 || sheet >= sheets.size()) return c.pivotName;
        const QString file = !m_saveFileName.isEmpty() ? m_saveFileName : m_path.isEmpty() ? QStringLiteral("Book1.xlsx") : QFileInfo(m_path).fileName();
        return QStringLiteral("[%1]%2!%3").arg(file, sheets[sheet].name, pb.table(c.pivotTable).table->name);
    });
    // the buttons of a pivot chart carry the names of its table's fields
    m_objects->setPivotFielder([this](jp::office::Chart *c, int ownSheet) {
        if (!m_session || !m_loaded) return;
        const auto &pb = m_session->workbook()->pivots();
        const auto &sheets = m_loaded->book->info().sheets;
        // "[Book.xlsx]Sheet1!Table1": the sheet (maybe quoted) and the table; a name with no sheet in it is a table of the chart's own sheet
        QString text = c->pivotName;
        if (text.startsWith(QLatin1Char('['))) text = text.mid(text.indexOf(QLatin1Char(']')) + 1);
        const int bang = int(text.lastIndexOf(QLatin1Char('!')));
        const QString name = text.mid(bang + 1);
        QString sheetName = bang > 0 ? text.left(bang) : QString();
        if (sheetName.size() > 1 && sheetName.startsWith(QLatin1Char('\'')) && sheetName.endsWith(QLatin1Char('\''))) sheetName = sheetName.mid(1, sheetName.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
        for (int t = 0; t < pb.tableCount(); ++t) {
            const auto &entry = pb.table(t);
            if (!entry.table || entry.cache < 0) continue;
            const int tableSheet = pb.sheetOfTable(t);
            if (tableSheet < 0 || tableSheet >= sheets.size()) continue;
            // (a chart made here knows its table's place; one a file holds is found by the table's name and its sheet's)
            if (c->pivotTable >= 0) {
                if (t != c->pivotTable) continue;
            } else if (entry.table->name != name || (sheetName.isEmpty() ? tableSheet != ownSheet : sheets[tableSheet].name.compare(sheetName, Qt::CaseInsensitive) != 0)) {
                continue;
            }
            const auto &cache = pb.cache(entry.cache).cache;
            if (!cache) return;
            const jp::office::sheet::PivotTable &pt = *entry.table;
            auto fieldName = [&](int f) {
                if (f == -2) return pt.dataCaption;
                if (f < 0 || f >= pt.fields.size()) return QString();
                if (!pt.fields[f].name.isEmpty()) return pt.fields[f].name;
                return f < cache->fields.size() ? cache->fields[f].name : QString();
            };
            jp::office::Chart::PivotButtons &b = c->pivotButtons;
            b.filterNames.clear();
            b.categoryNames.clear();
            b.seriesNames.clear();
            b.dataNames.clear();
            for (const auto &p : pt.pageFields) b.filterNames << fieldName(p.fld);
            for (int f : pt.rowFields) b.categoryNames << fieldName(f);
            for (int f : pt.colFields) b.seriesNames << fieldName(f);
            for (const auto &d : pt.dataFields) b.dataNames << (d.name.isEmpty() ? fieldName(d.fld) : d.name);
            for (QStringList *l : {&b.filterNames, &b.categoryNames, &b.seriesNames, &b.dataNames}) l->removeAll(QString());
            return;
        }
        // (no such table: no names, so no buttons)
        c->pivotButtons.filterNames.clear();
        c->pivotButtons.categoryNames.clear();
        c->pivotButtons.seriesNames.clear();
        c->pivotButtons.dataNames.clear();
    });
    m_objects->layer()->attach(&m_view->sheet().layout());
    m_picker = new ObjectPicker(m_objects->layer(), this);
    m_view->setOverlay(m_picker);
    // (the layer asks for its own repaints: the view has set its hook on the picker)
    m_objects->layer()->setRepaintHook([this](const QRect &) { m_view->viewport()->update(); });
    connect(m_picker, &ObjectPicker::pickedChanged, this, [this] {
        updateChartState();
        updateArrangeActions();
    });
    connect(m_picker, &ObjectPicker::deleteRequested, this, [this] {
        const std::vector<int> ids = pickedObjects();
        if (ids.size() < 2 || !m_session || m_edit.active || m_saving) return;
        QUndoStack *stack = m_session->undoStack();
        stack->beginMacro(tr("Delete Objects"));
        for (const int id : ids) deleteObject(id);
        stack->endMacro();
    });
    m_objects->layer()->setDeleteHook([this](int id) { deleteObject(id); });
    m_objects->setSaveNote([this](const QString &text) { notice(tr("Save"), text); });
    // the person moves or sizes an object, picks one, or asks for its menu
    connect(m_objects->layer(), &jp::grid::ObjectLayer::objectMoved, this, [this](int id, const jp::office::sheet::DrawingAnchor &anchor) { moveObject(id, anchor); });
    connect(m_objects->layer(), &jp::grid::ObjectLayer::contextMenuRequested, this, [this](int id, const QPoint &global) { objectContextMenu(id, global); });
    connect(m_objects->layer(), &jp::grid::ObjectLayer::objectActivated, this, [this](int id) {
        QString line;
        if (m_objects->equationOf(m_sheet, id, &line, nullptr)) insertEquationDialog();   // (an equation opens its editor)
        else propertiesDialog();
    });
    connect(m_objects->layer(), &jp::grid::ObjectLayer::selectionChanged, this, [this](int) { updateChartState(); });
    connect(m_view, &grid::GridView::viewChanged, this, [this] { m_chartButtonsTimer.start(); });
    connect(m_view, &grid::GridView::zoomChanged, this, [this] { m_chartButtonsTimer.start(); });
}

// The workbook is open (its first rows are read): the objects are its.
void CrunchWindow::openObjects()
{
    if (!m_objects || !m_loaded) return;
    m_objects->open(m_loaded->book);
    m_workbookId = QUuid::createUuid().toString(QUuid::WithoutBraces);   // (128 random bits: no other program, and no other opening of the same file, has it)
    m_objects->setEditsSource([this]() -> const std::vector<jp::office::sheet::StructEdit> & {
        static const std::vector<jp::office::sheet::StructEdit> none;
        return m_session ? m_session->structureEdits() : none;
    });
    m_objects->setSheetOpsSource([this]() -> const std::vector<jp::office::sheet::SheetOp> & {
        static const std::vector<jp::office::sheet::SheetOp> none;
        return m_session ? m_session->sheetOperations() : none;
    });
}

// The sheet is in the grid: a chart sheet has no cells to see (no headings or gridlines), and its objects are made a moment later, between events, so that the first screen is not held
// up by a drawing.
void CrunchWindow::prepareObjects(int sheet)
{
    if (!m_objects || !m_loaded || !m_loaded->book) return;
    const bool chartSheet = m_objects->isChartSheet(sheet);
    if (chartSheet) {
        // (a chart sheet is a chart: no cells, no cursor, no typing, no formula bar)
        m_view->setShowHeadings(false);
        m_view->setShowGridlines(false);
    }
    m_chartSheetShown = chartSheet;
    updateChartState();   // (the object that was picked is let go of with the sheet)
    updateEditActions();
    m_view->setCellsEnabled(!chartSheet);
    m_formulaBar->setReadOnly(chartSheet);
    m_nameBox->setEnabled(!chartSheet);
    if (chartSheet) {
        m_formulaBar->clear();
        m_nameBox->clear();
    }
    if (m_objects->shownSheet() == sheet) {
        // the objects of the sheet are in the layer (the sheet was made again, by an edit that moved cells or by being read whole): they stay, with their pictures and the one picked, and are
        // placed again on the rows and columns it has now
        m_objects->layer()->layoutChanged();
        if (m_haveCalc) m_objects->geometryKnown(sheet);
        return;
    }
    const quint64 token = ++m_objectsToken;
    QMetaObject::invokeMethod(
        this,
        [this, sheet, token] {
            if (token != m_objectsToken || !m_objects || sheet != m_sheet) return;
            m_objects->show(sheet);
            updateChartState();
            if (m_objects->isChartSheet(sheet) && m_objects->layer()->count() == 0 && !m_calculating && !m_loading)
                setStatus(tr("This chart sheet's chart can't be shown here."));
        },
        Qt::QueuedConnection);
}

void CrunchWindow::connectObjects()
{
    if (!m_objects || !m_session) return;
    connect(m_session.get(), &crunch::EditSession::cellsMoved, this, [this] {
        if (m_objects) m_objects->cellsMoved();
    });
    connect(m_session.get(), &crunch::EditSession::sheetsChanged, this, [this] {
        if (m_objects) m_objects->sheetsChanged();
    });
    connect(m_session.get(), &crunch::EditSession::sizesChanged, this, [this](int sheet) {
        if (m_objects) m_objects->sizesChanged(sheet);
    });
    m_session->setObjectHook([this](const crunch::ObjectEdit &edit, bool forward) {
        if (m_objects) m_objects->applyState(edit.sheet, edit.id, forward ? edit.after : edit.before);
        updateChartState();
    });
}

// Delete with an object picked: it is taken away, as a step Undo gives back.
void CrunchWindow::deleteObject(int id)
{
    if (!m_session || m_edit.active || m_saving) return;
    QString why;
    if (m_objects && !m_objects->canDelete(m_sheet, id, &why)) {
        notice(tr("Delete"), why);
        return;
    }
    const crunch::ObjectState before = m_objects->stateOf(m_sheet, id);
    crunch::ObjectState after = before;
    after.deleted = true;
    // (what writes it back if Undo is used after a save has dropped it: kept with the step)
    after.revive = m_objects->captureRevive(m_sheet, id);
    crunch::ObjectState keep = before;
    keep.revive = after.revive;
    // (a chart sheet with no chart is of no use: it goes with its chart, as one step)
    const bool chartSheet = m_objects && m_objects->isChartSheet(m_sheet);
    if (chartSheet && (!m_session->canChangeSheetList(false, &why) || !m_session->allows(m_sheet, crunch::EditSession::Allow::Objects, &why))) {
        notice(tr("Delete"), why);
        return;
    }
    QUndoStack *stack = m_session->undoStack();
    if (chartSheet) stack->beginMacro(tr("Delete Chart Sheet"));
    const int sheet = m_sheet;
    const crunch::EditSession::Outcome o = m_session->deleteObject(m_sheet, id, tr("Delete Object"), keep, after);
    if (!o.ok) notice(tr("Delete"), o.problem);
    if (chartSheet) {
        const crunch::EditSession::Outcome gone = o.ok ? m_session->deleteSheet(sheet) : o;
        stack->endMacro();
        if (!gone.ok) notice(tr("Delete"), gone.problem);
    }
}

} // namespace jp
