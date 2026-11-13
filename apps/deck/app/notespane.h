#pragma once
// The speaker notes under the slide. What is typed goes into the slide's
// notes story as one undo step once typing pauses, or on leaving the pane.

#include "app/grammarcheck.h"

#include <QPlainTextEdit>
#include <memory>
#include <QTimer>

class QMenu;

namespace jp {

class Editor;

class NotesPane : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit NotesPane(Editor *ed, QWidget *parent = nullptr);
    void reload();   // shows the current slide's notes, unless typing waits to be saved
    void commit();   // saves what was typed into the slide
    bool hasTyping() const { return m_typed; }
    // Grammar as you type, as in the slides' text (the suite's setting): the issues of the notes now shown, a blue dotted line under each, and the right-click help for
    // the one at a position (false, and nothing added, when there is none).
    QVector<GrammarCheck::Issue> grammarIssues() const;
    bool addGrammarSuggestions(QMenu &menu, int position);
    void setGrammarDelayMs(int ms) { m_grammar->setDelayMs(ms); }

protected:
    void focusOutEvent(QFocusEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    void markGrammar();   // the blue dotted lines, for the issues now known (the rest are asked for)
    std::unique_ptr<GrammarCheck> m_grammar;
    Editor *m_ed;
    QTimer m_idle;
    QString m_pageId;
    bool m_loading = false;
    bool m_typed = false;   // typed since the last save or load
};

} // namespace jp
