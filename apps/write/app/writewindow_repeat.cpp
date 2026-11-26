// Edit > Repeat (F4, and Ctrl+Y while there is nothing to redo): the last command done again on the selection of now, and the lists of steps on Undo and Redo.

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/plaintext.h"
#include "app/undohistory.h"

#include <QAbstractSpinBox>
#include <QAction>
#include <QApplication>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSet>
#include <QTextEdit>

namespace jp {

bool WriteWindow::repeatableCommand(const QString &id)
{
    // Named one by one, so that a command added later is not repeated until it is put here: each does its work with no box to answer and nothing but the selection to work on.
    static const QSet<QString> exact = [] {
        QSet<QString> ids;
        for (const char *name : {"fmt.bold", "fmt.italic", "fmt.underline", "fmt.strike", "fmt.subscript", "fmt.superscript", "fmt.charBorder", "fmt.grow", "fmt.shrink", "fmt.clear",
                                 "fmt.alignLeft", "fmt.alignCenter", "fmt.alignRight", "fmt.justify", "fmt.case.cycle",
                                 "para.bullets", "para.numbers", "para.indentInc", "para.indentDec", "para.horizontalLine",
                                 "style.normal", "list.bulletNone", "list.numberNone",
                                 "table.insertAbove", "table.insertBelow", "table.insertLeft", "table.insertRight", "table.deleteCells", "table.deleteRows", "table.deleteColumns",
                                 "table.deleteTable", "table.moveUp", "table.moveDown", "table.merge", "table.autofitContents", "table.autofitWindow", "table.fixedWidth", "table.distributeRows",
                                 "table.distributeColumns", "table.splitTable", "table.style.clear",
                                 "insert.pageBreak", "insert.blankPage"})
            ids.insert(QLatin1String(name));
        return ids;
    }();
    if (exact.contains(id)) return true;
    for (const char *prefix : {"style.heading", "list.level.", "para.border.", "table.align.", "table.border.", "table.direction.", "insert.special.", "insert.ready."})
        if (id.startsWith(QLatin1String(prefix))) return true;
    return false;
}

// A command that can't be run now (the document is read-only or protected, or a search holds the text) is refused with the reason, as the command was when it was given.
bool WriteWindow::refuseCommand(const QString &id)
{
    if (isReadOnly() && !commandAllowedReadOnly(id) && !protectionAllows(id)) {
        refuseReadOnlyChange();
        return true;
    }
    if (searchBlocked() && !commandAllowedReadOnly(id)) {
        refuseSearchBlockedChange();
        return true;
    }
    return false;
}

void WriteWindow::recordRepeat(const QString &name, std::function<void()> again, quint64 document)
{
    if (!m_repeat || !m_doc) return;
    m_view->endTypedRun();
    m_repeatIsTyping = false;
    m_repeat->record(name, std::move(again), document);
}

bool WriteWindow::changeAndRepeat(const QString &name, const std::function<void()> &change)
{
    if (!m_doc || refuseIfReadOnly()) return false;
    const quint64 document = m_repeat->document();
    m_doc->setStepName(name);
    change();
    if (m_doc) m_doc->setStepName(QString());
    recordRepeat(name, [this, name, change] {
        if (!m_doc || refuseIfReadOnly()) return;
        const write::WriteDoc::StepGroup group(*m_doc);
        m_doc->setStepName(name);
        change();
        m_doc->setStepName(QString());
    }, document);
    return true;
}

void WriteWindow::createRepeatActions()
{
    m_repeat = new jp::RepeatCommand(this);
    connect(m_repeat, &jp::RepeatCommand::changed, this, [this] { updateRepeatActions(); });
    mk("edit.repeat", tr("Repeat"), "repeat", QKeySequence(Qt::Key_F4), [this] { runRepeat(); });
    // (Ctrl+Y: a key of its own for Repeat, with no place on the ribbon: while Redo has a step the key is Redo.)
    m_repeatCtrlY = new QAction(tr("Repeat"), this);
    m_repeatCtrlY->setObjectName(QStringLiteral("edit.repeat.ctrlY"));
    m_repeatCtrlY->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    m_repeatCtrlY->setShortcutContext(Qt::WindowShortcut);
    m_repeatCtrlY->setEnabled(false);
    addAction(m_repeatCtrlY);
    connect(m_repeatCtrlY, &QAction::triggered, this, [this] { runRepeat(); });
    // A run of typing is the last command from the moment it begins; what it types again is read when Repeat is done (the run goes on while it is typed).
    connect(m_view, &write::PageView::typedRunChanged, this, [this] {
        if (m_view->typedRun().isEmpty()) {
            if (m_repeatIsTyping) m_repeat->clear();
            m_repeatIsTyping = false;
            return;
        }
        const quint64 document = m_repeat->document();
        m_repeat->record(tr("Typing"), [this] {
            if (refuseCommand(QStringLiteral("edit.repeat"))) return;
            m_view->repeatTyping(m_view->typedRun());
            m_view->setFocus(Qt::OtherFocusReason);
        }, document);
        m_repeatIsTyping = true;
    });
}

void WriteWindow::runRepeat()
{
    if (!m_doc) return;
    // (F4 is the text's: a box of text elsewhere in the window, the find box or a field of the ribbon, keeps its key.)
    if (QWidget *w = QApplication::focusWidget(); w && w != m_view && !m_view->isAncestorOf(w) && (qobject_cast<QLineEdit *>(w) || qobject_cast<QTextEdit *>(w) || qobject_cast<QPlainTextEdit *>(w) || qobject_cast<QAbstractSpinBox *>(w))) return;
    m_repeat->run();
}

void WriteWindow::updateRepeatActions()
{
    QAction *repeat = act("edit.repeat");
    if (!m_repeat || !repeat) return;
    const bool open = m_doc && mayEdit();
    repeat->setEnabled(open && m_repeat->available());
    m_repeatCtrlY->setEnabled(open && m_repeat->ctrlYEnabled());
    repeat->setToolTip(jp::plainToolTip(m_repeat->tip()));
    repeat->setStatusTip(jp::plainToolTip(m_repeat->available() ? tr("Repeat %1").arg(m_repeat->name()) : tr("Repeat")));
}

// The steps the history list on Undo or Redo shows, newest (nearest) first, and doing the first `count` of them.
QMenu *WriteWindow::buildHistoryMenu(QWidget *owner, bool redo)
{
    return new jp::UndoHistoryMenu(redo, [this, redo] {
        if (!m_doc || m_view->shapeEditor()) return QStringList();
        return redo ? m_doc->redoNames(jp::UndoHistoryMenu::kMaxShown) : m_doc->undoNames(jp::UndoHistoryMenu::kMaxShown);
    }, [this, redo](int count) {
        if (!m_doc || refuseCommand(redo ? QStringLiteral("edit.redo") : QStringLiteral("edit.undo"))) return;
        for (int i = 0; i < count && (redo ? m_doc->canRedo() : m_doc->canUndo()); ++i) {
            if (redo) m_view->redo();
            else m_view->undo();
        }
    }, owner);
}

} // namespace jp
