#pragma once
// The Find and Replace pane, on the right of the window (Home > Editing, Ctrl+F and Ctrl+H). Find Next looks at the text of every
// slide in order from the one being edited (titles, text boxes, shapes, table cells, and, unless turned off, the speaker notes),
// round to the start again, and shows each match on its slide, selected. Replace swaps the match that is shown and goes to the
// next. Replace All swaps every one as a single step that undoes, and says how many there were.

#include "app/findreplace.h"

#include <QTextDocument>
#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;

namespace jp {

class DeckWindow;

class FindPane : public QWidget {
    Q_OBJECT
public:
    explicit FindPane(DeckWindow *win, QWidget *parent = nullptr);
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
    DeckWindow *m_win;
    QLineEdit *m_find, *m_replace;
    QCheckBox *m_case, *m_whole, *m_notes;
    QLabel *m_status;
    // The last match shown in the speaker notes: its story, and where it ended (the notes' cursor is there until it moves).
    QString m_noteStory;
    int m_noteEnd = -1;
};

} // namespace jp
