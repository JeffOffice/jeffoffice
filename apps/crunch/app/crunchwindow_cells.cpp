// Rows, columns, and cells inserted and deleted: the ribbon's Insert and Delete, the right-click menus of cells and of the headers, and Ctrl++ and Ctrl+-. Each asks the edit session for one
// step; the window makes its picture of the sheet again when the cells have moved.
#include "app/crunchwindow.h"

#include "sheet/cellref.h"

#include <QAction>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
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
    // sizes
    mk("cells.format", tr("Format"), "table-properties", QKeySequence(), [] {});
    mk("format.rowHeight", tr("Row Height…"), "", QKeySequence(), [this] { setSizeDialog(false); });
    mk("format.columnWidth", tr("Column Width…"), "", QKeySequence(), [this] { setSizeDialog(true); });
    mk("format.fitRow", tr("AutoFit Row Height"), "", QKeySequence(), [this] { fitSelection(false); });
    mk("format.fitColumn", tr("AutoFit Column Width"), "", QKeySequence(), [this] { fitSelection(true); });
    mk("format.hideRows", tr("Hide Rows"), "", QKeySequence(Qt::CTRL | Qt::Key_9), [this] { hideSelection(false, true); });
    mk("format.hideColumns", tr("Hide Columns"), "", QKeySequence(Qt::CTRL | Qt::Key_0), [this] { hideSelection(true, true); });
    mk("format.unhideRows", tr("Unhide Rows"), "", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_ParenLeft), [this] { hideSelection(false, false); });
    mk("format.unhideColumns", tr("Unhide Columns"), "", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_ParenRight), [this] { hideSelection(true, false); });
    for (const char *id : {"cells.format", "format.rowHeight", "format.columnWidth", "format.fitRow", "format.fitColumn", "format.hideRows", "format.hideColumns", "format.unhideRows", "format.unhideColumns"})
        m_opsActions << QString::fromLatin1(id);
    connect(m_view, &grid::GridView::resizeRequested, this, [this](bool column, qint64 index, int size) { resizeDragged(column, index, size); });
    connect(m_view, &grid::GridView::fitRequested, this, [this](bool column, qint64 index) { fitDragged(column, index); });
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

// ---- the width of columns and the height of rows

void CrunchWindow::selectedLines(bool column, quint32 *first, quint32 *last) const
{
    const grid::GridRange r = m_view->selection().range;
    *first = quint32(std::max<qint64>(0, column ? r.col1 : r.row1));
    *last = quint32(std::max<qint64>(0, column ? r.col2 : r.row2));
}

void CrunchWindow::onSizesChanged(int sheet)
{
    if (!m_loaded) return;
    if (sheet >= 0 && size_t(sheet) < m_loaded->geometry.size()) m_loaded->geometry[size_t(sheet)].reset();
    if (sheet == m_sheet) {
        showSheet(m_sheet, true);
        placeEditor();
    }
}

// A border dragged: the whole selection of columns (rows) if the border is the end of one of them, else the one column (row).
void CrunchWindow::resizeDragged(bool column, qint64 index, int size)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const grid::Selection &sel = m_view->selection();
    quint32 first = quint32(index), last = quint32(index);
    const bool whole = column ? (sel.whole == grid::Selection::Whole::Columns || sel.whole == grid::Selection::Whole::All) : (sel.whole == grid::Selection::Whole::Rows || sel.whole == grid::Selection::Whole::All);
    if (whole && index >= (column ? sel.range.col1 : sel.range.row1) && index <= (column ? sel.range.col2 : sel.range.row2)) selectedLines(column, &first, &last);
    crunch::EditSession::SizeChange by;
    if (size <= 0) {
        by.setHidden = true;
        by.hidden = true;
    } else {
        by.setSize = true;
        by.size = column ? grid::GridSheet::widthForPixels(size, m_view->sheet().looks().maxDigitWidth()) : grid::GridSheet::pointsForPixels(size);
    }
    const crunch::EditSession::Outcome o = column ? m_session->resizeColumns(m_sheet, first, last, by, size <= 0 ? tr("Hiding columns") : tr("Changing column width"))
                                                   : m_session->resizeRows(m_sheet, first, last, by, size <= 0 ? tr("Hiding rows") : tr("Changing row height"));
    if (!o.ok) notice(column ? tr("Column Width") : tr("Row Height"), o.problem);
}

void CrunchWindow::setSizeDialog(bool column)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    quint32 first = 0, last = 0;
    selectedLines(column, &first, &last);
    const int mdw = m_view->sheet().looks().maxDigitWidth();
    SizeRequest request;
    request.column = column;
    if (column) request.value = grid::GridSheet::shownWidth(m_view->sheet().columnWidth(first) / double(std::max(1, mdw)), mdw);
    else request.value = grid::GridSheet::pointsForPixels(m_view->sheet().layout().rows.size(first));
    if (m_sizeAsker) {
        if (!m_sizeAsker(&request)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(column ? tr("Column Width") : tr("Row Height"));
        auto *form = new QFormLayout;
        auto *layout = new QVBoxLayout(&dialog);
        layout->addLayout(form);
        auto *value = new QDoubleSpinBox;
        value->setRange(0, column ? 255 : 409.5);
        value->setDecimals(2);
        value->setValue(request.value);
        form->addRow(column ? tr("Column width") : tr("Row height"), value);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        request.value = value->value();
    }
    crunch::EditSession::SizeChange by;
    if (request.value <= 0) {
        by.setHidden = true;
        by.hidden = true;
    } else {
        by.setSize = true;
        by.size = column ? grid::GridSheet::widthOfShown(request.value, mdw) : request.value;
        by.setHidden = true;   // (a size asked for shows what was hidden)
        by.hidden = false;
    }
    const crunch::EditSession::Outcome o = column ? m_session->resizeColumns(m_sheet, first, last, by, request.value <= 0 ? tr("Hiding columns") : tr("Changing column width"))
                                                   : m_session->resizeRows(m_sheet, first, last, by, request.value <= 0 ? tr("Hiding rows") : tr("Changing row height"));
    if (!o.ok) notice(column ? tr("Column Width") : tr("Row Height"), o.problem);
}

// AutoFit: the columns (rows) of the selection each get the size that holds what is in them (one that holds nothing keeps its size).
void CrunchWindow::fitSelection(bool column)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    quint32 first = 0, last = 0;
    selectedLines(column, &first, &last);
    std::vector<std::pair<quint32, double>> sizes;
    for (quint32 i = first; i <= last && sizes.size() < 20000; ++i) {
        const double size = column ? m_view->sheet().fitWidth(i) : m_view->sheet().fitHeight(i);
        if (size > 0) sizes.push_back({i, size});
        if (i == 0xFFFFFFFFu) break;
    }
    if (sizes.empty()) return;
    const crunch::EditSession::Outcome o = column ? m_session->fitColumns(m_sheet, sizes) : m_session->fitRows(m_sheet, sizes);
    if (!o.ok) notice(column ? tr("Column Width") : tr("Row Height"), o.problem);
}

void CrunchWindow::fitDragged(bool column, qint64 index)
{
    // a border at the end of a column (row) of the selection fits all of the selection's; any other fits that one
    const grid::Selection &sel = m_view->selection();
    const bool whole = column ? (sel.whole == grid::Selection::Whole::Columns || sel.whole == grid::Selection::Whole::All) : (sel.whole == grid::Selection::Whole::Rows || sel.whole == grid::Selection::Whole::All);
    if (!(whole && index >= (column ? sel.range.col1 : sel.range.row1) && index <= (column ? sel.range.col2 : sel.range.row2))) {
        if (column) m_view->selectColumns(index, index);
        else m_view->selectRows(index, index);
    }
    fitSelection(column);
}

void CrunchWindow::hideSelection(bool column, bool hide)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    quint32 first = 0, last = 0;
    selectedLines(column, &first, &last);
    if (!hide) {
        // the hidden ones around the selection show too: the unhiding is for what the selection covers and one more at each end
        if (first > 0) --first;
        if (last + 1 < (column ? jp::office::sheet::kMaxColumns : jp::office::sheet::kMaxRows)) ++last;
    }
    crunch::EditSession::SizeChange by;
    by.setHidden = true;
    by.hidden = hide;
    const crunch::EditSession::Outcome o = column ? m_session->resizeColumns(m_sheet, first, last, by, hide ? tr("Hiding columns") : tr("Unhiding columns"))
                                                   : m_session->resizeRows(m_sheet, first, last, by, hide ? tr("Hiding rows") : tr("Unhiding rows"));
    if (!o.ok) notice(column ? tr("Columns") : tr("Rows"), o.problem);
}

// A number just typed in a column that has the width it began with widens the column to show it.
void CrunchWindow::widenForEntry(int sheet, quint32 row, quint32 col)
{
    CellRange one;
    one.valid = true;
    one.row1 = one.row2 = row;
    one.col1 = one.col2 = col;
    widenForCells(sheet, one);
}

void CrunchWindow::showContextMenu(const QPoint &globalPos)
{
    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->setObjectName(QStringLiteral("cellMenu"));
    for (const char *id : {"edit.cut", "edit.copy", "edit.paste"}) menu->addAction(act(QString::fromLatin1(id)));
    menu->addAction(act("paste.special"));
    menu->addSeparator();
    const grid::Selection::Whole whole = m_view->selection().whole;
    if (whole == grid::Selection::Whole::Rows) {
        menu->addAction(act("cells.insert.rows"));
        menu->addAction(act("cells.delete.rows"));
        menu->addSeparator();
        menu->addAction(act("format.rowHeight"));
        menu->addAction(act("format.fitRow"));
        menu->addAction(act("format.hideRows"));
        menu->addAction(act("format.unhideRows"));
    } else if (whole == grid::Selection::Whole::Columns) {
        menu->addAction(act("cells.insert.columns"));
        menu->addAction(act("cells.delete.columns"));
        menu->addSeparator();
        menu->addAction(act("format.columnWidth"));
        menu->addAction(act("format.fitColumn"));
        menu->addAction(act("format.hideColumns"));
        menu->addAction(act("format.unhideColumns"));
    } else {
        menu->addAction(act("cells.insert.cells"));
        menu->addAction(act("cells.delete.cells"));
    }
    menu->addSeparator();
    menu->addAction(act("edit.selectAll"));
    menu->popup(globalPos);
}

} // namespace jp
