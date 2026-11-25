// Data > Get Data > From File, and Data > Refresh and Refresh All for the tables it makes: the choice is made in a window with a preview, the file is read on a thread of its own behind a box with Cancel,
// and the rows are put on the sheet as a table in one step of Undo. A refused or cancelled load changes nothing.
#include "app/crunchwindow.h"

#include "app/getdata.h"
#include "sheet/cellref.h"
#include "sheet/datasource.h"

#include "app/plaintext.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>

#include <atomic>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::DataConnection;

void CrunchWindow::createGetDataActions()
{
    mk("data.fromFile", tr("From File…"), "database", QKeySequence(), [this] { getDataCommand(); });
    mk("data.refresh", tr("Refresh"), "refresh-cw", QKeySequence(), [this] { refreshDataCommand(false); });
    mk("data.refreshAll", tr("Refresh All"), "refresh-ccw", QKeySequence(), [this] { refreshDataCommand(true); });
    for (const char *id : {"data.fromFile", "data.refresh", "data.refreshAll"}) m_opsActions << QString::fromLatin1(id);
}

void CrunchWindow::getDataCommand()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed() || !m_session || !m_loaded) return;
    const grid::Selection &sel = m_view->selection();
    GetDataRequest req;
    req.place = jp::office::sheet::cellName(quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
    if (m_getDataAsker) {
        if (!m_getDataAsker(&req)) return;
    } else {
        QPointer<GetDataDialog> dialog = new GetDataDialog(req.place, this);
        const bool accepted = dialog->exec() == QDialog::Accepted;
        if (accepted && dialog) req = dialog->request();
        delete dialog;
        if (!accepted) return;
    }
    loadGetData(req);
}

bool CrunchWindow::loadGetData(const GetDataRequest &req)
{
    const QString title = tr("Get Data");
    quint32 row = 0, col = 0;
    if (!jp::office::sheet::parseCellRef(req.place.trimmed().remove(QLatin1Char('$')), &row, &col)) {
        notice(title, tr("That is not a cell. Write it as B2."));
        return false;
    }
    if (req.conn.kind.isEmpty()) {
        notice(title, tr("That file is not JSON, XML, or a workbook."));
        return false;
    }
    GetDataFetch result;
    std::atomic<bool> cancel{false};
    QPointer<CrunchWindow> self(this);
    const bool finished = runGetDataWork(this, tr("Reading the file…"), &cancel, [&] { result = fetchGetData(req.conn, 0, [&cancel] { return cancel.load(); }); });
    if (!self) return false;
    if (!finished) {
        setStatus(tr("The read was stopped. Nothing was changed."));
        return false;
    }
    if (!result.ok) {
        notice(title, result.why + QLatin1Char(' ') + tr("Nothing was changed."));
        return false;
    }
    // (the file may have taken a while: the workbook is looked at again)
    if (!m_session || !editingAllowed()) return false;
    bool replace = false;
    CellRange where;
    where.valid = true;
    where.row1 = row;
    where.col1 = col;
    where.row2 = row + quint32(result.rows.rows.size());
    where.col2 = col + quint32(std::max<qsizetype>(1, result.rows.header.size())) - 1;
    if (where.row2 < row || where.col2 < col) return false;
    if (m_session->cellsHoldData(m_sheet, where)) {
        const int cells = int(std::min<qint64>(qint64(where.row2 - where.row1 + 1) * (where.col2 - where.col1 + 1), 2000000000));
        const bool yes = m_getDataReplaceAsker ? m_getDataReplaceAsker(cells)
                                               : plainQuestion(this, title, tr("The table would go over cells that hold something (%1 cells in all). Replace them?").arg(cells), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Yes;
        if (!yes) return false;
        replace = true;
    }
    DataConnection conn = req.conn;
    const auto outcome = m_session->placeDataTable(m_sheet, row, col, result.rows, result.kinds, conn.text(), replace, nullptr);
    if (!outcome.ok) {
        notice(title, outcome.problem + QLatin1Char(' ') + tr("Nothing was changed."));
        return false;
    }
    if (!result.rows.notes.isEmpty()) setStatus(result.rows.notes.join(QLatin1Char(' ')));
    return true;
}

// Refresh: the table at the cursor, read again from its file. Refresh All: every table that came from a file, then the pivot tables. Each table is one step of Undo.
void CrunchWindow::refreshDataCommand(bool all)
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!editingAllowed() || !m_session || !m_loaded) return;
    const QString title = all ? tr("Refresh All") : tr("Refresh");
    QVector<int> tables;
    const auto &book = m_session->workbook()->tables();
    if (all) {
        for (int i = 0; i < book.size(); ++i)
            if (!book[i].dataSource.isEmpty()) tables << i;
    } else {
        const int t = selectedTable();
        if (t < 0 || book[t].dataSource.isEmpty()) {
            notice(title, tr("Click a cell in a table that was made by Get Data first."), false);
            return;
        }
        tables << t;
    }
    QStringList problems;
    int done = 0;
    QPointer<CrunchWindow> self(this);
    for (int t : tables) {
        if (!self || !m_session || t >= m_session->workbook()->tables().size()) break;
        const jp::office::sheet::Table table = m_session->workbook()->tables()[t];
        DataConnection conn;
        if (!DataConnection::parse(table.dataSource, &conn)) {
            problems << tr("%1: the link to the file can't be read, so nothing was changed.").arg(table.name);
            continue;
        }
        GetDataFetch result;
        std::atomic<bool> cancel{false};
        const bool finished = runGetDataWork(this, tr("Reading %1…").arg(QFileInfo(conn.path).fileName()), &cancel, [&] { result = fetchGetData(conn, 0, [&cancel] { return cancel.load(); }); });
        if (!self) return;
        if (!finished) {
            problems << tr("The read was stopped. The tables after %1 were not read.").arg(table.name);
            break;
        }
        if (!result.ok) {
            problems << tr("%1: %2 Nothing was changed.").arg(table.name, result.why);
            continue;
        }
        const auto outcome = m_session->refreshDataTable(t, result.rows, result.kinds);
        if (!outcome.ok) {
            problems << tr("%1: %2").arg(table.name, outcome.problem);
            continue;
        }
        ++done;
        m_session->finishFills();
    }
    if (all && self && m_session && m_session->workbook()->pivots().tableCount() > 0) act("pivot.refreshAll")->trigger();
    if (!problems.isEmpty()) notice(title, problems.join(QLatin1Char('\n')));
    else if (self) setStatus(done > 0 ? tr("The data was read again from its files.") : tr("Nothing needed to be read again."));
}

}   // namespace jp
