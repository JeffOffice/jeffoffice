// Insert Function, and AutoSum's More Functions: the dialog picks a function and the formula is begun with it, in the cell being typed in (at the cursor) or in the cell with the cursor.
#include "app/crunchwindow.h"

#include "app/insertfunctiondialog.h"
#include "core/functioninfo.h"

#include <QAction>

namespace jp {

void CrunchWindow::createFunctionActions()
{
    mk("formulas.insertFunction", tr("Insert Function"), "square-function", QKeySequence(Qt::SHIFT | Qt::Key_F3), [this] { insertFunctionCommand(); });
    mk("edit.autosum.more", tr("More Functions…"), "", QKeySequence(), [this] { insertFunctionCommand(); });
    for (const char *id : {"formulas.insertFunction", "edit.autosum.more"}) m_opsActions << QString::fromLatin1(id);
    updateEditActions();
}

void CrunchWindow::insertFunctionCommand()
{
    if (!m_session || !m_loaded) return;
    QString name;
    if (m_functionAsker) {
        if (!m_functionAsker(&name)) return;
    } else {
        InsertFunctionDialog dialog(this);
        if (dialog.exec() != QDialog::Accepted) return;
        name = dialog.chosen();
    }
    insertFunction(name);
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
