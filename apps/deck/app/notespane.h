#pragma once
// The speaker notes under the slide: the formatted story of the slide's notes, edited in place through the editor (libs/page/app/storyedit), so every change is a step of
// the presentation's undo history and the ribbon's text commands, AutoCorrect, and paste work here as they do in a text box.

#include "app/grammarcheck.h"
#include "app/storyedit.h"

#include <functional>
#include <memory>

class QMenu;

namespace jp {

class Editor;

class NotesPane : public StoryEdit {
    Q_OBJECT
public:
    explicit NotesPane(Editor *ed, QWidget *parent = nullptr);
    ~NotesPane() override;
    void reload();   // shows the current slide's notes (the story of the slide, as the document has it now)
    void commit();   // closes what was typed into one undo step
    // Grammar as you type, as in the slides' text (the suite's setting): the issues of the notes now shown, a blue dotted line under each, and the right-click help for
    // the one at a position (false, and nothing added, when there is none).
    QVector<GrammarCheck::Issue> grammarIssues() const;
    bool addGrammarSuggestions(QMenu &menu, int position);
    void setGrammarDelayMs(int ms) { m_grammar->setDelayMs(ms); }
    // The window's commands for the right-click menu, below the spelling and grammar help.
    void setMenuFiller(std::function<void(QMenu &)> fill) { m_fill = std::move(fill); }

protected:
    QString makeStory() override;
    void fillMenu(QMenu &menu, int position) override;

private:
    void markGrammar();   // the blue dotted lines, for the issues now known (the rest are asked for)
    std::unique_ptr<GrammarCheck> m_grammar;
    Editor *m_ed;
    QString m_pageId;
    std::function<void(QMenu &)> m_fill;
};

} // namespace jp
