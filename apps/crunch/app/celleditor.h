#pragma once
// The line a person types a cell's contents into, over the cell. It knows the keys of an entry: Enter and Tab (and with Shift) finish it, and say which way the selection goes on;
// Escape gives it up; F2 changes between typing over a cell (where an arrow finishes the entry and moves on) and editing what is in it (where an arrow moves in the text); Alt+Enter
// starts a new line in the text. The window does the rest.

#include <QLineEdit>

namespace jp {

class CellEditor : public QLineEdit {
    Q_OBJECT
public:
    explicit CellEditor(QWidget *parent = nullptr);

    // How the entry was begun: typing over the cell (the first key is the cell's first character; arrows finish it), or editing what the cell holds.
    enum class Mode { Typing, Editing };
    Mode mode() const { return m_mode; }
    void setMode(Mode mode) { m_mode = mode; }

    // How an entry was finished: the key, and so the way the selection goes.
    enum class Way { Stay, Enter, ShiftEnter, Tab, ShiftTab, Up, Down, Left, Right };

Q_SIGNALS:
    void finished(jp::CellEditor::Way way);
    void cancelled();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    bool focusNextPrevChild(bool) override { return false; }

private:
    Mode m_mode = Mode::Typing;
};

} // namespace jp
