#pragma once
// The dialog boxes that are JeffWrite's own: Font, Paragraph, and Zoom.

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
// The zoom: a percent, the width of the page, the whole page, or several pages across.
void zoomDialog(QWidget *parent, PageView *view);

} // namespace jp::write
