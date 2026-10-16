#pragma once
// Editing the story: typing, deleting, moving, formatting characters and paragraphs, lists, and styles. Each is a command on the story's
// text cursor that the story's undo stack takes as one step (a command made of several changes is one edit block). The window and the
// page view call these; they know nothing of pages.

#include "core/writedoc.h"

#include <QColor>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <functional>

namespace jp::write {

// A paragraph that stands for an element kept as XML (a table): not edited.
bool isLocked(const QTextBlock &block);
// Text that is kept as XML and shows nothing (a bookmark, a field's code).
bool isInvisibleAtom(const QTextCharFormat &format);

// ---- typing and deleting
void insertText(WriteDoc &doc, QTextCursor &c, const QString &text);
void insertParagraph(WriteDoc &doc, QTextCursor &c);
// The page break (Ctrl+Enter): the paragraph is cut here and the rest goes on the next page.
void insertPageBreak(WriteDoc &doc, QTextCursor &c);
// A section break: the next section starts on a new page, alike the one the cursor is in.
void insertSectionBreak(WriteDoc &doc, QTextCursor &c);
void deleteBack(WriteDoc &doc, QTextCursor &c, bool byWord);
void deleteForward(WriteDoc &doc, QTextCursor &c, bool byWord);
void removeSelection(WriteDoc &doc, QTextCursor &c);
// The tab key: a tab character, or a deeper (or shallower, with `back`) level in a list.
void insertTab(WriteDoc &doc, QTextCursor &c, bool back);

// ---- the clipboard
// The selection as text and as a fragment that keeps its formats (null fragment when nothing is selected).
QTextDocumentFragment selectionFragment(const QTextCursor &c);
void pasteFragment(WriteDoc &doc, QTextCursor &c, const QTextDocumentFragment &fragment);
void pasteText(WriteDoc &doc, QTextCursor &c, const QString &text);

// ---- moving
// A move of the cursor that does not stop beside the invisible pieces of the file (a step is a letter and the bookmarks it carries).
void moveCursor(QTextCursor &c, QTextCursor::MoveOperation op, bool select, int n = 1);

// ---- characters
// Gives the characters selected (the text typed next, with none selected) the format that `change` makes of theirs.
void changeCharFormat(WriteDoc &doc, QTextCursor &c, const std::function<void(QTextCharFormat &)> &change);
bool allSelected(const QTextCursor &c, const std::function<bool(const QTextCharFormat &)> &test);
void toggleBold(WriteDoc &doc, QTextCursor &c);
void toggleItalic(WriteDoc &doc, QTextCursor &c);
void toggleUnderline(WriteDoc &doc, QTextCursor &c);
void toggleStrike(WriteDoc &doc, QTextCursor &c);
void toggleSubscript(WriteDoc &doc, QTextCursor &c);
void toggleSuperscript(WriteDoc &doc, QTextCursor &c);
void setFontFamily(WriteDoc &doc, QTextCursor &c, const QString &family);   // "+Heading" and "+Body" follow the theme
void setFontSize(WriteDoc &doc, QTextCursor &c, double points);
void changeFontSize(WriteDoc &doc, QTextCursor &c, int steps);               // Grow and Shrink: along the sizes of the size box
void setTextColor(WriteDoc &doc, QTextCursor &c, const QColor &color);       // invalid: automatic
void setHighlight(WriteDoc &doc, QTextCursor &c, const QString &name);       // empty: none
void clearFormatting(WriteDoc &doc, QTextCursor &c);

// ---- paragraphs
void changeBlockFormat(WriteDoc &doc, QTextCursor &c, const std::function<void(QTextBlockFormat &)> &change);
void setAlignment(WriteDoc &doc, QTextCursor &c, Qt::Alignment alignment);
void changeIndent(WriteDoc &doc, QTextCursor &c, double points);             // by a step; in a list, the level
void setIndents(WriteDoc &doc, QTextCursor &c, double left, double right, double firstLine);
void setSpacing(WriteDoc &doc, QTextCursor &c, double before, double after);
// Line spacing: a multiple of a line (1, 1.15, 2...), or exactly or at least that many points.
void setLineSpacing(WriteDoc &doc, QTextCursor &c, QTextBlockFormat::LineHeightTypes type, double value);

// ---- lists and styles
enum class ListKind { None, Bullets, Numbers };
ListKind listKindOf(const WriteDoc &doc, const QTextBlock &block);
void toggleList(WriteDoc &doc, QTextCursor &c, bool numbered);
void applyParagraphStyle(WriteDoc &doc, QTextCursor &c, const QString &styleId);
void applyCharacterStyle(WriteDoc &doc, QTextCursor &c, const QString &styleId);
// The style the first selected paragraph has (its id).
QString paragraphStyleAt(const WriteDoc &doc, const QTextCursor &c);

// The paragraphs that the selection touches (the one the cursor is in without one).
QVector<QTextBlock> selectedBlocks(const QTextCursor &c);

} // namespace jp::write
