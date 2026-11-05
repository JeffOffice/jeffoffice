#pragma once
// Insert > Drop Cap: the first letters of a paragraph set large, dropped into the lines beside the text. The paragraph holds the setting as the text engine's drop cap (its letters stay
// in the paragraph, so editing goes on as ever); the .docx file has them as Word-style files do: a paragraph of their own, a frame set as a drop cap, before the rest of the paragraph.

#include "core/writedoc.h"

#include <QTextCursor>

namespace jp::write {

struct DropCapSetup {
    int lines = 0;                  // how many lines of text tall the letters are: 0 for none
    int chars = 1;                  // how many letters are dropped
    QString font;                   // the letters' font; empty for the paragraph's own
    bool operator==(const DropCapSetup &o) const { return lines == o.lines && chars == o.chars && font == o.font; }
};
constexpr int kMostDropCapLines = 10, kMostDropCapChars = 10;

// The drop cap of the paragraph a block is: the one made here, or the frame a file has (a paragraph of its own set as a drop cap, or the paragraph after it, whose text it begins).
DropCapSetup dropCapOf(const QTextBlock &block);
// Whether the block is a paragraph a file sets as a drop cap (a frame of the big letters, whose word goes on in the paragraph after it).
bool isDropCapFrame(const QTextBlock &block);
// Whether the paragraph can have a drop cap made here: it has letters first, is not in a list or a table, and is not kept as XML.
bool canHaveDropCap(const QTextBlock &block);
// Sets (or, with 0 lines, takes away) the drop cap of the paragraph the caret is in, as one step; false where it can't be (see canHaveDropCap, and a file's own drop cap is not changed).
bool setDropCap(WriteDoc &doc, QTextCursor &c, const DropCapSetup &setup);

} // namespace jp::write
