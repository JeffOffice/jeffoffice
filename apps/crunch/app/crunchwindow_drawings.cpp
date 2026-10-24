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
}

// The workbook is open (its first rows are read): the objects are its.
void CrunchWindow::openObjects()
{
    if (!m_objects || !m_loaded) return;
    m_objects->open(m_loaded->book);
    m_objects->setEditsSource([this] { return m_session ? m_session->structureEdits() : std::vector<jp::office::sheet::StructEdit>(); });
}

// The sheet is in the grid: a chart sheet has no cells to see (no headings or gridlines), and its objects are made a moment later, between events, so that the first screen is not held
// up by a drawing.
void CrunchWindow::prepareObjects(int sheet)
{
    if (!m_objects || !m_loaded || !m_loaded->book) return;
    if (m_objects->isChartSheet(sheet)) {
        m_view->setShowHeadings(false);
        m_view->setShowGridlines(false);
    }
    const quint64 token = ++m_objectsToken;
    QMetaObject::invokeMethod(
        this,
        [this, sheet, token] {
            if (token != m_objectsToken || !m_objects || sheet != m_sheet) return;
            m_objects->show(sheet);
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
    connect(m_session.get(), &crunch::EditSession::sizesChanged, this, [this](int sheet) {
        if (m_objects) m_objects->sizesChanged(sheet);
    });
}

void CrunchWindow::deleteObject(int id)
{
    Q_UNUSED(id);
}

} // namespace jp
