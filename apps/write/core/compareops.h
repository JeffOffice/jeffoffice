#pragma once
// Review > Compare: two versions of a document made into one, a copy of the first with the changes that make it the second as tracked changes.

#include "core/writedoc.h"

#include <functional>
#include <memory>

namespace jp::write {

struct CompareOptions {
    QString author;                 // who the changes are signed with
    bool ignoreCase = false;        // a word that differs only in its capitals is the same
    bool ignoreWhiteSpace = false;  // runs of spaces and tabs are the same
    bool byCharacter = false;       // compare letter by letter, not word by word
};

// The words that are the same or differ between two paragraphs' texts, as the operations that turn `from` into `to`: a delete is a range of `from` (in its characters), an insert is text that
// goes after the range in `from` it is given at. Ranges and inserts are in order. Texts that are too long to compare word by word are one replacement.
struct TextEdit {
    int deleteFrom = 0, deleteTo = 0;   // a range of `from`; empty when nothing is deleted
    QString insertText;                 // put at deleteTo
};
QVector<TextEdit> diffText(const QString &from, const QString &to, const CompareOptions &options);

// A copy of `original` with the changes that make it `revised` as tracked changes by `options.author`: paragraphs of the text that are only in one, and the words that differ in the paragraphs
// that are in both. Only the text of the body is compared (tables, headers and footers, notes, and the looks of the text are not). `changes` gets how many changes were made.
// Inserted paragraphs have the revised paragraph's style, list, alignment, indents, and the look of its letters; the result has no protection of the original's. `progress` is told how many of the
// differing places are done (of the total), and returns false to cancel (the result is then null, with an empty `error`). Null, with `error`, when the original cannot be copied.
std::unique_ptr<WriteDoc> compareDocuments(WriteDoc &original, WriteDoc &revised, const CompareOptions &options, int *changes = nullptr, QString *error = nullptr,
                                           const std::function<bool(int done, int total)> &progress = {});

} // namespace jp::write
