// Rows, columns, and cells inserted and deleted: the ribbon's Insert and Delete, the right-click menus of cells and of the headers, and Ctrl++ and Ctrl+-. Each asks the edit session for one
// step; the window makes its picture of the sheet again when the cells have moved.
#include "app/crunchwindow.h"

#include "sheet/cellref.h"

#include <QAction>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMenu>
#include <QRadioButton>
#include <QVBoxLayout>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::StructEdit;

void CrunchWindow::createCellsActions()
{
    mk("cells.insert", tr("Insert"), "square-plus", QKeySequence(Qt::CTRL | Qt::Key_Plus), [this] { insertCommand(); });
    mk("cells.delete", tr("Delete"), "square-minus", QKeySequence(Qt::CTRL | Qt::Key_Minus), [this] { deleteCommand(); });
    // (the plus sign is Shift and the equals key on many keyboards)
    act("cells.insert")->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_Plus), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Plus), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Equal)});
    mk("cells.insert.cells", tr("Insert Cells…"), "", QKeySequence(), [this] { insertOrDeleteCells(true); });
    mk("cells.insert.rows", tr("Insert Sheet Rows"), "", QKeySequence(), [this] { insertLines(true); });
    mk("cells.insert.columns", tr("Insert Sheet Columns"), "", QKeySequence(), [this] { insertLines(false); });
    mk("cells.delete.cells", tr("Delete Cells…"), "", QKeySequence(), [this] { insertOrDeleteCells(false); });
    mk("cells.delete.rows", tr("Delete Sheet Rows"), "", QKeySequence(), [this] { deleteLines(true); });
    mk("cells.delete.columns", tr("Delete Sheet Columns"), "", QKeySequence(), [this] { deleteLines(false); });
    for (const char *id : {"cells.insert", "cells.insert.cells", "cells.insert.rows", "cells.insert.columns", "cells.delete", "cells.delete.cells", "cells.delete.rows", "cells.delete.columns"})
        m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

CellRange CrunchWindow::selectedRange() const
{
    const grid::GridRange r = m_view->selection().range;
    CellRange out;
    out.valid = true;
    out.row1 = quint32(std::max<qint64>(0, r.row1));
    out.col1 = quint32(std::max<qint64>(0, r.col1));
    out.row2 = quint32(std::max<qint64>(0, r.row2));
    out.col2 = quint32(std::max<qint64>(0, r.col2));
    return out;
}

// Insert (the button, and Ctrl++): whole rows or columns selected are inserted as they are; cells ask which way the cells after them move.
void CrunchWindow::insertCommand()
{
    const grid::Selection::Whole whole = m_view->selection().whole;
    if (whole == grid::Selection::Whole::Rows) insertLines(true);
    else if (whole == grid::Selection::Whole::Columns) insertLines(false);
    else insertOrDeleteCells(true);
}

void CrunchWindow::deleteCommand()
{
    const grid::Selection::Whole whole = m_view->selection().whole;
    if (whole == grid::Selection::Whole::Rows) deleteLines(true);
    else if (whole == grid::Selection::Whole::Columns) deleteLines(false);
    else insertOrDeleteCells(false);
}

void CrunchWindow::insertLines(bool rows)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const CellRange r = selectedRange();
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = rows ? m_session->insertRows(m_sheet, r.row1, r.row2 - r.row1 + 1) : m_session->insertColumns(m_sheet, r.col1, r.col2 - r.col1 + 1);
    if (!o.ok) {
        notice(tr("Insert"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
}

void CrunchWindow::deleteLines(bool rows)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const CellRange r = selectedRange();
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = rows ? m_session->deleteRows(m_sheet, r.row1, r.row2 - r.row1 + 1) : m_session->deleteColumns(m_sheet, r.col1, r.col2 - r.col1 + 1);
    if (!o.ok) {
        notice(tr("Delete"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
}

void CrunchWindow::insertOrDeleteCells(bool inserting)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    ShiftRequest request;
    request.inserting = inserting;
    request.way = inserting ? ShiftRequest::Way::Down : ShiftRequest::Way::Up;
    if (!askShift(&request)) return;
    using Way = ShiftRequest::Way;
    if (request.way == Way::Rows) {
        inserting ? insertLines(true) : deleteLines(true);
        return;
    }
    if (request.way == Way::Columns) {
        inserting ? insertLines(false) : deleteLines(false);
        return;
    }
    const CellRange block = selectedRange();
    const qint64 before = m_loader.changesMade();
    crunch::EditSession::Outcome o;
    if (inserting) o = m_session->insertCells(m_sheet, block, request.way == Way::Down);
    else o = m_session->deleteCells(m_sheet, block, request.way == Way::Up);
    if (!o.ok) {
        notice(inserting ? tr("Insert") : tr("Delete"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
}

bool CrunchWindow::askShift(ShiftRequest *request)
{
    if (m_shiftAsker) return m_shiftAsker(request);
    using Way = ShiftRequest::Way;
    QDialog dialog(this);
    dialog.setWindowTitle(request->inserting ? tr("Insert") : tr("Delete"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *group = new QButtonGroup(&dialog);
    struct Choice {
        Way way;
        QString text;
    };
    const QList<Choice> choices = request->inserting ? QList<Choice>{{Way::Right, tr("Shift cells right")}, {Way::Down, tr("Shift cells down")}, {Way::Rows, tr("Entire row")}, {Way::Columns, tr("Entire column")}}
                                                     : QList<Choice>{{Way::Left, tr("Shift cells left")}, {Way::Up, tr("Shift cells up")}, {Way::Rows, tr("Entire row")}, {Way::Columns, tr("Entire column")}};
    for (int i = 0; i < choices.size(); ++i) {
        auto *button = new QRadioButton(choices[i].text);
        button->setChecked(choices[i].way == request->way);
        group->addButton(button, i);
        layout->addWidget(button);
    }
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return false;
    request->way = choices[group->checkedId()].way;
    return true;
}

// Rows, columns, or cells were moved (by an edit, or by Undo or Redo): the picture of the sheet is made again from the cells, with the view as it was.
void CrunchWindow::onCellsMoved(const StructEdit &edit)
{
    if (!m_loaded) return;
    for (const int sheet : {edit.sheet, edit.toSheet})
        if (sheet >= 0 && size_t(sheet) < m_loaded->geometry.size()) m_loaded->geometry[size_t(sheet)].reset();   // (the one made ahead is of the sheet as it was)
    if (m_sheet == edit.sheet || (edit.kind == StructEdit::Kind::Move && m_sheet == edit.toSheet)) showSheet(m_sheet, true);
    updateBars();
}

void CrunchWindow::showContextMenu(const QPoint &globalPos)
{
    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->setObjectName(QStringLiteral("cellMenu"));
    for (const char *id : {"edit.cut", "edit.copy", "edit.paste"}) menu->addAction(act(QString::fromLatin1(id)));
    menu->addSeparator();
    const grid::Selection::Whole whole = m_view->selection().whole;
    if (whole == grid::Selection::Whole::Rows) {
        menu->addAction(act("cells.insert.rows"));
        menu->addAction(act("cells.delete.rows"));
    } else if (whole == grid::Selection::Whole::Columns) {
        menu->addAction(act("cells.insert.columns"));
        menu->addAction(act("cells.delete.columns"));
    } else {
        menu->addAction(act("cells.insert.cells"));
        menu->addAction(act("cells.delete.cells"));
    }
    menu->addSeparator();
    menu->addAction(act("edit.selectAll"));
    menu->popup(globalPos);
}

} // namespace jp
