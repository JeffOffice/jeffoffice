#pragma once
// AutoCorrect and AutoFormat as you type: what a typed character or Enter turns into, by the text rules the programs share (text/autocorrect.h). Each fix is made after the
// character is in, as one step of the history of its own, so that one Undo takes back the fix and leaves what was typed.

#include "core/writedoc.h"
#include "text/autocorrect.h"

#include <QString>
#include <QTextCursor>

namespace jp::write {

// The last change AutoCorrect made to a typed word, for the options offered after it (undo it, stop making it, open the settings). `revision` is the story's revision just after the change.
struct AutoFix {
    bool valid = false;
    QString typed, fixed, language;   // the word as typed, what it became, and the language of the text (whose lists the change came from)
    autotext::Fix::Kind kind = autotext::Fix::Kind::None;
    QString entry;                    // for a replacement: the entry of the list that made it
    int from = 0, to = 0;             // where the new word is in the story
    int revision = -1;
};

struct AutoOptions {
    bool correct = true;   // typing slips, symbols, and capitals (AutoCorrect)
    bool quotes = true;    // straight quotes made curly
    bool format = true;    // dashes, fractions, ordinals, lists, *bold*, _underline_, and lines (AutoFormat)
    bool links = true;     // web and email addresses made links
    AutoFix *record = nullptr;   // where to note the AutoCorrect change of the word typed, if there is one (it is made empty first)
};

// `text` typed at the caret (or over the selection), with the fixes the options allow. A single character from the keyboard is the case; anything longer, or any text over a selection, is put in as it is.
void typeText(WriteDoc &doc, QTextCursor &c, const QString &text, const AutoOptions &options);

// Enter in a paragraph that is three or more of - = _ and nothing else: the paragraph's text is taken away and it gets a bottom border (single, double, or thick), and a plain paragraph
// follows, as one step. False, with nothing done, where Enter is only Enter.
bool enterAfterRule(WriteDoc &doc, QTextCursor &c, const AutoOptions &options);

} // namespace jp::write
