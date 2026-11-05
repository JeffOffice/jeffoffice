#pragma once
// Commands of the Home tab that work on text as text: Change Case and Sort. Each is one step of the story's history (Change Case is a tracked change when Track Changes is on).

#include "core/writedoc.h"

#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QVector>

namespace jp::write {

// ---- Change Case
enum class CaseMode { Sentence, Lower, Upper, Capitalize, Toggle };
// The text `text` as `mode` writes it. `startsSentence` says whether the text begins a sentence (for Sentence), and `startsWord` whether it begins a word (for Capitalize).
QString caseOf(const QString &text, CaseMode mode, bool startsSentence = true, bool startsWord = true);
// Gives the selected text (the word the caret is in, with none selected) the case `mode` says; how many letters changed. The look of each letter stays.
int changeCase(WriteDoc &doc, QTextCursor &c, CaseMode mode);
// The case Shift+F3 gives next to the selection: capitalize each word, then UPPERCASE, then lowercase.
CaseMode nextCaseInCycle(const QString &selected);

// ---- Sort
struct SortKey {
    enum Type { Text, Number, Date };
    int field = 1;              // 1 is the whole paragraph, or its first field when the paragraphs are split
    Type type = Text;
    bool descending = false;
};
struct SortOptions {
    QVector<SortKey> keys;      // up to three
    QChar separator;            // fields are split at this (a tab, a comma...); null: the whole paragraph is the one field
    bool header = false;        // the first paragraph stays where it is
    bool caseSensitive = false;
};
constexpr int kMostSortParagraphs = 5000;   // a sort of more paragraphs is refused (each moves as a piece of the story)
// The text of the paragraph that the key sorts by, as the key reads it.
QString sortFieldOf(const QString &paragraph, const SortKey &key, QChar separator);
// The paragraphs the selection touches sorted by the keys, as one step: the text moves, and each paragraph's place keeps its own look (style, list, indents). How many paragraphs are sorted
// (the selection's, less a header); 0 and nothing done when there are fewer than two, when one is kept as XML, or when Track Changes is on (a sort is not recorded as a change).
int sortParagraphs(WriteDoc &doc, QTextCursor &c, const SortOptions &options);

// ---- Paste, merging the formatting
// The fragment pasted as text in the look of the place it goes to; only its emphasis (bold, italic, underline, strikethrough) stays. One step.
void pasteMerged(WriteDoc &doc, QTextCursor &c, const QTextDocumentFragment &fragment);

// ---- Horizontal line
// A horizontal line (a paragraph of its own with a border under it) after the paragraph the caret is in, as one step; the caret goes to a plain paragraph below it. False where no paragraph can be
// made (in a paragraph kept as XML).
bool insertHorizontalLine(WriteDoc &doc, QTextCursor &c);

// ---- Line numbers
// Makes the paragraph format say, by its own element, that the paragraph takes no line number (or does, when `on` is false and its style says it does not): the layout's property and the
// kept `w:suppressLineNumbers` (the one a file had is replaced).
void setSuppressLineNumbers(QTextBlockFormat &bf, bool on);

// ---- Reveal Formatting
// What is set where the caret is, as pairs of a name and its value: the styles, the font, the look of the letters, and the paragraph's alignment, indents, and spacing (Style Inspector, Shift+F1).
QVector<QPair<QString, QString>> describeFormatting(WriteDoc &doc, const QTextCursor &c);

} // namespace jp::write
