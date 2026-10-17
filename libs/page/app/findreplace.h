#pragma once
// Find and Replace across a document's stories (the text of every box, shape,
// and table cell, each slide's speaker notes, and the words of a Text Art): the
// stories in the order to search them, the walk that finds the next match,
// Replace All, and showing a match. Each app's Find pane is built on it.

#include <QString>
#include <QTextCursor>
#include <QTextDocument>
#include <QVector>
#include <functional>
#include <memory>

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
    // A Text Art's words (no story): a copy to search, numbered as the characters of the words are. See FindScope::textArt.
    std::shared_ptr<QTextDocument> art;
};

struct FindScope {
    bool masters = true;      // the master pages' stories, after the pages'
    bool notes = false;       // each page's speaker notes, after the page's own stories
    int firstPage = 0;        // the page to start at: the pages before it come last
    bool textArt = false;     // the words of each Text Art, in the page's list with the boxes
};

// How a search reaches the words of a story, for a document that is not a Document of placed objects (JeffWrite's is one long story): the
// words of a story (null for none), whether a match is to be passed over (words that are not shown), and how a match gets its replacement
// (it leaves the cursor after the new words; empty: QTextCursor::insertText, which gives them the look of the match's last character).
struct FindWords {
    std::function<QTextDocument *(const FindStory &)> story;
    std::function<bool(const QTextCursor &match)> skip;
    std::function<void(QTextCursor &match, const QString &with)> put;
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
bool findInStories(const FindWords &words, const QVector<FindStory> &stories, const QString &text, QTextDocument::FindFlags flags, int start, int pos,
                   int *story, QTextCursor *found);

// Replaces every match of `text` in the stories with `with`, and says how
// many there were. `changed` gets each story that had a match. The caller
// makes it one change, and fits what grew (refitFoundStories).
int replaceInStories(const Document &doc, const QVector<FindStory> &stories, const QString &text, QTextDocument::FindFlags flags, const QString &with,
                     QVector<FindStory> *changed = nullptr);
int replaceInStories(const FindWords &words, const QVector<FindStory> &stories, const QString &text, QTextDocument::FindFlags flags, const QString &with,
                     QVector<FindStory> *changed = nullptr);

// Fits each box and table that had words replaced to its new words, as typing in it would: a text box that grows to fit grows
// (with the frames linked to it), and a table's rows grow. Once for each, in the change the replacing is in.
void refitFoundStories(Editor *ed, const QVector<FindStory> &changed);

// Puts `with` in place of the characters [start, end) of a Text Art's words (as the search numbers them) when they are `expected`
// (the one a search found), and says whether it did.
bool replaceArtWords(const Document &doc, const QString &itemId, int start, int end, const QString &expected, Qt::CaseSensitivity cs, const QString &with);

// What a message says of the text that was searched for: the first 40 characters (then an ellipsis), and long runs without a space
// broken with invisible spaces, so that a long word can't make the pane that shows it wider.
QString findTextForMessage(const QString &text);

// Brings the match to the screen and selects it: its page (or master page),
// with the text cursor in its box or cell; a Text Art is selected. Speaker notes
// are shown by the caller, who has the pane that holds them: this only goes to
// their page.
void showFindMatch(Editor *ed, const FindStory &story, const QTextCursor &found);

} // namespace jp
