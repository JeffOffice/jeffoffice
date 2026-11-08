#pragma once
// The line a person types a cell's contents into, over the cell. It knows the keys of an entry: Enter and Tab (and with Shift) finish it, and say which way the selection goes on;
// Escape gives it up; F2 changes between typing over a cell (where an arrow finishes the entry and moves on) and editing what is in it (where an arrow moves in the text); Alt+Enter
// starts a new line in the text. The window does the rest. While an entry is typed, words that are not in the dictionary are underlined in red, the right-click menu offers the
// dictionary's words, and AutoCorrect fixes what is typed (a typing slip, capitals, smart quotes, dashes, fractions); none of it inside a formula.

#include <QLineEdit>
#include <QPair>
#include <QTimer>
#include <QVector>
#include <QFont>
#include <optional>
#include "text/autocorrect.h"

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
    // How many times the characters of an entry were measured for the underlines (once for a text and a font, not at every paint; for tests).
    int measureCount() const { return m_measureCount; }
    // How many times the entry was looked at for misspelled words (for tests: none while "Check spelling as you type" is off).
    int checkCount() const { return m_checkCount; }
    // Whether Options > Proofing has "Check spelling as you type" on (it is, unless turned off).
    static bool spellingOn();
    void checkSpelling();
    // Puts the dictionary's words for the misspelled word at `position`, Ignore All, and Add to Dictionary in the menu, and a separator after them. False, with nothing put in, if the
    // word there is spelled right (or the entry is a formula).
    bool addSpellingSuggestions(QMenu &menu, int position);
    // Whether the entry is a formula (it starts with = , or with + or - and then something that is not a number): AutoCorrect leaves it alone.
    static bool isFormulaEntry(const QString &text);
    // The most characters of an entry that are looked at for misspelled words as it is typed.
    static constexpr int kSpellLimit = 20000;

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
    static std::optional<jp::autotext::Fix> proseFix(const QString &before, QChar typed, bool smartQuotes);
    Mode m_mode = Mode::Typing;
    QVector<QPair<int, int>> m_wrong;
    QTimer m_spellTimer;
    QString m_measuredText;       // the text whose characters are measured in m_advances (how far along the line each starts), with the font they were measured in
    QFont m_measuredFont;
    QVector<int> m_advances;
    int m_measureCount = 0, m_checkCount = 0;
};

} // namespace jp
