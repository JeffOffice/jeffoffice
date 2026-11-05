// Draw Border, Draw Border Grid, and Erase Border of the Borders menu: with one on, the person drags over cells and the lines are put on (the outside of what was dragged over, or all its
// lines) or taken off (every line of those cells). The mode stays on for more drags until Esc.
#include "app/crunchwindow.h"

#include <QAction>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QUndoStack>

namespace jp {

using namespace jp::office::sheet;

void CrunchWindow::createDrawBorderActions()
{
    mk("fmt.border.draw", tr("Draw Border"), "pencil", QKeySequence(), [this] { startDrawBorder(1); });
    mk("fmt.border.drawGrid", tr("Draw Border Grid"), "grid-2x2", QKeySequence(), [this] { startDrawBorder(2); });
    mk("fmt.border.erase", tr("Erase Border"), "eraser", QKeySequence(), [this] { startDrawBorder(3); });
    for (const char *id : {"fmt.border.draw", "fmt.border.drawGrid", "fmt.border.erase"}) m_opsActions << QString::fromLatin1(id);
}

void CrunchWindow::startDrawBorder(int mode)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed()) return;
    m_drawBorder = mode;
    m_drawFromRow = m_drawFromCol = -1;
    m_view->viewport()->installEventFilter(this);   // (the window sees the grid's mouse only through this)
    m_view->viewport()->setCursor(Qt::CrossCursor);
    setStatus(mode == 3 ? tr("Drag over cells to erase their borders. Press Esc when you are done.") : tr("Drag over cells to draw borders. Press Esc when you are done."));
}

void CrunchWindow::endDrawBorder()
{
    if (!m_drawBorder) return;
    m_drawBorder = 0;
    m_drawFromRow = m_drawFromCol = -1;
    m_view->viewport()->removeEventFilter(this);
    m_view->viewport()->unsetCursor();
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
}

// The mouse and keys while a border is being drawn: true when the event was used.
bool CrunchWindow::drawBorderEvent(QObject *watched, QEvent *e)
{
    if (e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape && (watched == m_view || watched == m_view->viewport())) {
        endDrawBorder();
        return true;
    }
    if (watched != m_view->viewport()) return false;
    const auto type = e->type();
    if (type != QEvent::MouseButtonPress && type != QEvent::MouseButtonRelease && type != QEvent::MouseMove && type != QEvent::MouseButtonDblClick) return false;
    auto *m = static_cast<QMouseEvent *>(e);
    const grid::Hit h = m_view->sheet().layout().hit(m->position().toPoint());
    if (type == QEvent::MouseMove) return m_drawFromRow >= 0 || h.kind == grid::Hit::Kind::Cell;
    if (m->button() != Qt::LeftButton) return false;
    if (type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick) {
        if (h.kind != grid::Hit::Kind::Cell) return false;
        m_drawFromRow = h.row;
        m_drawFromCol = h.col;
        return true;
    }
    if (m_drawFromRow < 0) return false;
    const qint64 r1 = m_drawFromRow, c1 = m_drawFromCol;
    m_drawFromRow = m_drawFromCol = -1;
    if (h.kind != grid::Hit::Kind::Cell) return true;
    using E = crunch::EditSession;
    CellRange range;
    range.row1 = quint32(std::min(r1, h.row));
    range.row2 = quint32(std::max(r1, h.row));
    range.col1 = quint32(std::min(c1, h.col));
    range.col2 = quint32(std::max(c1, h.col));
    range.valid = true;
    BorderSide line;
    line.style = m_lineStyle;
    line.color = m_lineColor;
    const int outside = E::EdgeLeft | E::EdgeRight | E::EdgeTop | E::EdgeBottom, inside = E::EdgeInsideHorizontal | E::EdgeInsideVertical;
    int edges = outside;
    QString text = tr("Draw Border");
    if (m_drawBorder == 2) {
        edges = outside | inside;
        text = tr("Draw Border Grid");
    } else if (m_drawBorder == 3) {
        edges = outside | inside | E::EdgeDiagonalUp | E::EdgeDiagonalDown;
        line = BorderSide();
        text = tr("Erase Border");
    }
    if (!editingAllowed()) return true;
    const E::Outcome o = m_session->setBorders(m_sheet, range, edges, line, text);
    if (!o.ok) notice(tr("Borders"), o.problem);
    updateFormatState();
    return true;
}

} // namespace jp
