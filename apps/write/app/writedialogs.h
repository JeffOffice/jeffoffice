#pragma once
// The dialog boxes that are JeffWrite's own: Paragraph, and Zoom.

class QTextCursor;
class QWidget;

namespace jp::write {

class PageView;
class WriteDoc;

// Alignment, indents, spacing, and line spacing of the paragraphs the cursor selects (those it changes, and only those the person changed).
void paragraphDialog(QWidget *parent, WriteDoc *doc, QTextCursor *cursor);
// The zoom: a percent, the width of the page, the whole page, or several pages across.
void zoomDialog(QWidget *parent, PageView *view);

} // namespace jp::write
