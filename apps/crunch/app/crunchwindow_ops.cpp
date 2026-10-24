// The commands on a block of cells that are not typing: AutoSum, fill, the clipboard, inserting and deleting, the sizes of rows and columns. Each asks the edit session for one step.
#include "app/crunchwindow.h"

#include "app/crunchinfo.h"
#include "sheet/cellref.h"

#include <QAction>

namespace jp {

using jp::office::sheet::CellRange;

void CrunchWindow::createOpsActions()
{
    // AutoSum, and the functions of its menu
    mk("edit.autosum", tr("AutoSum"), "sigma", crunch::autoSumKey(), [this] { autoSum(QStringLiteral("SUM")); });
    mk("edit.autosum.sum", tr("Sum"), "sigma", QKeySequence(), [this] { autoSum(QStringLiteral("SUM")); });
    mk("edit.autosum.average", tr("Average"), "", QKeySequence(), [this] { autoSum(QStringLiteral("AVERAGE")); });
    mk("edit.autosum.count", tr("Count Numbers"), "", QKeySequence(), [this] { autoSum(QStringLiteral("COUNT")); });
    mk("edit.autosum.max", tr("Max"), "", QKeySequence(), [this] { autoSum(QStringLiteral("MAX")); });
    mk("edit.autosum.min", tr("Min"), "", QKeySequence(), [this] { autoSum(QStringLiteral("MIN")); });
    m_opsActions << QStringLiteral("edit.autosum") << QStringLiteral("edit.autosum.sum") << QStringLiteral("edit.autosum.average") << QStringLiteral("edit.autosum.count")
                 << QStringLiteral("edit.autosum.max") << QStringLiteral("edit.autosum.min");
    updateEditActions();
}

// AutoSum (and Average, Count Numbers, Max, and Min): with one cell selected the formula is put in the cell to be edited, with the range it guessed; with a block of cells the formulas
// are put in at once.
void CrunchWindow::autoSum(const QString &function)
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return;
    const grid::Selection &sel = m_view->selection();
    CellRange range;
    range.valid = true;
    range.row1 = quint32(std::max<qint64>(0, sel.range.row1));
    range.col1 = quint32(std::max<qint64>(0, sel.range.col1));
    range.row2 = quint32(std::max<qint64>(0, sel.range.row2));
    range.col2 = quint32(std::max<qint64>(0, sel.range.col2));
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::AutoSumResult r = m_session->autoSum(m_sheet, range, function);
    if (!r.outcome.ok) {
        notice(tr("AutoSum"), r.outcome.problem);
        return;
    }
    if (r.kind == crunch::EditSession::AutoSumResult::Kind::Edit) {
        m_view->selectCell(r.row, r.col);
        beginEdit(r.text, CellEditor::Mode::Editing, false);
        if (m_edit.active) {
            const int at = int(r.text.size()) - r.cursorBack;
            m_cellEditor->setCursorPosition(at);
            m_formulaBar->setCursorPosition(at);
        }
    } else if (r.kind == crunch::EditSession::AutoSumResult::Kind::Done && m_loader.changesMade() != before) {
        m_calculating = true;
    }
}

} // namespace jp
