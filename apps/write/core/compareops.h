#pragma once
// Review > Compare: two versions of a document made into one, a copy of the first with the changes that make it the second as tracked changes.

#include "core/writedoc.h"
#include "review/textdiff.h"

#include <functional>
#include <memory>

namespace jp::write {

struct CompareOptions : textdiff::DiffOptions {   // (ignoreCase, ignoreWhiteSpace, byCharacter: the words are told apart as review/textdiff.h says)
    QString author;                 // who the changes are signed with
    bool detectMoves = true;        // a paragraph that is only moved, with its words the same or nearly, is a move and not a deletion and an insertion
    bool compareFormatting = true;  // text that is the same in both, in a look or a paragraph setting that differs, is a change of formatting
    int minMoveWords = 12;          // the fewest words of a paragraph that is looked for among the moved
};
using textdiff::TextEdit;
using textdiff::diffText;

// The most work a compare does, so that no document makes it run long or hold much: every cap says "too many changes" in the note of the result when it was reached.
struct CompareLimits {
    int moveParagraphs = 2000;                 // the most paragraphs of one text that are looked at for moves (more: no move is looked for)
    qint64 moveBytes = 8 * 1024 * 1024;        // the most characters of the paragraphs that are looked at for moves
    qint64 moveWork = 20000000;                // the most words that are looked at to tell how like two paragraphs are
    qint64 moveCells = 100000000;              // the most cells of the tables that tell whether the words of two like paragraphs are in the same order (more: no more moves are looked for)
    qint64 diffCells = 2000000000;             // the most cells of the tables of lengths of all the paragraphs told apart word by word (more: a paragraph is one replacement); each paragraph is held to its own cap besides, so this is a bound of time
    int stories = 500;                         // the most notes, headers, and footers (besides the text) that are told apart; the rest are left as they are, with a note
    int boxes = 1000;                          // the most text boxes of a document that are read to count the differences in them (more: the rest are not read, with a note)
    int drawings = 20000;                      // the most pictures and shapes of a document that are looked at to find its text boxes (more: the rest are not looked at, with a note)
    qint64 alignBlocks = 100000;               // the most paragraphs and tables, of all the stories and both documents, that are lined up to tell which table is which (more: no tables are paired, and they are counted)
    int formatChanges = 20000;                 // the most changes of formatting that are made (more: the rest are left out)
    qint64 formatFragments = 500000;           // the most pieces of text of one look that are held to compare looks (more: the looks of the rest are not compared)
};
// What a compare did, for the tests of its cost: the pairs of paragraphs looked at for moves (with their words), the cells of tables, and the moves found.
struct CompareWork {
    // The kinds of difference that are counted and not marked, in the order the note names them.
    enum Unmarked { Tables, Footnotes, Endnotes, TextBoxes, HeadersFooters, KeptAsWritten, UnmarkedKinds };
    qint64 stories = 0, notMarked = 0, moveComparisons = 0, moveWords = 0, moveCells = 0, diffCells = 0, moves = 0, formatChanges = 0, formatFragments = 0, alignBlocks = 0, drawings = 0, boxesRead = 0;
    qint64 unmarked[UnmarkedKinds] = {};   // (notMarked is the sum of these)
    void countUnmarked(Unmarked kind, qint64 n = 1)
    {
        unmarked[kind] += n;
        notMarked += n;
    }
};
// The limits and the count of work of one compare (made for each, so that none is shared).
struct CompareContext {
    CompareLimits limits;
    CompareWork work;
};

// What a compare covers: the text, the footnotes (lined up by their words, so a note put in or taken out shifts none of the rest), and the headers and footers (by section, kind, and slot); tables are paired by
// lining up the paragraphs and tables of a story. What it finds in a place it can't mark yet (a table with no partner of the same shape, a note put in or taken out or whose mark is in another place, a header or
// footer only one has, an endnote, a text box) is counted by kind, and the note says how many: nothing that differs is left out without being told. (`revised` may have its notes and headers made ready to read; its words are not changed.)
// A copy of `original` with the changes that make it `revised` as tracked changes by `options.author`: paragraphs of the text that are only in one, and the words that differ in the paragraphs
// that are in both. Moves are looked for in the text only; paragraphs of it that were only moved are moves, and text that is the same but looks or is set differently is a change of formatting. `changes` gets how many changes were made, and `note` (when it is given) what the compare left out for the limits it has (empty when nothing).
// Inserted paragraphs have the revised paragraph's style, list, alignment, indents, and the look of its letters; the result has no protection of the original's. `progress` is told how many of the
// differing places are done (of the total), and returns false to cancel (the result is then null, with an empty `error`). Null, with `error`, when the original cannot be copied.
std::unique_ptr<WriteDoc> compareDocuments(WriteDoc &original, WriteDoc &revised, const CompareOptions &options, int *changes = nullptr, QString *error = nullptr,
                                           const std::function<bool(int done, int total)> &progress = {}, QString *note = nullptr);
// The same with the limits of `context` (the tests set them), and the work done put in it.
std::unique_ptr<WriteDoc> compareDocumentsWith(CompareContext &context, WriteDoc &original, WriteDoc &revised, const CompareOptions &options, int *changes = nullptr, QString *error = nullptr,
                                               const std::function<bool(int done, int total)> &progress = {}, QString *note = nullptr);

} // namespace jp::write
