#pragma once
// Find and Replace across a document's stories (the text of every box, shape,
// and table cell, and each slide's speaker notes): the stories in the order to
// search them, the walk that finds the next match, Replace All, and showing a
// match. Each app's Find pane is built on it.

#include <QString>
#include <QTextCursor>
#include <QTextDocument>
#include <QVector>

namespace jp {

class Document;
class Editor;

// A story to search and where it sits, so that a match in it can be shown.
struct FindStory {
    QString itemId, storyId;
    int row = -1, col = -1;   // table cell
    int page = -1;            // the page it is on, -1 on a master page
    QString master;           // the master page it is on, empty on a page
    int half = 0;             // 1: the second half of a cell divided by a diagonal
    bool notes = false;       // a page's speaker notes (no item)
};

struct FindScope {
    bool masters = true;      // the master pages' stories, after the pages'
    bool notes = false;       // each page's speaker notes, after the page's own stories
    int firstPage = 0;        // the page to start at: the pages before it come last
};

// Every story once, in the order to search them: the pages (a text box's whole
// chain once, at its first frame), then the master pages.
QVector<FindStory> findStories(const Document &doc, const FindScope &scope = FindScope());

// The first match of `text`, looking in stories[start] from `pos` on (-1: from
// its end), then in each story after it (before it, with FindBackward) and
// round again to `start`. False when there is none; else `story` is the
// index of the story it is in and `found` selects the match.
bool findInStories(const Document &doc, const QVector<FindStory> &stories, const QString &text, QTextDocument::FindFlags flags, int start, int pos,
                   int *story, QTextCursor *found);

// Replaces every match of `text` in the stories with `with`, and says how
// many there were. The caller makes it one change.
int replaceInStories(const Document &doc, const QVector<FindStory> &stories, const QString &text, QTextDocument::FindFlags flags, const QString &with);

// Brings the match to the screen and selects it: its page (or master page),
// with the text cursor in its box or cell. Speaker notes are shown by the
// caller, who has the pane that holds them: this only goes to their page.
void showFindMatch(Editor *ed, const FindStory &story, const QTextCursor &found);

} // namespace jp
