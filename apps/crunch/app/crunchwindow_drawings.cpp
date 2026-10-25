// The charts, pictures, and shapes over the cells in JeffCrunch's window: made for the sheet in view when it is shown, drawn again when a cell a chart reads changes, moved when rows or
// columns are inserted or deleted. The work is core/sheetobjects.h's; this joins it to the window (the sheet in view, the editing session's signals, the calculation).

#include "app/crunchwindow.h"

#include "core/sheetobjects.h"

#include <QMetaObject>

namespace jp {

void CrunchWindow::setupObjects()
{
    m_objects = std::make_unique<crunch::SheetObjects>();
    m_objects->layer()->attach(&m_view->sheet().layout());
    m_view->setOverlay(m_objects->layer());
    m_objects->layer()->setDeleteHook([this](int id) { deleteObject(id); });
    // the person moves or sizes an object, picks one, or asks for its menu
    connect(m_objects->layer(), &jp::grid::ObjectLayer::objectMoved, this, [this](int id, const jp::office::sheet::DrawingAnchor &anchor) { moveObject(id, anchor); });
    connect(m_objects->layer(), &jp::grid::ObjectLayer::contextMenuRequested, this, [this](int id, const QPoint &global) { objectContextMenu(id, global); });
    connect(m_objects->layer(), &jp::grid::ObjectLayer::objectActivated, this, [this](int) { propertiesDialog(); });
    connect(m_objects->layer(), &jp::grid::ObjectLayer::selectionChanged, this, [this](int) { updateChartState(); });
    connect(m_view, &grid::GridView::viewChanged, this, [this] { m_chartButtonsTimer.start(); });
    connect(m_view, &grid::GridView::zoomChanged, this, [this] { m_chartButtonsTimer.start(); });
}

// The workbook is open (its first rows are read): the objects are its.
void CrunchWindow::openObjects()
{
    if (!m_objects || !m_loaded) return;
    m_objects->open(m_loaded->book);
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
    const crunch::EditSession::Outcome o = m_session->deleteObject(m_sheet, id, tr("Delete Object"), before, after);
    if (!o.ok) notice(tr("Delete"), o.problem);
}

} // namespace jp
