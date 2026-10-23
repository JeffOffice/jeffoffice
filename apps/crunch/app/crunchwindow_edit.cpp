// Editing in JeffCrunch's window: typing over a cell, in it (F2, a double click), or in the formula bar; Delete; Undo and Redo. What is typed is read and put in the cell by the
// EditSession (core/editsession.h), which holds the worker back for the moment the cell is put in; the worker works out what depends on it and tells the window in batches.

#include "app/crunchwindow.h"

#include "app/sheettabs.h"
#include "core/editsession.h"
#include "grid/gridview.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLineEdit>
#include <QUndoStack>

namespace jp {

QUndoStack *CrunchWindow::undoStack() const { return m_session ? m_session->undoStack() : nullptr; }

void CrunchWindow::createEditing()
{
    m_session = std::make_unique<crunch::EditSession>(&m_loader, m_loaded, &m_values, nullptr);
    connect(m_session.get(), &crunch::EditSession::cellsChanged, this, &CrunchWindow::onCellsChanged);
    QUndoStack *u = m_session->undoStack();
    connect(u, &QUndoStack::canUndoChanged, this, &CrunchWindow::updateEditActions);
    connect(u, &QUndoStack::canRedoChanged, this, &CrunchWindow::updateEditActions);
    connect(u, &QUndoStack::undoTextChanged, this, &CrunchWindow::updateEditActions);
    connect(u, &QUndoStack::redoTextChanged, this, &CrunchWindow::updateEditActions);
    connect(u, &QUndoStack::cleanChanged, this, [this](bool clean) {
        setWindowModified(!clean);
        updateEditActions();
    });
    setWindowModified(false);
    updateEditActions();
}

void CrunchWindow::updateEditActions()
{
    QUndoStack *u = undoStack();
    const bool open = m_session && !m_saving;
    act("edit.undo")->setEnabled(open && u->canUndo());
    act("edit.redo")->setEnabled(open && u->canRedo());
    act("edit.undo")->setToolTip(u && u->canUndo() ? tr("Undo %1 (%2)").arg(u->undoText(), QKeySequence(QKeySequence::Undo).toString(QKeySequence::NativeText)) : tr("Undo"));
    act("edit.redo")->setToolTip(u && u->canRedo() ? tr("Redo %1 (%2)").arg(u->redoText(), QKeySequence(QKeySequence::Redo).toString(QKeySequence::NativeText)) : tr("Redo"));
    const bool canSave = open && m_loaded && m_loaded->problem.isEmpty();
    act("file.save")->setEnabled(canSave);
    act("file.saveAs")->setEnabled(canSave);
}

// Whether a cell can be changed now; if not, the person is told why (as a note in the status line while the workbook is still being opened, else in a message).
bool CrunchWindow::editingAllowed()
{
    QString why;
    if (!m_session) {
        setStatus(tr("The workbook is still being opened. It can be changed when it is open."));
        return false;
    }
    if (!m_session->canEdit(&why)) {
        if (m_saving) setStatus(why);
        else notice(tr("Edit"), why);
        return false;
    }
    return true;
}

void CrunchWindow::beginEdit(const QString &initial, CellEditor::Mode mode, bool fromBar)
{
    if (m_edit.active) return;
    if (!editingAllowed()) return;
    const grid::Selection &sel = m_view->selection();
    const int sheet = m_sheet;
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    QString why;
    if (!m_session->canEditCell(sheet, row, col, &why)) {
        notice(tr("Edit"), why);
        return;
    }
    m_edit.active = true;
    m_edit.sheet = sheet;
    m_edit.row = row;
    m_edit.col = col;
    m_edit.original = m_formulaFull;
    m_view->ensureVisible(row, col);
    m_syncingEditors = true;
    m_cellEditor->setMode(mode);
    m_cellEditor->setText(initial);
    if (!fromBar) m_formulaBar->setText(initial);
    m_syncingEditors = false;
    m_cellEditor->show();
    m_cellEditor->raise();
    placeEditor();
    if (!fromBar) {
        m_cellEditor->setFocus(Qt::OtherFocusReason);
        m_cellEditor->setCursorPosition(int(initial.size()));
        m_formulaBar->setCursorPosition(int(initial.size()));
    }
}

void CrunchWindow::placeEditor()
{
    if (!m_edit.active) return;
    const QRect r = m_view->cellRect(m_edit.row, m_edit.col);
    if (r.isEmpty() || m_edit.sheet != m_sheet) {
        m_cellEditor->hide();
        return;
    }
    // the font of the cell, as the grid shows it
    const grid::GridSheet &gs = m_view->sheet();
    const grid::Cell *cell = gs.cellAt(m_edit.row, m_edit.col);
    const grid::CellLook &look = gs.looks().look(gs.styleOf(m_edit.row, m_edit.col, cell));
    QFont f(look.family.isEmpty() ? m_view->font().family() : look.family);
    f.setPixelSize(std::max(9, int(std::lround(look.pointSize * 96.0 / 72.0 * m_view->zoom()))));
    f.setBold(look.bold);
    f.setItalic(look.italic);
    m_cellEditor->setFont(f);
    // as wide as the cell, and wider as the text grows, as far as the edge of the grid
    const QRect vp = m_view->viewport()->rect();
    const int want = QFontMetrics(f).horizontalAdvance(m_cellEditor->text()) + 16;
    const int width = std::max(r.width(), std::min(want, std::max(r.width(), vp.right() - r.left())));
    m_cellEditor->setGeometry(r.left(), r.top(), width, r.height());
    m_cellEditor->show();
}

bool CrunchWindow::commitEdit(CellEditor::Way way)
{
    if (!m_edit.active) return true;
    const QString text = m_cellEditor->text();
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->type(m_edit.sheet, m_edit.row, m_edit.col, text);
    if (!o.ok) {
        // the entry stays as it was typed, with the cursor where the fault is
        QLineEdit *w = m_formulaBar->hasFocus() ? static_cast<QLineEdit *>(m_formulaBar) : static_cast<QLineEdit *>(m_cellEditor);
        notice(o.problemAt >= 0 ? tr("Formula") : tr("Edit"), o.problem);
        if (o.problemAt >= 0) w->setCursorPosition(std::min<int>(o.problemAt, int(text.size())));
        w->setFocus(Qt::OtherFocusReason);
        return false;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    finishEdit(way);
    return true;
}

void CrunchWindow::finishEdit(CellEditor::Way way)
{
    m_cellEditor->hide();
    const quint32 row = m_edit.row, col = m_edit.col;
    m_edit.active = false;
    m_view->setFocus(Qt::OtherFocusReason);
    using Way = CellEditor::Way;
    // Tab after Tab goes on along the row, and Enter then goes down to the column the Tabs began in
    if (way == Way::Tab || way == Way::ShiftTab) {
        if (m_tabOriginCol < 0 || m_tabRow != qint64(row)) {
            m_tabOriginCol = col;
            m_tabRow = row;
        }
    }
    const bool backToOrigin = way == Way::Enter && m_tabOriginCol >= 0 && m_tabRow == qint64(row);
    const qint64 origin = m_tabOriginCol;
    m_finishing = true;
    switch (way) {
    case Way::Stay: break;
    case Way::Enter: m_view->moveAfterEnter(false); break;
    case Way::ShiftEnter: m_view->moveAfterEnter(true); break;
    case Way::Tab: m_view->moveAfterTab(false); break;
    case Way::ShiftTab: m_view->moveAfterTab(true); break;
    case Way::Up: m_view->moveAfterArrow(-1, 0); break;
    case Way::Down: m_view->moveAfterArrow(1, 0); break;
    case Way::Left: m_view->moveAfterArrow(0, -1); break;
    case Way::Right: m_view->moveAfterArrow(0, 1); break;
    }
    if (backToOrigin && m_view->selection().isOneCell()) m_view->selectCell(m_view->selection().activeRow, origin);
    if (way != Way::Tab && way != Way::ShiftTab) m_tabOriginCol = m_tabRow = -1;
    else m_tabRow = m_view->selection().activeRow;
    m_finishing = false;
    updateBars();
}

void CrunchWindow::cancelEdit()
{
    if (!m_edit.active) return;
    m_cellEditor->hide();
    m_edit.active = false;
    m_view->setFocus(Qt::OtherFocusReason);
    updateBars();
}

void CrunchWindow::onCellsChanged(int sheet, const std::vector<jp::grid::ValueStore::Item> &cells)
{
    if (sheet == m_sheet && m_view->valuesChanged(sheet, cells)) updateBars();
    if (m_undoing && !cells.empty()) {
        // Undo and Redo show the cell they changed
        if (sheet != m_sheet) setActiveSheet(sheet);
        m_view->selectCell(cells.front().row, cells.front().col);
        m_view->ensureVisible(cells.front().row, cells.front().col);
    }
}

void CrunchWindow::clearSelection()
{
    if (m_edit.active || !editingAllowed()) return;
    const grid::Selection &sel = m_view->selection();
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->clear(m_sheet, sel.range.row1, sel.range.col1, sel.range.row2, sel.range.col2);
    if (!o.ok) notice(tr("Clear"), o.problem);
    else if (m_loader.changesMade() != before) m_calculating = true;
}

void CrunchWindow::undoRedo(bool redo)
{
    QUndoStack *u = undoStack();
    if (!u || m_edit.active || m_saving) return;
    if (redo ? !u->canRedo() : !u->canUndo()) return;
    const qint64 before = m_loader.changesMade();
    const QString what = redo ? u->redoText() : u->undoText();
    m_undoing = true;
    if (redo) u->redo();
    else u->undo();
    m_undoing = false;
    if (m_loader.changesMade() != before) m_calculating = true;
    setStatus(redo ? tr("Redone: %1").arg(what) : tr("Undone: %1").arg(what));
}

} // namespace jp
