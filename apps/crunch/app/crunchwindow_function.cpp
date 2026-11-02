// Pick a Function, and AutoSum's All Functions: the dialog picks a function, and the Function Arguments dialog then fills in its arguments with the result shown as they are typed; OK puts the call in
// the cell being typed in (at the cursor) or in the cell with the cursor. With the cursor inside a function's brackets in a formula being typed (Shift+F3, or the fx button), that call is what is changed.
#include "app/crunchwindow.h"
#include "core/formularefs.h"

#include "app/functionargumentsdialog.h"
#include "app/help.h"
#include "app/insertfunctiondialog.h"
#include "core/functioncall.h"
#include "core/functioninfo.h"
#include "grid/gridview.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QEventLoop>

namespace jp {

void CrunchWindow::createFunctionActions()
{
    mk("formulas.insertFunction", tr("Pick a Function…"), "square-function", QKeySequence(Qt::SHIFT | Qt::Key_F3), [this] { insertFunctionCommand(); });
    mk("edit.autosum.more", tr("All Functions…"), "", QKeySequence(), [this] { insertFunctionCommand(); });
    for (const char *id : {"formulas.insertFunction", "edit.autosum.more"}) m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

void CrunchWindow::insertFunctionCommand()
{
    if (!m_session || !m_loaded || m_argsDialog) return;
    const QChar separator = crunch::argumentSeparator(m_session->locale());
    QLineEdit *line = nullptr;
    if (m_edit.active) {
        line = m_formulaBar->hasFocus() ? m_formulaBar : m_cellEditor;
        const crunch::FunctionCall call = crunch::functionCallAt(line->text(), line->cursorPosition(), separator);
        if (call.found) {
            // the cursor is in the brackets of a function: that call is what the dialog changes
            functionArguments(*crunch::docOf(call.name), call.arguments, call.start, call.end, call.name, call.raw);
            return;
        }
    }
    QString name;
    if (m_functionAsker) {
        if (!m_functionAsker(&name)) return;
        if (!m_argumentsAsker) {
            insertFunction(name);
            return;
        }
    } else {
        InsertFunctionDialog dialog(this);
        if (dialog.exec() != QDialog::Accepted) return;
        name = dialog.chosen();
    }
    const crunch::FunctionDoc *doc = crunch::docOf(name);
    if (!doc) return;
    if (m_edit.active) {
        const QString text = line->text();
        if (!text.isEmpty() && !text.startsWith(QLatin1Char('='))) return;   // (text is being typed: not a formula, so a function does not go in it)
        const int at = text.isEmpty() ? 1 : std::max(1, line->cursorPosition());
        functionArguments(*doc, QStringList(), at, at);
    } else {
        if (!editingAllowed()) return;
        // (a cell that can't be changed is told of before the dialog, not after the arguments were filled in)
        QString why;
        const grid::Selection &sel = m_view->selection();
        if (!m_session->canEditCell(m_sheet, quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)), &why)) {
            notice(tr("Edit"), why);
            return;
        }
        functionArguments(*doc, QStringList(), 0, 0);
    }
}

// The Function Arguments dialog for a call, in the formula being typed (`start` to `end` of its text is what the call replaces, or where a new one goes) or, with no entry going, for a new formula
// of the cell with the cursor. The entry is put away while the dialog is open (the sheet is pointed at, and a click must not finish it) and is made again after: with the call in place for OK, as it
// was for Cancel.
void CrunchWindow::functionArguments(const crunch::FunctionDoc &doc, const QStringList &arguments, int start, int end, const QString &writtenName, const QStringList &raw)
{
    if (m_argsDialog) return;   // (one at a time)
    const bool editing = m_edit.active;
    const QChar separator = crunch::argumentSeparator(m_session->locale());
    QString text;
    int caret = 0;
    bool fromBar = false;
    CellEditor::Mode mode = CellEditor::Mode::Editing;
    int sheet = m_sheet;
    // where the formula is of, and what is selected (the whole of it comes back, not only the cell with the cursor)
    const grid::Selection selection = m_view->selection();
    quint32 row = quint32(std::max<qint64>(0, selection.activeRow)), col = quint32(std::max<qint64>(0, selection.activeCol));
    if (editing) {
        QLineEdit *line = m_formulaBar->hasFocus() ? m_formulaBar : m_cellEditor;
        text = line->text();
        caret = line->cursorPosition();
        fromBar = m_formulaBar->hasFocus();
        mode = m_cellEditor->mode();
        sheet = m_edit.sheet;
        row = m_edit.row;
        col = m_edit.col;
        cancelEdit();
    }
    const QString was = text;
    if (text.isEmpty()) text = QStringLiteral("=");
    FunctionArgumentsDialog dialog(this, doc, arguments, separator);
    if (!writtenName.isEmpty()) dialog.setCallName(writtenName);
    dialog.setRawArguments(raw);
    crunch::EditSession *session = m_session.get();
    // (the session is looked for each time: a workbook that is closed while the dialog is open leaves nothing to read)
    dialog.setEvaluator([this, session, sheet, row, col](const QString &typed) {
        if (!m_session || m_session.get() != session) return crunch::EditSession::Preview();
        return session->previewFormula(typed, sheet, row, col);
    });
    dialog.setHelpTopic(functionHelpTopic(QString::fromLatin1(doc.name)));
    m_argsDialog = &dialog;
    connect(&dialog, &FunctionArgumentsDialog::helpRequested, this, [this, &dialog](const QString &topic) {
        // (Help is a window of the dialog's own, so the dialog that blocks this window does not block it)
        auto *w = dialog.findChild<HelpWindow *>(QString(), Qt::FindDirectChildrenOnly);
        if (!w) w = new HelpWindow(this, &dialog);
        w->view()->showTopic(topic);
        w->show();
        w->raise();
    });
    // cells pointed at while a box is collapsed are written in it, with the sheet's name when it is another sheet
    QMetaObject::Connection pointing;
    connect(&dialog, &FunctionArgumentsDialog::pickingStarted, this, [this, &dialog, &pointing, sheet](int boxRow) {
        pointing = connect(m_view, &grid::GridView::selectionChanged, &dialog, [this, &dialog, boxRow, sheet] {
            if (!dialog.isPicking() || !m_loaded) return;
            // (the same addresses as pointing writes: A:A for a whole column, 3:3 for a whole row)
            const grid::Selection &sel = m_view->selection();
            using Kind = crunch::FormulaRef::Kind;
            const Kind kind = sel.whole == grid::Selection::Whole::Columns ? Kind::Columns : sel.whole == grid::Selection::Whole::Rows ? Kind::Rows : Kind::Range;
            QString where = crunch::referenceText(kind, quint32(sel.range.row1), quint32(sel.range.col1), quint32(sel.range.row2), quint32(sel.range.col2));
            if (m_sheet >= 0 && m_sheet < int(m_loaded->book->info().sheets.size()) && m_sheet != sheet) where = jp::office::sheet::sheetNamePrefix(m_loaded->book->info().sheets[size_t(m_sheet)].name) + where;
            dialog.setArgumentText(boxRow, where);
        });
    });
    connect(&dialog, &FunctionArgumentsDialog::pickingEnded, this, [&pointing] { QObject::disconnect(pointing); });
    bool accepted;
    if (m_argumentsAsker) {
        accepted = m_argumentsAsker(&dialog);
    } else {
        QEventLoop loop;
        connect(&dialog, &QDialog::finished, &loop, &QEventLoop::quit);
        dialog.show();
        loop.exec();
        accepted = dialog.result() == QDialog::Accepted;
    }
    QObject::disconnect(pointing);
    m_argsDialog = nullptr;
    if (m_session.get() != session) return;   // (the workbook was closed or replaced: there is nothing to put the call in)
    // the sheet, selection, and entry as they were
    if (m_sheet != sheet) setActiveSheet(sheet);
    if (!(m_view->selection() == selection)) m_view->setSelection(selection);
    // the entry again (a call that is not closed has its end where the text ends)
    auto resume = [&](const QString &entry, int cursor) {
        beginEdit(entry, mode, false);
        if (!m_edit.active) return;
        m_cellEditor->setCursorPosition(cursor);
        m_formulaBar->setCursorPosition(cursor);
        if (fromBar) m_formulaBar->setFocus(Qt::OtherFocusReason);
    };
    if (!accepted) {
        if (editing) resume(was, caret);
        return;
    }
    const QString call = dialog.callText();
    if (editing) {
        const int e = std::min<int>(end, int(text.size()));
        resume(text.left(start) + call + text.mid(e), start + int(call.size()));
        return;
    }
    beginEdit(QLatin1Char('=') + call, CellEditor::Mode::Editing, false);
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
}

// The Help topic of a function: its own page when Help has one, else the page on the Function Arguments dialog.
QString CrunchWindow::functionHelpTopic(const QString &name) const
{
    const QString own = QStringLiteral("fn-") + name.toLower();
    return help::topicIds().contains(own) ? own : QStringLiteral("function-arguments");
}

// The function is put in the formula: at the cursor of the entry being made (a formula has "=" in front of it already, or gets it), else in the cell with the cursor as a formula to go on with.
bool CrunchWindow::insertFunction(const QString &name)
{
    const crunch::FunctionDoc *doc = crunch::docOf(name);
    if (!doc) return false;
    const QString call = QString::fromLatin1(doc->name) + QStringLiteral("()");
    if (m_edit.active) {
        QLineEdit *line = m_formulaBar->hasFocus() ? m_formulaBar : m_cellEditor;
        const QString text = line->text();
        if (text.isEmpty()) {
            // (nothing yet: the entry becomes a formula that begins with the function)
            line->setText(QLatin1Char('=') + call);
            line->setCursorPosition(int(call.size()));   // (inside the brackets)
        } else if (text.startsWith(QLatin1Char('='))) {
            line->insert(call);
            line->setCursorPosition(line->cursorPosition() - 1);
        } else {
            return false;   // (text is being typed: not a formula, so a function does not go in it)
        }
        line->setFocus(Qt::OtherFocusReason);
        return true;
    }
    if (!editingAllowed()) return false;
    beginEdit(QLatin1Char('=') + call, CellEditor::Mode::Editing, false);
    if (!m_edit.active) return false;
    const int at = int(call.size());   // (after "=" and the name and "(": before ")")
    m_cellEditor->setCursorPosition(at);
    m_formulaBar->setCursorPosition(at);
    return true;
}

} // namespace jp
