#pragma once
// The line a person types a cell's contents into, over the cell. It knows the keys of an entry: Enter and Tab (and with Shift) finish it, and say which way the selection goes on;
// Escape gives it up; F2 changes between typing over a cell (where an arrow finishes the entry and moves on) and editing what is in it (where an arrow moves in the text); Alt+Enter
// starts a new line in the text. The window does the rest. While an entry is typed, words that are not in the dictionary are underlined in red, the right-click menu offers the
// dictionary's words, and AutoCorrect fixes what is typed (a typing slip, capitals, smart quotes, dashes, fractions); none of it inside a formula.

#include <QLineEdit>
#include <QPair>
#include <QTimer>
#include <QVector>

class QMenu;

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
    enum class Way { Stay, Enter, ShiftEnter, Tab, ShiftTab, Up, Down, Left, Right, Fill };

    // The misspelled words in the entry, as [start, end) places in its text, as they are underlined (an entry that is a formula has none; the word still being typed is not one).
    QVector<QPair<int, int>> misspellings() const { return m_wrong; }
    void checkSpelling();
    // Puts the dictionary's words for the misspelled word at `position`, Ignore All, and Add to Dictionary in the menu, and a separator after them. False, with nothing put in, if the
    // word there is spelled right (or the entry is a formula).
    bool addSpellingSuggestions(QMenu &menu, int position);
    // Whether the entry is a formula (it starts with = , or with + or - and then something that is not a number): AutoCorrect leaves it alone.
    static bool isFormulaEntry(const QString &text);

Q_SIGNALS:
    void finished(jp::CellEditor::Way way);
    void cancelled();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    bool focusNextPrevChild(bool) override { return false; }
    void paintEvent(QPaintEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    // A character typed, with the fixes AutoCorrect makes; false if it is typed as it is.
    bool typeWithAutoCorrect(QKeyEvent *e);
    void replaceRange(int from, int to, const QString &with);
    Mode m_mode = Mode::Typing;
    QVector<QPair<int, int>> m_wrong;
    QTimer m_spellTimer;
};

} // namespace jp
