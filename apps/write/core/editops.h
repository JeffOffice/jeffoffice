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
// The character format of a paragraph's mark (what an empty paragraph is drawn with, and the paragraph mark's own formatting in the file). The engine
// can't undo a change to the first paragraph's, so that one is kept in the paragraph's format as well, and read from there.
QTextCharFormat markFormat(const QTextBlock &block);
void setMarkFormat(QTextBlock block, const QTextCharFormat &format);
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
// A fragment pasted where the cursor is, as a copy from this same document (what its kept pieces refer to is this document's).
void pasteFragment(WriteDoc &doc, QTextCursor &c, const QTextDocumentFragment &fragment);
void pasteText(WriteDoc &doc, QTextCursor &c, const QString &text);
// What a fragment says in plain text: with the pieces kept as XML left out, and a line break as a new line.
QString plainTextOf(const QTextDocumentFragment &fragment);

// A copy that can be pasted into another document: the fragment, and what the pieces kept as XML in it refer to in the document it came from (a
// picture's bytes, a hyperlink's address). Pasting it into another document carries pictures and hyperlinks over, with relationships of the
// new document's own; any other piece that refers to a part of the file (a chart, say) the new document would not have is not carried,
// and the text it had is pasted as text, so that nothing is saved that points at a part the file lacks.
struct Clip {
    struct Link {
        QString id, type, target;
        bool external = false;
        QByteArray data;            // the bytes of a part that is a picture
        QString contentType;
    };
    QTextDocumentFragment fragment;
    quint64 source = 0;             // the document it was copied from
    quint64 generation = 0;         // the store of kept bytes its formats refer to (when the last document is closed, those are gone)
    QVector<Link> links;
    bool isEmpty() const { return fragment.isEmpty(); }
};
Clip copySelection(const WriteDoc &doc, const QTextCursor &c);
void pasteClip(WriteDoc &doc, QTextCursor &c, const Clip &clip);

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
// The spacing before or after a paragraph, in points, as a person sets it: it replaces the automatic spacing the paragraph had on that side (on the page, and in
// the file, which keeps no automatic spacing the person has set aside).
void setSpaceBefore(QTextBlockFormat &bf, double points);
void setSpaceAfter(QTextBlockFormat &bf, double points);
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
