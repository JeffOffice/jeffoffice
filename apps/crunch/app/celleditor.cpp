#include "app/celleditor.h"

#include "app/icons.h"

#include <QKeyEvent>

namespace jp {

CellEditor::CellEditor(QWidget *parent) : QLineEdit(parent)
{
    setObjectName(QStringLiteral("cellEditor"));
    setFrame(false);
    setMaxLength(1 << 20);   // (a line holds 32,767 characters by default and cuts what is typed or pasted past that; a cell's limit is told to the person when the entry is finished)
    setContextMenuPolicy(Qt::DefaultContextMenu);
    // over the cell: the program's accent round it, and the window's own base inside (a cell is white in a light window and dark in a dark one)
    setStyleSheet(QStringLiteral("QLineEdit#cellEditor { border: 2px solid %1; background: palette(base); color: palette(text); padding: 0px 2px; }").arg(uiAccent().name()));
}

void CellEditor::keyPressEvent(QKeyEvent *e)
{
    const bool shift = e->modifiers() & Qt::ShiftModifier;
    const bool alt = e->modifiers() & Qt::AltModifier;
    const bool ctrlOrMeta = e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier);
    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (alt) {
            insert(QStringLiteral("\n"));
        } else {
            Q_EMIT finished(shift ? Way::ShiftEnter : Way::Enter);
        }
        e->accept();
        return;
    case Qt::Key_Tab:
        Q_EMIT finished(Way::Tab);
        e->accept();
        return;
    case Qt::Key_Backtab:
        Q_EMIT finished(Way::ShiftTab);
        e->accept();
        return;
    case Qt::Key_Escape:
        Q_EMIT cancelled();
        e->accept();
        return;
    case Qt::Key_F2:
        m_mode = m_mode == Mode::Typing ? Mode::Editing : Mode::Typing;
        e->accept();
        return;
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
        // typing over a cell: an arrow finishes the entry and moves the selection on
        if (m_mode == Mode::Typing && !shift && !ctrlOrMeta && !alt) {
            Q_EMIT finished(e->key() == Qt::Key_Up ? Way::Up : e->key() == Qt::Key_Down ? Way::Down : e->key() == Qt::Key_Left ? Way::Left : Way::Right);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
            e->accept();   // (one line: nothing above or below to go to)
            return;
        }
        break;
    default: break;
    }
    QLineEdit::keyPressEvent(e);
}

} // namespace jp
