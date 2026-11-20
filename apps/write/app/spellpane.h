#pragma once
// Spelling in the window: the help the right-click menu gives on a misspelled word, and the Spelling pane that steps through the misspellings.

#include "app/grammarcheck.h"
#include "core/spellcheck.h"

#include <map>
#include <QPair>
#include <QPointer>
#include <QTimer>
#include <QWidget>
#include <memory>

class QLabel;
class QTextDocument;
class QListWidget;
class QMenu;
class QPushButton;

namespace jp::write {

class PageView;

// When the caret is on a misspelled word: the dictionary's suggestions (the first six, the first in bold; each puts itself in the word's place), Ignore All,
// and Add to Dictionary, added to the menu. False, and nothing added, when it is not.
bool addSpellingSuggestions(QMenu &menu, PageView *view);

// When the caret is on a grammar or style issue (the blue dotted line): its message (a line that does nothing), the suggestions (the first in bold; each puts itself in the
// issue's place, as one step that undoes), Ignore This Issue (this place only), and Ignore All (these words with this message), added to the menu. False, and nothing added, when it is not.
bool addGrammarSuggestions(QMenu &menu, PageView *view);

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
    // The pane goes through the spelling first and then, if grammar is checked, the grammar and style issues: whether it is at one now, and its message.
    bool inGrammar() const { return m_grammarPhase && m_issue.valid(); }
    QString message() const { return m_issue.valid() ? m_issue.message : QString(); }
    // Looking for the next misspelled word, in slices of time (a long document takes seconds to check, and the window is not held up for them).
    bool searching() const { return m_searching; }

protected:
    void hideEvent(QHideEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    void collectStories();
    SpellCheck *checkFor(QTextDocument *story);
    bool reveal(QTextDocument *story);
    bool nextStory();
    void refreshChecks();
    void find(int from);
    void search();
    void checking();
    void show(QTextDocument *story, const QPair<int, int> &range);
    void spellingDone();
    void searchGrammar();
    void showIssue(const GrammarCheck::Issue &issue);
    void finish();
    bool stillThere();
    void change(bool all);
    void ignore(bool all);
    void addToDictionary();

    PageView *m_view;
    QPair<int, int> m_range{-1, -1};
    QString m_word;
    // The stories the check goes through (the text, the notes, the headers and footers), the one it is in and the one it began in, the pane's own checks of them, and the story of
    // the word shown.
    QVector<QPointer<QTextDocument>> m_order;
    int m_idx = 0, m_startIdx = 0;
    std::map<QTextDocument *, std::unique_ptr<SpellCheck>> m_checks;
    QPointer<QTextDocument> m_story;
    int m_start = 0;                  // where the check began (the caret's place in its story, as the text changes)
    int m_pos = 0;                    // where the search is
    bool m_wrapped = false, m_complete = false, m_searching = false;
    bool m_grammarPhase = false, m_grammarRan = false;
    GrammarCheck::Issue m_issue;
    QTimer m_timer;
    QLabel *m_status = nullptr, *m_wordLabel = nullptr, *m_context = nullptr;
    QListWidget *m_list = nullptr;
    QPushButton *m_change = nullptr, *m_changeAll = nullptr, *m_ignore = nullptr, *m_ignoreAll = nullptr, *m_add = nullptr;
};

} // namespace jp::write
