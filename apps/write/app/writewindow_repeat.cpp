// Edit > Repeat (F4, and Ctrl+Y while there is nothing to redo): the last command done again on the selection of now, and the lists of steps on Undo and Redo.

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/plaintext.h"
#include "app/undohistory.h"

#include <QAbstractSpinBox>
#include <QAction>
#include <QApplication>
#include <QLineEdit>
#include <QPointer>
#include <QPlainTextEdit>
#include <QHash>
#include <QSet>
#include <optional>
#include "app/shapetextedit.h"
#include "core/editops.h"
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

namespace {

// The commands that turn a look on or off: Repeat does not toggle again, it sets the state the command made.
struct ToggleKind {
    bool list = false, numbered = false;
    write::CharToggle look = write::CharToggle::Bold;
};

std::optional<ToggleKind> toggleKindOf(const QString &id)
{
    static const QHash<QString, ToggleKind> kinds = {
        {QStringLiteral("fmt.bold"), {false, false, write::CharToggle::Bold}},
        {QStringLiteral("fmt.italic"), {false, false, write::CharToggle::Italic}},
        {QStringLiteral("fmt.underline"), {false, false, write::CharToggle::Underline}},
        {QStringLiteral("fmt.strike"), {false, false, write::CharToggle::Strike}},
        {QStringLiteral("fmt.subscript"), {false, false, write::CharToggle::Subscript}},
        {QStringLiteral("fmt.superscript"), {false, false, write::CharToggle::Superscript}},
        {QStringLiteral("fmt.charBorder"), {false, false, write::CharToggle::Border}},
        {QStringLiteral("para.bullets"), {true, false, write::CharToggle::Bold}},
        {QStringLiteral("para.numbers"), {true, true, write::CharToggle::Bold}},
    };
    const auto it = kinds.constFind(id);
    if (it == kinds.constEnd()) return std::nullopt;
    return *it;
}

} // namespace

// What a toggle command made of the selection now: on when all of it has the look (or is in the list), else off.
std::optional<bool> WriteWindow::toggleResult(const QString &id) const
{
    const std::optional<ToggleKind> kind = toggleKindOf(id);
    if (!kind || !m_doc) return std::nullopt;
    if (ShapeTextEdit *box = m_view->shapeEditor()) {   // (the box that is being edited takes the commands that it has)
        if (id == QLatin1String("fmt.bold")) return box->charFormat().font().bold();
        if (id == QLatin1String("fmt.italic")) return box->charFormat().font().italic();
        if (id == QLatin1String("fmt.underline")) return box->charFormat().font().underline();
        if (id == QLatin1String("para.bullets")) return box->hasBullets();
        return std::nullopt;
    }
    const QTextCursor &c = m_view->cursor();
    if (!kind->list) return write::selectionHasLook(c, kind->look);
    const write::ListKind want = kind->numbered ? write::ListKind::Numbers : write::ListKind::Bullets;
    const int last = c.hasSelection() ? c.selectionEnd() - 1 : c.position();
    bool all = true;
    for (QTextBlock b = c.document()->findBlock(c.selectionStart()); b.isValid() && b.position() <= std::max(last, c.selectionStart()); b = b.next()) all = all && write::listKindOf(*m_doc, b) == want;
    return all;
}

// Sets the state a toggle command made, on the selection of now: a selection that has it already is left as it is, and one that has part of it gets all of it.
void WriteWindow::applyToggle(const QString &id, bool on)
{
    const std::optional<ToggleKind> kind = toggleKindOf(id);
    if (!kind || !m_doc) return;
    if (ShapeTextEdit *box = m_view->shapeEditor()) {
        if (id == QLatin1String("fmt.bold")) box->setBold(on);
        else if (id == QLatin1String("fmt.italic")) box->setItalic(on);
        else if (id == QLatin1String("fmt.underline")) box->setUnderline(on);
        else if (id == QLatin1String("para.bullets")) box->setBullets(on);
        return;
    }
    m_doc->takeFormattingRefused();
    if (kind->list) write::setList(*m_doc, m_view->cursor(), kind->numbered, on);
    else write::setCharToggle(*m_doc, m_view->cursor(), kind->look, on);
    if (m_doc->takeFormattingRefused()) statusBar()->showMessage(tr("Formatting is limited in this document. Only styles that are not locked can be used."), 6000);
    m_view->setFocus(Qt::OtherFocusReason);
    m_view->viewport()->update();
}

// The commands that do their work in the box of text being edited, when there is one; Repeat does no other command there (it would change the text behind the box, where nobody is typing).
bool WriteWindow::boxTakesCommand(const QString &id)
{
    static const QSet<QString> ids = {QStringLiteral("fmt.bold"), QStringLiteral("fmt.italic"), QStringLiteral("fmt.underline"), QStringLiteral("fmt.grow"), QStringLiteral("fmt.shrink"),
                                      QStringLiteral("fmt.alignLeft"), QStringLiteral("fmt.alignCenter"), QStringLiteral("fmt.alignRight"), QStringLiteral("fmt.justify"),
                                      QStringLiteral("para.bullets"), QStringLiteral("para.indentInc"), QStringLiteral("para.indentDec")};
    return ids.contains(id);
}

bool WriteWindow::refuseInBox(bool boxTakesIt)
{
    if (!m_view->shapeEditor() || boxTakesIt) return false;
    statusBar()->showMessage(tr("This cannot be repeated while the text of a box or shape is being edited."), 6000);
    return true;
}

// A command that can't be run now (the document is read-only or protected, or a search holds the text) is refused with the reason, as the command was when it was given. Each reason is its own
// test, and only the first is the one forms protection may lift: *openHere is set when the document is read only only because it is protected for forms and the command is one formsAllows lets
// act where the caret and the selection are (text the protection leaves out); the command then runs through runWhereOpen, and the view still judges each place it changes.
bool WriteWindow::refuseCommand(const QString &id, bool *openHere)
{
    if (openHere) *openHere = false;
    // Held by read-only or by a protection kind that does not allow the command (formsAllows is false for a read-only file, a search that holds the text, and every other kind).
    const bool held = isReadOnly() && !commandAllowedReadOnly(id) && !protectionAllows(id);
    // Held only by forms protection, and the caret and selection are where the protection leaves the text open: not held after all.
    const bool openByForms = held && formsAllows(id);
    // Repeat itself is held only as the command it repeats is (it checks that one again, where it acts now), so under forms protection it is let through, and never as an open place.
    const bool repeatGoesOn = held && id == QLatin1String("edit.repeat") && formsHeld();
    if (held && !openByForms && !repeatGoesOn) {
        refuseReadOnlyChange();
        return true;
    }
    // A search that holds the text refuses every command but those allowed while read only, in an open section too.
    if (searchBlocked() && !commandAllowedReadOnly(id)) {
        refuseSearchBlockedChange();
        return true;
    }
    if (openHere) *openHere = openByForms;
    return false;
}

// Runs a command in text a forms-protected document leaves open: the window is not read only for the length of the command.
void WriteWindow::runWhereOpen(bool openHere, const std::function<void()> &fn)
{
    if (!openHere) {
        fn();
        return;
    }
    const QPointer<WriteWindow> self(this);   // (a command may close the window)
    m_readOnly = false;
    fn();
    if (self && m_readOnlyByProtection) m_readOnly = true;
}

QAction *WriteWindow::addRepeatableCommandForTest(const QString &id, const std::function<void()> &fn)
{
    m_testRepeatable.insert(id);
    return mk(id, id, QString(), QKeySequence(), fn);
}

void WriteWindow::recordRepeat(const QString &name, std::function<void()> again, quint64 document)
{
    if (!m_repeat || !m_doc) return;
    m_view->endTypedRun();
    m_repeatIsTyping = false;
    m_repeat->record(name, std::move(again), document);
}

// A repeat is always a step of its own, and exactly one, wherever the caret is (typing right after typing would otherwise join the step before it).
void WriteWindow::runRepeatStep(const QString &name, const std::function<void()> &fn)
{
    if (!m_doc) return;
    const QPointer<WriteWindow> self(this);   // (a command may close the window)
    m_doc->startNewStep();
    {
        const write::WriteDoc::StepGroup group(*m_doc);
        const write::WriteDoc::StepName stepName(*m_doc, name);
        fn();
    }
    if (self && m_doc) m_doc->startNewStep();
}

bool WriteWindow::changeAndRepeat(const QString &name, const std::function<void()> &change, bool boxTakesIt)
{
    if (!m_doc || refuseIfReadOnly()) return false;
    const quint64 document = m_repeat->document();
    const QPointer<WriteWindow> self(this);
    {
        const write::WriteDoc::StepName stepName(*m_doc, name);
        change();
    }
    if (!self) return true;
    recordRepeat(name, [this, name, change, boxTakesIt] {
        if (!m_doc || refuseIfReadOnly() || refuseInBox(boxTakesIt)) return;
        runRepeatStep(name, change);
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
            if (refuseCommand(QStringLiteral("edit.repeat"))) return;   // (typing goes where the person is editing: the box, the header or footer, the note, or the text)
            const QPointer<WriteWindow> self(this);
            runRepeatStep(tr("Typing"), [this] { m_view->repeatTyping(m_view->typedRun()); });
            if (self) m_view->setFocus(Qt::OtherFocusReason);
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

// Undo, Redo, and the keys that follow them are as the history is the moment it changes (Ctrl+Y is Redo while there is a step to redo, so it must never be taken from a state that is a moment old).
void WriteWindow::syncHistoryActions()
{
    if (!m_doc || !m_repeat) return;
    if (QAction *undo = act("edit.undo")) undo->setEnabled(m_doc->canUndo());
    if (QAction *redo = act("edit.redo")) redo->setEnabled(m_doc->canRedo());
    m_repeat->setRedoAvailable(m_doc->canRedo());
    updateRepeatActions();
}

void WriteWindow::updateRepeatActions()
{
    QAction *repeat = act("edit.repeat");
    if (!m_repeat || !repeat) return;
    const bool open = m_doc && (mayEdit() || (formsHeld() && !searchBlocked()));   // (under forms protection Repeat is judged where it acts)
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
