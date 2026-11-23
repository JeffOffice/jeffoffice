#pragma once
// The Style Inspector's facts: the paragraph style and the character style where the caret is (or in the selection), and what the text says of its own on top of them (direct formatting), named
// for a person; and the commands that take that direct formatting off. One table of properties (styleinspect.cpp) is both what is listed and what is taken off, so they agree. Reveal Formatting
// shows the look itself (homeops.h); this is where it comes from.

#include "core/writedoc.h"

#include <QHash>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>

namespace jp::write {

struct AppliedFormatting {
    QString paragraphStyle, paragraphStyleName;    // the id and the name (the default paragraph style when the paragraph names none)
    QString characterStyle, characterStyleName;    // empty when the text has none
    QString tableStyleName;                        // the style of the table the paragraph is in, empty outside a table
    QStringList paragraphDirect, characterDirect;  // what the paragraph and the letters say besides their styles ("Alignment", "Bold", ...), empty when nothing
    bool mixedParagraphStyle = false;              // the selection has paragraphs of more than one style
    bool mixedCharacterStyle = false;              // ... letters of more than one character style (or some with one and some without)
    bool mixedLook = false;                        // ... letters that say different things of their own
};

// What applies at the start of the selection (or at the caret), and, for a selection, what its paragraphs and letters say together (a bounded look: a very long selection is read in part).
AppliedFormatting appliedFormattingAt(WriteDoc &doc, const QTextCursor &c);
// The name a person sees for the style of a paragraph (the default paragraph style's when it names none). `cache` keeps the names found by style id, for many paragraphs in a row.
QString paragraphStyleNameOf(WriteDoc &doc, const QTextBlock &block, QHash<QString, QString> *cache = nullptr);
// What the buttons act on: the selection, or, at a caret, the whole run of letters with the look the caret's letter has (the letter the pane describes). The cursor `c` itself is not moved.
QTextCursor inspectedRange(const QTextCursor &c);
// Takes the paragraphs' own look (the properties in the table) off, so that their style says them (one step); the style and what the file keeps beside the look stay.
void clearDirectParagraphFormatting(WriteDoc &doc, QTextCursor &c);
// Takes what the letters say of their own (the properties in the table, not the language the file states) off, so that the paragraph's style and the text's character style say them (one
// step); the character style stays.
void clearDirectCharacterFormatting(WriteDoc &doc, QTextCursor &c);
// Takes every character style the selection's letters have off them (one step).
void clearDirectCharacterStyles(WriteDoc &doc, QTextCursor &c);
// The names of the properties of the table, for the paragraph (true) or the letters (false), in the order they are listed.
QStringList inspectedPropertyLabels(bool paragraph);

} // namespace jp::write
