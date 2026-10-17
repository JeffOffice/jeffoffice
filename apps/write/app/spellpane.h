#pragma once
// Spelling in the window: the help the right-click menu gives on a misspelled word, and the Spelling pane that steps through the misspellings.

#include <QPair>
#include <QWidget>

class QLabel;
class QListWidget;
class QMenu;
class QPushButton;

namespace jp::write {

class PageView;

// When the caret is on a misspelled word: the dictionary's suggestions (the first six, the first in bold; each puts itself in the word's place), Ignore All,
// and Add to Dictionary, added to the menu. False, and nothing added, when it is not.
bool addSpellingSuggestions(QMenu &menu, PageView *view);

// The Spelling pane: shows the first misspelled word from the caret (selected in the text) with the dictionary's suggestions, and goes on to the next as each
// is changed, ignored, or added to the dictionary. It begins where the caret is, goes to the end, then from the top to where it began.
class SpellPane : public QWidget {
    Q_OBJECT
public:
    explicit SpellPane(PageView *view, QWidget *parent = nullptr);

    // Begins again from the caret (or the start of what is selected).
    void start();
    // The word being shown, empty when there is none (the check is complete).
    QString word() const { return m_word; }
    bool complete() const { return m_complete; }

private:
    void find(int from);
    void show(const QPair<int, int> &range);
    void finish();
    bool stillThere();
    void change(bool all);
    void ignore(bool all);
    void addToDictionary();

    PageView *m_view;
    QPair<int, int> m_range{-1, -1};
    QString m_word;
    int m_start = 0;
    bool m_wrapped = false, m_complete = false;
    QLabel *m_status = nullptr, *m_wordLabel = nullptr, *m_context = nullptr;
    QListWidget *m_list = nullptr;
    QPushButton *m_change = nullptr, *m_changeAll = nullptr, *m_ignore = nullptr, *m_ignoreAll = nullptr, *m_add = nullptr;
};

} // namespace jp::write
