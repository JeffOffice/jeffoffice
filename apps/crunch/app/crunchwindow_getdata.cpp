// Data > Get Data > From File, and Data > Refresh and Refresh All for the tables it makes: the choice is made in a window with a preview, the file is read on a thread of its own behind a box with Cancel,
// and the rows are put on the sheet as a table in one step of Undo. A refused or cancelled load changes nothing.
#include "app/crunchwindow.h"

#include "app/getdata.h"
#include "sheet/cellref.h"
#include "sheet/datasource.h"

#include "app/plaintext.h"
#include "core/appinfo.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>

#include <atomic>
#include <memory>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::DataConnection;

static QString getDataTrustKey(const QString &path)
{
    return jp::office::sheet::dataReadablePath(path);
}

bool CrunchWindow::getDataTrusted(const QString &path) const
{
    return m_getDataTrusted.contains(getDataTrustKey(path));
}

// A query or pivot table that came out of an opened workbook names a file of its own choosing: nothing is read until this person has seen the whole path and said yes. The yes is kept for this window only.
// `paths` are the places as they will be opened (links and ".." worked out); the box shows those, with anything that could fake the text shown as visible codes.
bool CrunchWindow::askToReadFiles(QWidget *parent, const QString &title, const QStringList &names, const QStringList &paths)
{
    const int shown = int(std::min<qsizetype>(paths.size(), 20));
    QString list;
    for (int i = 0; i < shown; ++i) list += (paths.size() == 1 ? QString() : getDataVisibleText(names[i]) + QStringLiteral(": ")) + getDataVisibleText(paths[i]) + QLatin1Char('\n');
    if (paths.size() > shown) list += tr("and %1 more").arg(paths.size() - shown) + QLatin1Char('\n');
    QString text;
    if (paths.size() == 1) text = tr("%1 in this workbook asks to read this file from this computer:").arg(getDataVisibleText(names.first()));
    else text = tr("Queries and pivot tables in this workbook ask to read these files from this computer:");
    text += QStringLiteral("\n\n") + list + QStringLiteral("\n") + tr("Read the file only if you trust where this workbook came from. Anything in it will be put in this workbook.");
    if (paths.size() > 1) text += QLatin1Char(' ') + tr("Read the Files reads every file named here, including any that are only counted.");
    if (m_getDataTrustAsker) return m_getDataTrustAsker(paths, text);
    jp::MessageBox box(QMessageBox::Warning, appInfo().name, title, QMessageBox::NoButton, parent);
    box.setObjectName(QStringLiteral("trustBox"));
    box.setText(text);
    box.setTextFormat(Qt::PlainText);
    QPushButton *no = box.addButton(tr("Don't Read"), QMessageBox::RejectRole);
    QPushButton *yes = box.addButton(paths.size() == 1 ? tr("Read the File") : tr("Read the Files"), QMessageBox::AcceptRole);
    box.setDefaultButton(no);
    box.setEscapeButton(no);
    box.exec();
    return box.clickedButton() == yes;
}

// The pivot tables' own refresh may read a file only if it is one this window has approved: the gate holds while the returned guard lives, then the session is as it was.
std::shared_ptr<void> CrunchWindow::armPivotFileGate()
{
    if (!m_session) return nullptr;
    QPointer<CrunchWindow> self(this);
    m_session->setPivotFileGate([self](const QString &path) { return self && self->m_getDataTrusted.contains(path); });
    return std::shared_ptr<void>(nullptr, [self](void *) {
        if (self && self->m_session) self->m_session->setPivotFileGate(nullptr);
    });
}

// Before a Refresh: the files of other workbooks that the pivot tables `pivots` (every one when empty) would read, asked about together with the files of queries (`names`, `paths`) in ONE box.
// False when the person said not to read.
bool CrunchWindow::approveFiles(QWidget *parent, const QString &title, const std::vector<int> &pivots, bool withPivots, QStringList names, QStringList paths)
{
    if (withPivots && m_session)
        for (const auto &f : m_session->pivotExternalFiles(pivots)) {
            if (m_getDataTrusted.contains(f.second) || paths.contains(f.second)) continue;
            names << f.first;
            paths << f.second;
        }
    if (paths.isEmpty()) return true;
    if (!askToReadFiles(parent, title, names, paths)) return false;
    for (const QString &p : paths) m_getDataTrusted.insert(p);
    return true;
}

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
    const bool finished = runGetDataWork(this, tr("Reading the file…"), &cancel, [&] {
        if (m_getDataHold) m_getDataHold(cancel);
        result = fetchGetData(req.conn, 0, [&cancel] { return cancel.load(); });
    });
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
    conn.columns = m_session->dataTableNames(result.rows.header);
    const auto outcome = m_session->placeDataTable(m_sheet, row, col, result.rows, result.kinds, conn.text(), replace, nullptr);
    if (!outcome.ok) {
        notice(title, outcome.problem + QLatin1Char(' ') + tr("Nothing was changed."));
        return false;
    }
    m_getDataTrusted.insert(getDataTrustKey(req.conn.path));   // (made here: the person chose this file)
    if (!result.rows.notes.isEmpty()) setStatus(result.rows.notes.join(QLatin1Char(' ')));
    return true;
}

// Refresh: the table at the cursor, read again from its file. Refresh All: every table that came from a file, then the pivot tables. Refresh is one step of Undo; so are all the tables of one Refresh All.
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
    {
        QStringList names, paths;
        QVector<int> unasked;
        for (int t : tables) {
            const auto &table = book[t];
            DataConnection conn;
            if (!DataConnection::parse(table.dataSource, &conn) || conn.path.isEmpty() || getDataTrusted(conn.path)) continue;
            const QString key = getDataTrustKey(conn.path);
            unasked << t;
            if (paths.contains(key)) continue;
            names << table.name;
            paths << key;
        }
        if (!approveFiles(this, title, {}, all, names, paths)) {
            for (int t : unasked) tables.removeAll(t);
            if (tables.isEmpty() && !all) {
                setStatus(tr("Nothing was read."));
                return;
            }
        }
    }
    QStringList problems;
    int done = 0;
    bool grouped = false;
    QStringList notes;
    QPointer<CrunchWindow> self(this);
    for (int t : tables) {
        if (!self || !m_session || t >= m_session->workbook()->tables().size()) break;
        const jp::office::sheet::Table table = m_session->workbook()->tables()[t];
        DataConnection conn;
        if (!DataConnection::parse(table.dataSource, &conn)) {
            problems << tr("%1: the link to the file can't be read, so nothing was changed.").arg(table.name);
            continue;
        }
        // (asked about again here, for the very place about to be opened, and that place is what is read)
        const QString file = getDataTrustKey(conn.path);
        if (!m_getDataTrusted.contains(file)) {
            problems << tr("%1: the file was not read, because it was not approved.").arg(table.name);
            continue;
        }
        DataConnection toRead = conn;
        toRead.path = file;
        GetDataFetch result;
        std::atomic<bool> cancel{false};
        const auto hold = m_getDataHold;   // (the thread uses a copy: it must not reach into a window that may be closed meanwhile)
        const bool finished = runGetDataWork(this, tr("Reading %1…").arg(QFileInfo(conn.path).fileName()), &cancel, [&] {
            if (hold) hold(cancel);
            result = fetchGetData(toRead, 0, [&cancel] { return cancel.load(); });
        });
        if (!self) return;
        if (!finished) {
            problems << (tables.size() == 1 ? tr("The read was stopped. Nothing was changed.") : tr("The read was stopped. The tables after %1 were not read.").arg(table.name));
            break;
        }
        if (!result.ok) {
            problems << tr("%1: %2 Nothing was changed.").arg(table.name, result.why);
            continue;
        }
        const auto check = m_session->refreshDataTable(t, result.rows, result.kinds, conn.columns, true);
        if (!check.ok) {
            problems << tr("%1: %2").arg(table.name, check.problem);
            continue;
        }
        // (all the tables of one Refresh All are one step of Undo: it is begun when the first one is to be put in)
        if (all && !grouped) {
            m_session->undoStack()->beginMacro(tr("Refresh All"));
            grouped = true;
        }
        const auto outcome = m_session->refreshDataTable(t, result.rows, result.kinds, conn.columns);
        if (!outcome.ok) {
            problems << tr("%1: %2").arg(table.name, outcome.problem);
            continue;
        }
        ++done;
        notes << result.rows.notes;
        m_session->finishFills();
    }
    if (all && self && m_session && m_session->workbook()->pivots().tableCount() > 0) {
        const auto gate = armPivotFileGate();
        pivotProblem(tr("Refresh All"), m_session->refreshAllPivots());
    }
    if (grouped && self && m_session) m_session->undoStack()->endMacro();
    if (!problems.isEmpty()) notice(title, problems.join(QLatin1Char('\n')));
    else if (self) setStatus((done > 0 ? tr("The data was read again from its files.") : tr("Nothing needed to be read again.")) + (notes.isEmpty() ? QString() : QLatin1Char(' ') + notes.join(QLatin1Char(' '))));
}

}   // namespace jp
