// The Scenario Manager (Data > What-If Analysis): named sets of values for cells, kept in the sheet as `scenarios`. Each change is one step of Undo.
#include "app/crunchwindow.h"

#include "app/scenariodialogs.h"
#include "core/editsession.h"
#include "core/scenarios.h"
#include "sheet/cellref.h"

#include <QInputDialog>
#include <QLocale>
#include <QUndoStack>

namespace jp {

namespace {

using jp::office::sheet::Scenario;
using jp::office::sheet::ScenarioCell;
using jp::office::sheet::ScenarioList;

QString refsOf(const Scenario &s)
{
    QStringList out;
    for (const ScenarioCell &c : s.cells) out << c.ref;
    return out.join(QLatin1Char(' '));
}

int indexOfName(const ScenarioList &list, const QString &name, int except = -1)
{
    for (int i = 0; i < list.list.size(); ++i)
        if (i != except && list.list[i].name.compare(name, Qt::CaseInsensitive) == 0) return i;
    return -1;
}

} // namespace

void CrunchWindow::scenarioManagerDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session) return;
    ScenarioManagerDialog dialog(this);
    auto refresh = [this, &dialog] { dialog.setScenarios(m_session->scenariosOf(m_sheet)); };
    auto currentText = [this](const QString &ref) {
        quint32 row = 0, col = 0;
        return jp::office::sheet::parseCellRef(ref, &row, &col) ? m_session->editText(m_sheet, row, col) : QString();
    };
    connect(&dialog, &ScenarioManagerDialog::addRequested, this, [&] {
        ScenarioEditDialog edit(true, currentText, &dialog);
        const jp::office::sheet::CellRange sel = selectedRange();
        if (sel.valid && sel.cellCount() <= crunch::kMaxScenarioCells) {
            const QString text = sel.row1 == sel.row2 && sel.col1 == sel.col2 ? jp::office::sheet::cellName(sel.row1, sel.col1) : jp::office::sheet::rangeName(sel);
            QStringList cells;
            crunch::parseScenarioCells(text, &cells);
            QStringList values;
            for (const QString &c : cells) values << currentText(c);
            edit.setCells(text, values);
        }
        if (edit.exec() == QDialog::Accepted) addScenario(edit.name(), edit.cellsText(), edit.comment(), edit.values(), edit.locked());
        refresh();
    });
    connect(&dialog, &ScenarioManagerDialog::editRequested, this, [&](int index) {
        const ScenarioList &list = m_session->scenariosOf(m_sheet);
        if (index < 0 || index >= list.list.size()) return;
        const Scenario s = list.list[index];
        ScenarioEditDialog edit(false, currentText, &dialog);
        edit.setName(s.name);
        edit.setComment(s.comment);
        edit.setLocked(s.locked);
        QStringList values;
        for (const ScenarioCell &c : s.cells) values << c.value;
        edit.setCells(refsOf(s), values);
        if (edit.exec() == QDialog::Accepted) editScenario(index, edit.name(), edit.comment(), edit.values(), edit.locked() ? 1 : 0);
        refresh();
    });
    connect(&dialog, &ScenarioManagerDialog::deleteRequested, this, [&](int index) {
        const ScenarioList &list = m_session->scenariosOf(m_sheet);
        if (index < 0 || index >= list.list.size()) return;
        if (confirm(tr("Delete Scenario"), tr("Delete the scenario \"%1\"? You can undo this until the workbook is saved.").arg(list.list[index].name))) deleteScenario(index);
        refresh();
    });
    connect(&dialog, &ScenarioManagerDialog::showRequested, this, [&](int index) {
        showScenario(index);
        refresh();
    });
    connect(&dialog, &ScenarioManagerDialog::summaryRequested, this, [&] {
        bool ok = false;
        const QString text = QInputDialog::getText(&dialog, tr("Scenario Summary"), tr("The cells whose values you want to compare under each scenario, such as B1 or C3:C5:"), QLineEdit::Normal, QString(), &ok);
        if (ok && scenarioSummary(text)) dialog.accept();
    });
    refresh();
    dialog.exec();
}

bool CrunchWindow::addScenario(const QString &name, const QString &changing, const QString &comment, const QStringList &values, bool locked)
{
    const QString title = tr("Add Scenario");
    if (!m_session || !editingAllowed()) return false;
    const QString clean = name.trimmed();
    if (clean.isEmpty()) {
        notice(title, tr("Give the scenario a name."));
        return false;
    }
    QStringList cells;
    QString problem;
    if (!crunch::parseScenarioCells(changing, &cells, &problem)) {
        notice(title, problem);
        return false;
    }
    ScenarioList list = m_session->scenariosOf(m_sheet);
    if (indexOfName(list, clean) >= 0) {
        notice(title, tr("A scenario named \"%1\" is there already. Give this one another name.").arg(clean));
        return false;
    }
    if (list.list.size() >= ScenarioList::kMaxScenarios) {
        notice(title, tr("A sheet holds at most %1 scenarios.").arg(ScenarioList::kMaxScenarios));
        return false;
    }
    Scenario s;
    s.name = clean;
    s.comment = comment.trimmed();
    s.locked = locked;
    s.user = qEnvironmentVariable("USER", qEnvironmentVariable("USERNAME")).left(255);
    for (int i = 0; i < cells.size(); ++i) {
        ScenarioCell c;
        c.ref = cells[i];
        quint32 row = 0, col = 0;
        jp::office::sheet::parseCellRef(cells[i], &row, &col);
        c.value = i < values.size() ? values[i] : m_session->editText(m_sheet, row, col);
        s.cells.append(c);
    }
    list.list.append(s);
    list.sqref = cells.join(QLatin1Char(' '));
    const crunch::EditSession::Outcome o = m_session->setScenarios(m_sheet, list, title);
    if (!o.ok) {
        notice(title, o.problem);
        return false;
    }
    setStatus(tr("Scenario \"%1\" added.").arg(clean));
    return true;
}

bool CrunchWindow::editScenario(int index, const QString &name, const QString &comment, const QStringList &values, int locked)
{
    const QString title = tr("Edit Scenario");
    if (!m_session || !editingAllowed()) return false;
    ScenarioList list = m_session->scenariosOf(m_sheet);
    if (index < 0 || index >= list.list.size()) return false;
    const QString clean = name.trimmed();
    if (clean.isEmpty()) {
        notice(title, tr("Give the scenario a name."));
        return false;
    }
    if (indexOfName(list, clean, index) >= 0) {
        notice(title, tr("A scenario named \"%1\" is there already. Give this one another name.").arg(clean));
        return false;
    }
    Scenario &s = list.list[index];
    s.name = clean;
    s.comment = comment.trimmed();
    if (locked >= 0) s.locked = locked != 0;
    for (int i = 0; i < s.cells.size() && i < values.size(); ++i) s.cells[i].value = values[i];
    const crunch::EditSession::Outcome o = m_session->setScenarios(m_sheet, list, title);
    if (!o.ok) {
        notice(title, o.problem);
        return false;
    }
    return true;
}

bool CrunchWindow::deleteScenario(int index)
{
    const QString title = tr("Delete Scenario");
    if (!m_session || !editingAllowed()) return false;
    ScenarioList list = m_session->scenariosOf(m_sheet);
    if (index < 0 || index >= list.list.size()) return false;
    list.list.removeAt(index);
    if (list.current >= list.list.size()) list.current = 0;
    if (list.show >= list.list.size()) list.show = 0;
    const crunch::EditSession::Outcome o = m_session->setScenarios(m_sheet, list, title);
    if (!o.ok) {
        notice(title, o.problem);
        return false;
    }
    return true;
}

bool CrunchWindow::showScenario(int index)
{
    const QString title = tr("Show Scenario");
    if (!m_session || !editingAllowed()) return false;
    ScenarioList list = m_session->scenariosOf(m_sheet);
    if (index < 0 || index >= list.list.size()) return false;
    const Scenario s = list.list[index];
    // each cell must take the value before any is put in
    for (const ScenarioCell &c : s.cells) {
        quint32 row = 0, col = 0;
        QString why;
        if (!jp::office::sheet::parseCellRef(c.ref, &row, &col) || !m_session->canEditCell(m_sheet, row, col, &why)) {
            notice(title, why.isEmpty() ? tr("The scenario names a cell that is not on the sheet: %1.").arg(c.ref) : why);
            return false;
        }
    }
    QUndoStack *stack = m_session->undoStack();
    stack->beginMacro(title);
    bool failed = false;
    for (const ScenarioCell &c : s.cells) {
        if (c.deleted) continue;
        quint32 row = 0, col = 0;
        jp::office::sheet::parseCellRef(c.ref, &row, &col);
        const crunch::EditSession::Outcome o = m_session->type(m_sheet, row, col, c.value, false);
        if (!o.ok) {
            failed = true;
            break;
        }
    }
    if (!failed && (list.current != index || list.show != index)) {
        list.current = list.show = index;
        failed = !m_session->setScenarios(m_sheet, list, title).ok;
    }
    endEditStep(stack);
    if (failed) {
        stack->undo();
        notice(title, tr("The scenario could not be shown."));
        return false;
    }
    setStatus(tr("Scenario \"%1\" is shown.").arg(s.name));
    return true;
}

bool CrunchWindow::scenarioSummary(const QString &resultText)
{
    const QString title = tr("Scenario Summary");
    if (!m_session || !editingAllowed()) return false;
    if (m_saving || !m_saveAfterCalculation.isNull() || !m_saveAfterTrial.isNull()) {
        setStatus(tr("Scenario Summary can start when the workbook is saved."));
        return false;
    }
    const ScenarioList list = m_session->scenariosOf(m_sheet);
    if (list.list.isEmpty()) {
        notice(title, tr("This sheet has no scenarios. Add some first."));
        return false;
    }
    QStringList results;
    QString problem;
    if (!crunch::parseScenarioCells(resultText, &results, &problem)) {
        notice(title, problem);
        return false;
    }
    QStringList changing;
    for (const Scenario &s : list.list)
        for (const ScenarioCell &c : s.cells)
            if (!c.deleted && !changing.contains(c.ref)) changing << c.ref;
    auto place = [](const QString &ref, quint32 *row, quint32 *col) { return jp::office::sheet::parseCellRef(ref, row, col); };
    auto asText = [](QString text) {
        // (text that begins like a formula is kept as text)
        return text.startsWith(QLatin1Char('=')) || text.startsWith(QLatin1Char('+')) || text.startsWith(QLatin1Char('-')) || text.startsWith(QLatin1Char('@')) ? QLatin1Char('\'') + text : text;
    };
    // what the cells hold now, and what the result cells come to under each scenario (each is put in, worked out, and read; then all of it is undone)
    QStringList nowValues, nowResults;
    for (const QString &ref : changing) {
        quint32 row = 0, col = 0;
        place(ref, &row, &col);
        nowValues << m_session->editText(m_sheet, row, col);
    }
    auto resultsNow = [&] {
        QStringList out;
        for (const QString &ref : results) {
            quint32 row = 0, col = 0;
            place(ref, &row, &col);
            out << m_session->displayText(m_sheet, row, col);
        }
        return out;
    };
    nowResults = resultsNow();
    QVector<QStringList> underEach;
    QUndoStack *stack = m_session->undoStack();
    struct Trial {
        CrunchWindow *w;
        explicit Trial(CrunchWindow *window) : w(window) { ++w->m_trialMacros; }
        ~Trial()
        {
            if (--w->m_trialMacros == 0) w->resumeSaveAfterTrial();
        }
    } trialOpen(this);
    stack->beginMacro(title);
    bool failed = false;
    for (const Scenario &s : list.list) {
        for (const ScenarioCell &c : s.cells) {
            quint32 row = 0, col = 0;
            if (c.deleted || !place(c.ref, &row, &col)) continue;
            const qint64 before = m_loader.changesMade();
            if (!m_session->type(m_sheet, row, col, c.value, false).ok) failed = true;
            if (m_loader.changesMade() != before) {
                m_calculating = true;
                if (m_manualNow) m_loader.recalculate();
            }
        }
        if (failed || !settleCalculation()) {
            failed = true;
            break;
        }
        underEach.append(resultsNow());
    }
    endEditStep(stack);
    stack->undo();
    if (failed) {
        notice(title, tr("The scenarios could not all be worked out."));
        return false;
    }
    // the table: a blank row and column around it
    QVector<QStringList> table;
    auto row = [&](const QStringList &cells) { table.append(QStringList() << QString() << cells); };
    row({tr("Scenario Summary")});
    QStringList head = {QString(), tr("Current Values:")};
    for (const Scenario &s : list.list) head << asText(s.name);
    row(head);
    row({tr("Changing Cells:")});
    for (int i = 0; i < changing.size(); ++i) {
        QStringList line = {changing[i], asText(nowValues[i])};
        for (const Scenario &s : list.list) {
            QString value;
            for (const ScenarioCell &c : s.cells)
                if (!c.deleted && c.ref == changing[i]) value = asText(c.value);
            line << value;
        }
        row(line);
    }
    row({tr("Result Cells:")});
    for (int i = 0; i < results.size(); ++i) {
        QStringList line = {results[i], asText(nowResults[i])};
        for (int s = 0; s < underEach.size(); ++s) line << asText(underEach[s][i]);
        row(line);
    }
    row({});
    row({tr("The Current Values column shows the cells as they were when this summary was made.")});
    table.prepend(QStringList());
    // a sheet of its own for it
    QString name = tr("Scenario Summary");
    const auto &sheets = m_loaded->book->info().sheets;
    auto taken = [&](const QString &n) {
        for (const auto &s : sheets)
            if (!s.deleted && s.name.compare(n, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    for (int n = 2; taken(name); ++n) name = tr("Scenario Summary %1").arg(n);
    stack->beginMacro(title);
    int made = -1;
    crunch::EditSession::Outcome o = m_session->insertSheet(-1, name, &made);
    if (o.ok) {
        jp::office::sheet::CellRange target;
        target.valid = true;
        target.row1 = target.row2 = 0;
        target.col1 = target.col2 = 0;
        o = m_session->pasteText(made, target, table, crunch::EditSession::PasteOptions());
    }
    endEditStep(stack);
    if (!o.ok) {
        stack->undo();
        notice(title, o.problem);
        return false;
    }
    setActiveSheet(made);
    return true;
}

} // namespace jp
