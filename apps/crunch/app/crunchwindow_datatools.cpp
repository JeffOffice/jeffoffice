// The Data tab's tools that change cells: Remove Duplicates and Text to Columns, and the button that opens a text file (the same as Open Text File on the File tab).
#include "app/crunchwindow.h"

#include "app/datatoolsdialogs.h"
#include "app/dataformdialog.h"
#include "app/editlinksdialog.h"
#include "app/linkprogress.h"
#include "app/plaintext.h"
#include "app/removeduplicatesdialog.h"
#include "app/trust.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>

namespace jp {

using jp::office::sheet::CellRange;

void CrunchWindow::createDataToolsActions()
{
    mk("data.editLinks", tr("Edit Links…"), "link", QKeySequence(), [this] { editLinksCommand(); }, false, false);
    m_opsActions << QStringLiteral("data.editLinks");
    mk("data.fromText", tr("From Text/CSV"), "file-text", QKeySequence(), [this] { act("file.openText")->trigger(); });
    mk("data.removeDuplicates", tr("Remove Duplicates"), "list-x", QKeySequence(), [this] { removeDuplicatesDialog(); });
    mk("data.textToColumns", tr("Text to Columns"), "table-columns-split", QKeySequence(), [this] { textToColumnsDialog(); });
    mk("data.form", tr("Form…"), "clipboard-list", QKeySequence(), [this] { dataFormCommand(); });
    for (const char *id : {"data.removeDuplicates", "data.textToColumns", "data.form"}) m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

// Data > Form: the list round the cell with the cursor (the table it is in, or the block of cells), one record at a time. Each record changed, added, or deleted is one step of Undo.
void CrunchWindow::dataFormCommand()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session || !m_loaded) return;
    const grid::Selection &sel = m_view->selection();
    const QString sheetName = m_loaded->book->info().sheets.value(m_sheet).name;
    DataFormDialog dialog(m_session.get(), m_sheet, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)), sheetName.isEmpty() ? tr("Form") : sheetName, this);
    if (!dialog.ready()) {
        notice(tr("Form"), dialog.problem());
        return;
    }
    qint64 before = m_loader.changesMade();
    connect(&dialog, &DataFormDialog::recordsChanged, this, [this, &before] {
        if (m_loader.changesMade() != before) m_calculating = true;
        before = m_loader.changesMade();
        setWindowModified(true);
    });
    if (m_dataFormAsker) m_dataFormAsker(&dialog);
    else dialog.exec();
}

// Data > Edit Links: the workbooks this one reads.
void CrunchWindow::editLinksCommand()
{
    if (!m_session || !m_loaded || !commitEdit()) return;
    EditLinksDialog dialog(m_session.get(), m_path.isEmpty() ? QString() : QFileInfo(m_path).absolutePath(), this);
    connect(&dialog, &EditLinksDialog::openRequested, this, [this](const QString &path) { openLinkSource(path); });
    connect(&dialog, &EditLinksDialog::linksBroken, this, [this] {
        m_protectionChanged = true; invalidateRecovery();
        setWindowModified(true);
        updateEditActions();
    });
    dialog.exec();
}

// What the person has chosen for the links of a file as it opens (the file's own word is a question). Options > Security decides for a file whose Startup prompt the person has not set here: "Never" leaves
// the stored values (as a file that says "never" does), and "Update them" updates without asking (a file that says "never" is still left, and a source on another computer is never read).
crunch::EditSession::UpdateLinks CrunchWindow::linksOpeningMode() const
{
    crunch::EditSession::UpdateLinks mode = m_session->openingChoice();
    if (!m_session->updateLinksChosen() && mode != crunch::EditSession::UpdateLinks::Never) {
        const jp::trust::Choice trusted = jp::trust::linkedFiles();
        if (trusted == jp::trust::Choice::Never) mode = crunch::EditSession::UpdateLinks::Never;
        else if (trusted == jp::trust::Choice::Yes) mode = crunch::EditSession::UpdateLinks::Always;
    }
    return mode;
}

// Whether the files that pivot tables are made from may be read as the workbook opens: the same question and the same choice as for its links.
bool CrunchWindow::pivotFilesOnOpenAllowed(int tables)
{
    const crunch::EditSession::UpdateLinks mode = linksOpeningMode();
    if (mode == crunch::EditSession::UpdateLinks::Never) return false;
    if (mode == crunch::EditSession::UpdateLinks::Always) return true;
    if (m_linksAsker) return m_linksAsker(tables, 0);
    return plainQuestion(this, tr("Pivot Tables"), tr("%n pivot table(s) in this workbook read their data from other files. Read those files now?", nullptr, tables)) == QMessageBox::Yes;
}

// When a workbook that reads others has been opened and worked out: the choice the file has (Startup prompt in Edit Links) decides. Only links whose file is on this computer are asked about.
void CrunchWindow::linksOnOpen()
{
    if (!m_session || !m_loaded || isReadOnly()) return;
    // (no file is touched before the person says yes: the question is made from the link parts of the workbook alone, since asking whether a file is there would reach a computer on a network)
    crunch::EditSession::UpdateLinks mode = linksOpeningMode();
    if (mode == crunch::EditSession::UpdateLinks::Never) return;
    const std::vector<crunch::EditSession::ExternalSource> sources = m_session->externalLinkSources(false);
    if (sources.empty()) return;
    if (mode == crunch::EditSession::UpdateLinks::Ask) {
        const int cells = m_session->externalLinkCells();
        const bool yes = m_linksAsker ? m_linksAsker(int(sources.size()), cells)
                                      : plainQuestion(this, tr("Links"), tr("This workbook reads values from other workbooks. Update them now? You can change this in Edit Links, under Data.")) == QMessageBox::Yes;
        if (!yes) return;
    }
    // the files on this computer are read on a thread of its own, behind a window with Cancel (one on another computer is left until it is asked for in Edit Links)
    if (jp::office::sheet::ExternalBooks *links = m_session->externals()) {
        std::vector<int> local;
        for (const crunch::EditSession::ExternalSource &s : sources)
            if (s.local && !s.network) local.push_back(s.index);
        if (!runLinkReads(this, links, tr("Reading the workbooks the links point to..."), [&] {
                for (const int index : local) links->probe(index);
            })) {
            setStatus(tr("Links not updated."));
            return;
        }
    }
    int updated = 0;
    const crunch::EditSession::Outcome o = m_session->updateAllExternalLinks(&updated);
    updateEditActions();
    setStatus(o.ok ? tr("Links updated: %1 cells.").arg(updated) : o.problem.section(QLatin1Char('\n'), 0, 0));
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
