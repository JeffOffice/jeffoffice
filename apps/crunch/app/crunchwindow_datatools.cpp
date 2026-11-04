// The Data tab's tools that change cells: Remove Duplicates and Text to Columns, and the button that opens a text file (the same as Open Text File on the File tab).
#include "app/crunchwindow.h"

#include "app/datatoolsdialogs.h"
#include "app/removeduplicatesdialog.h"
#include "sheet/cellref.h"

#include <QAction>

namespace jp {

using jp::office::sheet::CellRange;

void CrunchWindow::createDataToolsActions()
{
    mk("data.fromText", tr("From Text/CSV"), "file-text", QKeySequence(), [this] { act("file.openText")->trigger(); });
    mk("data.removeDuplicates", tr("Remove Duplicates"), "list-x", QKeySequence(), [this] { removeDuplicatesDialog(); });
    mk("data.textToColumns", tr("Text to Columns"), "table-columns-split", QKeySequence(), [this] { textToColumnsDialog(); });
    for (const char *id : {"data.removeDuplicates", "data.textToColumns"}) m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

void CrunchWindow::removeDuplicatesDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session) return;
    CellRange range = selectedRange();
    // one cell: the block of data around it
    if (range.row1 == range.row2 && range.col1 == range.col2) {
        bool truncated = false;
        const CellRange region = crunch::currentRegion(*m_session, m_sheet, range.row1, range.col1, &truncated);
        if (truncated || region.cellCount() <= 1) {
            notice(tr("Remove Duplicates"), tr("Select the cells to look in, or a cell in a block of data."));
            return;
        }
        range = region;
    }
    const bool looksLikeHeader = m_session->looksLikeHeader(m_sheet, range);
    QStringList headerNames, plainNames;
    for (quint32 col = range.col1; col <= range.col2; ++col) {
        const jp::calc::Value top = m_session->valueAt(m_sheet, range.row1, col);
        plainNames << tr("Column %1").arg(jp::office::sheet::columnName(col));
        headerNames << (top.isText() && !top.text().isEmpty() ? top.text() : plainNames.last());
    }
    QList<int> chosen;
    bool header = looksLikeHeader;
    if (m_removeDuplicatesAsker) {
        if (!m_removeDuplicatesAsker(looksLikeHeader ? headerNames : plainNames, &chosen)) return;
    } else {
        RemoveDuplicatesDialog dialog(headerNames, plainNames, looksLikeHeader, this);
        if (dialog.exec() != QDialog::Accepted) return;
        chosen = dialog.chosen();
        header = dialog.hasHeader();
    }
    if (chosen.isEmpty()) {
        notice(tr("Remove Duplicates"), tr("Check at least one column to compare."));
        return;
    }
    int removed = 0;
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->removeDuplicates(m_sheet, range, header, chosen, &removed);
    if (!o.ok) {
        notice(tr("Remove Duplicates"), o.problem);
        return;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    const qint64 remaining = qint64(range.row2 - range.row1 + 1) - (header ? 1 : 0) - removed;
    notice(tr("Remove Duplicates"), removed == 0 ? tr("No row repeats an earlier one.") : tr("Repeated rows removed: %1. Unique rows left: %2.").arg(removed).arg(remaining), false);
}

bool CrunchWindow::removeDuplicatesIn(const std::vector<int> &columns, bool hasHeader)
{
    if (!m_session || !editingAllowed()) return false;
    const qint64 before = m_loader.changesMade();
    QList<int> chosen;
    for (const int c : columns) chosen << c;
    const crunch::EditSession::Outcome o = m_session->removeDuplicates(m_sheet, selectedRange(), hasHeader, chosen);
    if (!o.ok) return false;
    if (m_loader.changesMade() != before) m_calculating = true;
    return true;
}

void CrunchWindow::textToColumnsDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session) return;
    const CellRange range = selectedRange();
    if (range.col1 != range.col2) {
        notice(tr("Text to Columns"), tr("Select the cells of one column to split."));
        return;
    }
    QStringList lines;
    for (quint32 row = range.row1; row <= range.row2 && lines.size() < 6; ++row) {
        const jp::calc::Value v = m_session->valueAt(m_sheet, row, range.col1);
        lines << (v.isText() ? v.text() : m_session->editText(m_sheet, row, range.col1));
    }
    TextToColumnsDialog dialog(lines, jp::office::sheet::cellName(range.row1, range.col1), this);
    if (dialog.exec() != QDialog::Accepted) return;
    textToColumnsIn(dialog.spec(), dialog.destination());
}

bool CrunchWindow::textToColumnsIn(const crunch::EditSession::SplitSpec &spec, const QString &destination)
{
    if (!m_session || !editingAllowed()) return false;
    const CellRange range = selectedRange();
    quint32 row = range.row1, col = range.col1;
    if (!destination.isEmpty() && !jp::office::sheet::parseCellRef(destination, &row, &col)) {
        notice(tr("Text to Columns"), tr("That is not a cell. Write the first cell for the pieces, such as B2."));
        return false;
    }
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->textToColumns(m_sheet, range, spec, row, col);
    if (!o.ok) {
        notice(tr("Text to Columns"), o.problem);
        return false;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    return true;
}

} // namespace jp
