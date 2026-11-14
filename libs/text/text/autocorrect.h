// Text rules for AutoCorrect and AutoFormat as you type, shared by the programs: they work on a paragraph's text and say what to put in place of what; the
// program makes the edit (so each has its own undo).
#pragma once

#include <QChar>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <optional>

namespace jp::autotext {

// A change to make to the paragraph's text: [from, to) is replaced by `with`; or, with `superscript`, the letters of [from, to) are raised and nothing is replaced.
struct Fix {
    int from = 0, to = 0;
    QString with;
    bool superscript = false;
};

// What AutoCorrect does, and what it does it with: the replacement list and the two exception lists are the person's (kept in the settings, one list for each language).
struct Config {
    QString language;               // a tag such as "en-US" or "fr-CA"; empty is American English
    bool replace = true;            // replace text as you type
    bool sentences = true;          // capitalize the first letter of sentences
    bool cells = true;              // capitalize the first letter of table cells
    bool days = true;               // capitalize names of days
    bool twoInitials = true;        // correct TWo INitial CApitals
    bool capsLock = true;           // correct accidental usage of cAPS LOCK
    QList<QPair<QString, QString>> replacements;   // what is typed, and what it becomes
    QStringList exceptions;         // abbreviations after which a lowercase word is no new sentence ("e.g.")
    QStringList initialExceptions;  // words that keep their two initial capitals ("IDs")
};
// The language's first part ("fr" for "fr-CA"); "en" for none.
QString baseLanguage(const QString &language);
// The lists that come with the program, for a language (symbols for every language; the typing slips for English).
QList<QPair<QString, QString>> defaultReplacements(const QString &language);
QStringList defaultExceptions(const QString &language);
QStringList defaultInitialExceptions(const QString &language);
Config defaultConfig(const QString &language = QString());

// AutoCorrect: the word that ends at `rel` in `text` (a typing slip, a symbol such as (c), two initial capitals, a day's name, or the capital at the start of a sentence).
// `inCell`: the paragraph is in a table's cell.
std::optional<Fix> correctWord(const QString &text, int rel, bool smartQuotes);
std::optional<Fix> correctWord(const QString &text, int rel, bool smartQuotes, const Config &config, bool inCell = false);
// AutoFormat: "word--word" (an em dash), "word - word" (an en dash), 1/2 1/4 3/4, and 1st 2nd 3rd 4th, at the end of `before` (the paragraph's text up to where a word ends).
std::optional<Fix> formatWord(const QString &before);
// "\"" or "'" typed after `before` (a null character at the start of the paragraph): the curly quote it becomes.
QString smartQuote(const QString &typed, QChar before);
// The same in a language: English " " and ' ', German „ " and ‚ ', French « » with a no-break space inside.
QString smartQuote(const QString &typed, QChar before, const QString &language);
// "*" or "-" (a bulleted list), or "1.", "1)", or "a)" (a numbered one, from the number), when that is all of `before`, the paragraph's text up to a space that was typed.
struct ListStart {
    bool numbered = false;
    bool paren = false;
    int number = 1;
    bool letter = false;   // "a)": the number is the letter's place in the alphabet
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
