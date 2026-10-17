#pragma once
// The Find and Replace pane, on the right of the window (Home > Editing, Ctrl+F and Ctrl+H). Find Next looks in the text of the document from
// the end of the selection, round to the start again, and shows each match selected, scrolling the pages to it. Replace swaps the match that
// is shown and goes to the next. Replace All swaps every one as a single step that undoes, and says how many there were. Hidden text is not
// searched, a bookmark or a field's code between two words does not part them (the search is of the characters that show, and a swap leaves those pieces where
// they are), and the search is made by the code of libs/page that JeffDeck's pane uses (app/findreplace.h), given JeffWrite's one story.

#include "app/findreplace.h"

#include <QTextDocument>
#include <QWidget>
#include <memory>

class QCheckBox;
class QLabel;
class QLineEdit;

namespace jp {

class StoryWords;
class WriteWindow;

class WriteFindPane : public QWidget {
    Q_OBJECT
public:
    explicit WriteFindPane(WriteWindow *win, QWidget *parent = nullptr);
    ~WriteFindPane() override;
    // Puts the cursor in Find what, or in Replace with.
    void focusBox(bool replace);
    // False, with the reason in status(), when there is no such text.
    bool findNext();
    void replaceOne();
    void replaceAll();
    QString status() const;
    QLineEdit *findBox() const { return m_find; }
    QLineEdit *replaceBox() const { return m_replace; }
    QCheckBox *matchCase() const { return m_case; }
    QCheckBox *wholeWords() const { return m_whole; }

private:
    QTextDocument::FindFlags flags() const;
    // The story's words as they are now (made again when the story has changed), and the search's way to them.
    StoryWords *storyWords();
    FindWords words();
    QVector<FindStory> stories() const;
    bool selectionIsMatch();
    WriteWindow *m_win;
    std::unique_ptr<StoryWords> m_text;
    QLineEdit *m_find, *m_replace;
    QCheckBox *m_case, *m_whole;
    QLabel *m_status;
};

} // namespace jp
