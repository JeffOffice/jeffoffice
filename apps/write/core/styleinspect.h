#pragma once
// The Style Inspector's facts: the paragraph style and the character style where the caret is, and what the text says of its own on top of them (direct formatting), named for a person; and
// the two commands that take that direct formatting off. Reveal Formatting shows the look itself (homeops.h); this is where it comes from.

#include "core/writedoc.h"

#include <QStringList>
#include <QTextCursor>

namespace jp::write {

struct AppliedFormatting {
    QString paragraphStyle, paragraphStyleName;    // the id and the name (the default paragraph style when the paragraph names none)
    QString characterStyle, characterStyleName;    // empty when the text has none
    QStringList paragraphDirect, characterDirect;  // what the paragraph and the letters say besides their styles ("Alignment", "Bold", ...), empty when nothing
};

// What applies at the start of the selection (or at the caret).
AppliedFormatting appliedFormattingAt(WriteDoc &doc, const QTextCursor &c);
// Takes the paragraph's own alignment, indents, spacing, lines, and tab stops off, so that its style says them (one step); the style, the list, and the file's own pieces stay.
void clearDirectParagraphFormatting(WriteDoc &doc, QTextCursor &c);
// Takes what the letters say of their own (font, size, bold, color, and the rest, not the language the file states) off, so that the paragraph's style and the text's character style say them
// (one step); the character style stays.
void clearDirectCharacterFormatting(WriteDoc &doc, QTextCursor &c);

} // namespace jp::write
