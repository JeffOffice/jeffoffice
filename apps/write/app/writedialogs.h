#pragma once
// The dialog boxes that are JeffWrite's own: Font, Paragraph, and Zoom.

#include <QString>
#include <QVector>

class QTextCursor;
class QWidget;

namespace jp::write {

class PageView;
class WriteDoc;
struct Source;

// The font, style, size, color, underline (its style and color), and effects of the characters the cursor selects (of the text typed next, with none
// selected). What the person changed is given to the characters as one step that undoes; what they left alone stays as it was in each of them.
void fontDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor);
// Alignment, indents, spacing, and line spacing of the paragraphs the cursor selects (those it changes, and only those the person changed).
void paragraphDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor);
// The Page Setup dialog for the section `section`: margins and gutter, orientation, paper size, and layout (how the section starts, a different first page, different odd and even
// pages, and the header and footer distances), applied to the section or to the whole document when it is accepted, as one step that undoes. Only what the person changed is applied.
// Returns whether anything was applied.
bool pageSetupDialog(QWidget *parent, WriteDoc *doc, int section);
// More Columns: the number of columns (up to 12), equal widths or each column's own width and space, a line between, and presets (One, Two, Three, Left, Right); applied to the
// section or the whole document as one step that undoes. Returns whether anything was changed.
bool columnsDialog(QWidget *parent, WriteDoc *doc, int section);
// Line Numbers: on or off, start at, count by, distance from the text, and when the numbers begin again; for the section or the whole document.
bool lineNumbersDialog(QWidget *parent, WriteDoc *doc, int section);
// Page Borders: which sides, the line (style, width, color), its distance, where it is measured from, and the pages that have it; for the section or the whole document.
bool pageBordersDialog(QWidget *parent, WriteDoc *doc, int section);
// Hyperlink: the address (a web or email address) or the bookmark a link goes to, the words it shows, and a screen tip; for a link the caret is in it edits, and Remove Link takes it off.
// OK stays open, saying why, until the link can be made. Returns whether the text was changed.
bool linkDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Create New Style from Formatting (`modifyId` empty: it begins with the look of the selection, and the style made is given to the selection) and Modify Style (the style `modifyId`):
// name, kind, what it is based on, the style that follows it, font, paragraph look, whether it is in the gallery, and whether it updates itself. OK makes or changes the style as one
// step that undoes; a name or a base that is refused keeps the dialog open and says why. Returns whether a style was made or changed.
bool styleDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor, const QString &modifyId = QString());
// Go To: a page, section, line, bookmark, footnote, or heading by number or name; the caret is put there. Returns whether it went anywhere.
bool goToDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Bookmark: add the name typed at the selection, delete one, go to one, sort the list by name or by place, and show the hidden ones. Returns whether the text was changed or the caret moved.
bool bookmarkDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Cross-reference: a heading, bookmark, footnote, endnote, numbered item, or caption, shown as its text, number, page, or "above" or "below", inserted at the caret (as a link or not).
bool crossReferenceDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Field: a field of the list and its code to be edited; OK puts it at the caret with the result worked out.
bool fieldDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Table of Contents: the levels, whether page numbers show and line up at the right, the leader, and whether lines are links; OK puts the table at the caret.
bool tableOfContentsDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Update Table of Contents: page numbers only, or the entire table. Returns 0 for neither (canceled), 1 for page numbers only, and 2 for the entire table.
int updateTableDialog(QWidget *parent);
// Mark Table of Contents Entry: the entry's text, its level, and the table it is for; puts a table entry field at the caret.
bool tocEntryDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Insert Caption: the label, the text, above or below, with the chapter number or without the label; puts the caption next to the selection.
bool captionDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Table of Figures: the caption label and the form of the table; puts it at the caret.
bool tableOfFiguresDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Mark Index Entry: the main entry and subentry, the page (current, a range from a bookmark, or a cross-reference), and the look of the number; Mark or Mark All.
bool markIndexEntryDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Index: columns, tab leader, headings, and indented or run-in; puts an index at the caret.
bool indexDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Mark Citation (for a table of authorities): the long and the short citation and the category.
bool markCitationDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// Table of Authorities: the category (or all), passim, headings, and the tab leader.
bool tableOfAuthoritiesDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// One source (its type, authors, and the values its type asks for), made or changed in `source`; `others` are the tags that are taken. Returns false if canceled.
bool sourceDialog(QWidget *parent, Source *source, const QVector<Source> &others);
// The Source Manager: the master list the documents share beside this document's own list, with copy, delete, edit, and new. Returns whether the document's list changed.
bool manageSourcesDialog(QWidget *parent, WriteDoc *doc);
// Insert Citation: pick a source (or make one), the pages, and what to leave out; puts a citation at the caret.
bool insertCitationDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// The style citations and the bibliography are shown in. Returns whether it changed.
bool citationStyleDialog(QWidget *parent, WriteDoc *doc);
// Footnote and Endnote options: how each kind of note is numbered (format, start, and when the numbers begin again) and where the endnotes are set. What is chosen is set in the document
// (WriteDoc::setNoteSettings); `endnotesChanged` says whether the endnotes' were, which the document has to be read again for. Returns false if canceled or nothing changed.
bool notesOptionsDialog(QWidget *parent, WriteDoc *doc, bool *endnotesChanged);
// References > Convert: all footnotes to endnotes, all endnotes to footnotes, each to the other, or just the note the caret is in. True where notes were converted.
bool convertNotesDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// References > Separators: the line between the text and the notes, the line above a note that goes on, and the words below it, for the footnotes and the endnotes. `endnotesChanged` is
// set where an endnote's was changed (the window then reads the document again from what it would save).
bool noteSeparatorsDialog(QWidget *parent, WriteDoc *doc, bool *endnotesChanged);
// The alt text of a picture: a description for people who can't see it. Returns false if canceled; `alt` is what to keep.
bool altTextDialog(QWidget *parent, QString *alt);
// The zoom: a percent, the width of the page, the whole page, or several pages across.
void zoomDialog(QWidget *parent, PageView *view);

} // namespace jp::write
