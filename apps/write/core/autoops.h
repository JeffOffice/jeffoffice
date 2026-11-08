#pragma once
// AutoCorrect and AutoFormat as you type: what a typed character or Enter turns into, by the text rules the programs share (text/autocorrect.h). Each fix is made after the
// character is in, as one step of the history of its own, so that one Undo takes back the fix and leaves what was typed.

#include "core/writedoc.h"

#include <QString>
#include <QTextCursor>

namespace jp::write {

struct AutoOptions {
    bool correct = true;   // typing slips, symbols, and capitals (AutoCorrect)
    bool quotes = true;    // straight quotes made curly
    bool format = true;    // dashes, fractions, ordinals, lists, *bold*, _underline_, and lines (AutoFormat)
    bool links = true;     // web and email addresses made links
};

// `text` typed at the caret (or over the selection), with the fixes the options allow. A single character from the keyboard is the case; anything longer, or any text over a selection, is put in as it is.
void typeText(WriteDoc &doc, QTextCursor &c, const QString &text, const AutoOptions &options);

// Enter in a paragraph that is three or more of - = _ and nothing else: the paragraph's text is taken away and it gets a bottom border (single, double, or thick), and a plain paragraph
// follows, as one step. False, with nothing done, where Enter is only Enter.
bool enterAfterRule(WriteDoc &doc, QTextCursor &c, const AutoOptions &options);

} // namespace jp::write
