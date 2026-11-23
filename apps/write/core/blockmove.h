#pragma once
// Moving a run of whole paragraphs of the text to another place between paragraphs, exactly: every paragraph keeps its own look (the format of the paragraph and of its mark, its place in a
// table, its section break), every character keeps its own format (pictures, drawn shapes, notes, fields, bookmarks, comments, tracked changes), and no paragraph outside the run changes.
// The run is cut and put back with the paragraph break before it, as one step of the history; nothing goes through the clipboard's paste, which would settle the pieces it pastes.

#include "core/writedoc.h"

#include <QString>

namespace jp::write {

// Moves the paragraphs from the one holding `first` to the one holding `last` to before the paragraph holding `before` (the end of the text when `before` is negative). False where nothing
// moved: `why` (if given) says why when the move was refused, and is left empty when the run is already there. Where the first moved paragraph is now goes in `movedTo`.
// Refused: Track Changes on, a protected document, endnotes set at the ends of sections, a run or a place that cuts a table, a table put against another, a run that holds one end of a
// bookmark, comment, field, or content control whose other end is outside it, and a run that holds a section break unless it is whole sections that go between sections.
bool moveBlocks(WriteDoc &doc, int first, int last, int before, QString *why = nullptr, int *movedTo = nullptr);

// For tests: how many paragraphs the last call looked at or changed.
qint64 moveBlocksWorkForTest();

} // namespace jp::write
