#pragma once
// The Spelling dialog of JeffCrunch: the text cells of the sheet, or of the cells selected, one misspelled word at a time, with Ignore, Ignore All, Change, Change All, and Add (to the
// user's dictionary). It looks at text cells only; formulas, numbers, and dates are left alone. A change is a step of Undo (Change All is one step for all its cells).

#include "core/spellwalk.h"

#include <QDialog>

#include <memory>
#include <optional>

class QLabel;
class QLineEdit;
class QListWidget;

namespace jp {

class CrunchWindow;

class SpellingDialog : public QDialog {
    Q_OBJECT
public:
    SpellingDialog(CrunchWindow *window, int sheet, const jp::office::sheet::CellRange &range);
    // Goes to the first misspelled word. False when there is none (the dialog then says the check is complete, unless `quiet`).
    bool start(bool quiet = false);
    // The word the dialog stands on, and what it will be changed to (typed over, or the first suggestion).
    std::optional<crunch::SpellHit> current() const { return m_hit; }
    void setReplacement(const QString &text);
    // The buttons' work. Each goes on to the next misspelled word; false when there is none.
    bool ignore();
    bool ignoreAll();
    bool change();
    bool changeAll();
    bool addToDictionary();
    // The longest the dialog held the window while it looked for a word, in milliseconds (for tests); and whether it was given up (Close) while looking.
    qint64 longestSliceMs() const { return m_longestSlice; }

private:
    bool advance();
    CrunchWindow *m_win;
    int m_sheet;
    std::unique_ptr<crunch::SpellWalk> m_walk;
    std::optional<crunch::SpellHit> m_hit;
    QLabel *m_notIn;
    QLineEdit *m_change;
    QListWidget *m_suggestions;
    QLabel *m_progress;
    QList<QPushButton *> m_buttons;
    bool m_quiet = false, m_looking = false, m_cancelled = false;
    qint64 m_longestSlice = 0;
};

} // namespace jp
