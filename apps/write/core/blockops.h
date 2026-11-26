#pragma once
// Saved blocks in JeffWrite: a selection kept as a small document (the bytes of a .docx) in the library every app of the suite shares (libs/base/io/blocklibrary.h), and put in again at
// the caret as one step of the history. Text Shortcuts are blocks whose name, typed before the caret and followed by F3, becomes the block.

#include "core/writedoc.h"
#include "io/blocklibrary.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QTextCursor>

namespace jp::write {

// The bytes of a small document that holds the selection with its formatting: its paragraphs with the styles they use (the style sheet entries come along), tables, pictures, shapes, and
// fields. Empty, with `problem`, for no selection, for a selection that holds nothing a block can keep, or for one too large to keep (BlockLibrary::kMostEntryBytes).
// What a block does not carry as it is, in sentences for the person (`leftOut`, when given): part of a table is its words (one paragraph for each cell; a whole table is a table), footnotes and endnotes and comments are left out,
// tracked changes are accepted (the words as they read with every change taken), and section breaks are not kept.
QByteArray blockFromSelection(const WriteDoc &doc, const QTextCursor &selection, QString *problem = nullptr, QStringList *leftOut = nullptr);

// The first words of the selection, for the name box of the box that saves it.
QString suggestedBlockName(const QTextCursor &selection);
// What the selection has in it, for the galleries of the person's own tables and text boxes: a table (a paragraph in a cell), and a text box (a drawn object that holds text).
bool selectionHoldsTable(const QTextCursor &selection);
bool selectionHoldsTextBox(const QTextCursor &selection);

// Puts the block where the caret is (in place of a selection), as one step that undoes: `how` says whether it goes in as it is, in a paragraph of its own, or on a page of its own. The styles
// it uses that the document lacks are added (a style the document has under the same name is never changed; the block's paragraphs take the document's), and the fields in it are updated.
// False, with `problem`, for bytes that are no document or hold nothing, or where nothing can be put in (a header, a note, a paragraph kept as XML).
// Whether a block can be read and has something in it, so that a caller that clears a place for it first finds out before it does.
bool blockCanBeInserted(const QByteArray &block, QString *problem = nullptr);
bool insertBlock(WriteDoc &doc, QTextCursor &c, const QByteArray &block, BlockInsert how, QString *problem = nullptr);

// The Text Shortcut that ends just before the caret (the longest, without regard to case, that starts at the start of a word), or none.
struct ShortcutMatch {
    BlockInfo info;
    int start = -1;   // where the name begins in the story
};
std::optional<ShortcutMatch> shortcutBeforeCaret(const QTextCursor &c, const QVector<BlockInfo> &shortcuts);

// Takes the shortcut out and puts its block in its place, as one step. False when no shortcut is before the caret.
bool expandShortcut(WriteDoc &doc, QTextCursor &c, BlockLibrary &library, QString *problem = nullptr);

} // namespace jp::write
