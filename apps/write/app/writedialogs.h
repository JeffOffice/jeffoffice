#pragma once
// The dialog boxes that are JeffWrite's own: Font, Paragraph, and Zoom.

#include <QString>

class QTextCursor;
class QWidget;

namespace jp::write {

class PageView;
class WriteDoc;

// The font, style, size, color, underline (its style and color), and effects of the characters the cursor selects (of the text typed next, with none
// selected). What the person changed is given to the characters as one step that undoes; what they left alone stays as it was in each of them.
void fontDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor);
// Alignment, indents, spacing, and line spacing of the paragraphs the cursor selects (those it changes, and only those the person changed).
void paragraphDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor);
// The Page Setup dialog for the section `section`: margins and gutter, orientation, paper size, and layout (how the section starts, a different first page, different odd and even
// pages, and the header and footer distances), applied to the section or to the whole document when it is accepted, as one step that undoes. Only what the person changed is applied.
// Returns whether anything was applied.
bool pageSetupDialog(QWidget *parent, WriteDoc *doc, int section);
// Create New Style from Formatting (`modifyId` empty: it begins with the look of the selection, and the style made is given to the selection) and Modify Style (the style `modifyId`):
// name, kind, what it is based on, the style that follows it, font, paragraph look, whether it is in the gallery, and whether it updates itself. OK makes or changes the style as one
// step that undoes; a name or a base that is refused keeps the dialog open and says why. Returns whether a style was made or changed.
bool styleDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor, const QString &modifyId = QString());
// Go To: a page, section, line, bookmark, footnote, or heading by number or name; the caret is put there. Returns whether it went anywhere.
bool goToDialog(QWidget *parent, WriteDoc *doc, PageView *view);
// The alt text of a picture: a description for people who can't see it. Returns false if canceled; `alt` is what to keep.
bool altTextDialog(QWidget *parent, QString *alt);
// The zoom: a percent, the width of the page, the whole page, or several pages across.
void zoomDialog(QWidget *parent, PageView *view);

} // namespace jp::write
