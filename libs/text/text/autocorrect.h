// Text rules for AutoCorrect and AutoFormat as you type, shared by the programs: they work on a paragraph's text and say what to put in place of what; the
// program makes the edit (so each has its own undo).
#pragma once

#include <QChar>
#include <QString>
#include <optional>

namespace jp::autotext {

// A change to make to the paragraph's text: [from, to) is replaced by `with`; or, with `superscript`, the letters of [from, to) are raised and nothing is replaced.
struct Fix {
    int from = 0, to = 0;
    QString with;
    bool superscript = false;
};

// AutoCorrect: the word that ends at `rel` in `text` (a typing slip, a symbol such as (c), two initial capitals, or the capital at the start of a sentence).
std::optional<Fix> correctWord(const QString &text, int rel, bool smartQuotes);
// AutoFormat: "word--word" (an em dash), "word - word" (an en dash), 1/2 1/4 3/4, and 1st 2nd 3rd 4th, at the end of `before` (the paragraph's text up to where a word ends).
std::optional<Fix> formatWord(const QString &before);
// "\"" or "'" typed after `before` (a null character at the start of the paragraph): the curly quote it becomes.
QString smartQuote(const QString &typed, QChar before);
// "*" or "-" (a bulleted list), or "1." or "1)" (a numbered one, from the number), when that is all of `before`, the paragraph's text up to a space that was typed.
struct ListStart {
    bool numbered = false;
    bool paren = false;
    int number = 1;
    int length = 0;   // how much of the paragraph the mark takes
};
std::optional<ListStart> listStart(const QString &before);

// A web or email address that ends `before` (the paragraph's text up to a space that was typed), less the marks that end a sentence: [from, to) is the address as typed and
// `with` the address a link goes to ("https://" or "mailto:" put on).
std::optional<Fix> linkWord(const QString &before);
// *word* or _word_ that ends `before`: [from, to) is the word with its two marks; `mark` is the mark. A mark inside a word (snake_case) is no mark.
struct Mark {
    int from = 0, to = 0;
    QChar mark;
};
std::optional<Mark> markWord(const QString &before);
// A paragraph that is three or more of one of - = _ and nothing else: the mark, for the line that Enter turns it into.
std::optional<QChar> ruleLine(const QString &paragraph);

} // namespace jp::autotext
