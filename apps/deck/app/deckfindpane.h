#pragma once
// The Find and Replace pane, on the right of the window (Home > Editing, Ctrl+F and Ctrl+H). Find Next looks at the text of every
// slide in order from the one being edited (titles, text boxes, shapes, table cells, the words of a Text Art, and, unless turned off,
// the speaker notes), round to the start again, and shows each match on its slide, selected (a Text Art is selected whole). Replace
// swaps the match that is shown and goes to the next. Replace All swaps every one as a single step that undoes, fits what the new
// words need, and says how many there were.

#include "app/findreplace.h"

#include <QTextDocument>
#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;

namespace jp {

class DeckWindow;

class DeckFindPane : public QWidget {
    Q_OBJECT
public:
    explicit DeckFindPane(DeckWindow *win, QWidget *parent = nullptr);
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
    QCheckBox *speakerNotes() const { return m_notes; }

private:
    QTextDocument::FindFlags flags() const;
    FindScope scope(int firstPage) const;
    void showMatch(const FindStory &story, const QTextCursor &found);
    bool notesMatchShown() const;
    bool artMatchShown() const;
    DeckWindow *m_win;
    QLineEdit *m_find, *m_replace;
    QCheckBox *m_case, *m_whole, *m_notes;
    QLabel *m_status;
    // The last match shown in the speaker notes: its story, and where it ended (the notes' cursor is there until it moves).
    QString m_noteStory;
    int m_noteEnd = -1;
    // The last match shown in a Text Art's words: the Text Art (selected) and where the match was. Cleared as soon as another is shown.
    QString m_artItem;
    int m_artStart = -1, m_artEnd = -1;
};

} // namespace jp
