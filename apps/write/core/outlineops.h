#pragma once
// Outline view's work on the document: the paragraphs with their outline levels, promoting and demoting, and moving a heading with what is under it. Each change is one step of the history.

#include "core/writedoc.h"

#include <QString>
#include <QVector>

namespace jp::write {

constexpr int kBodyLevel = 9;          // the outline level of body text; headings are 0 to 8 (Heading 1 to Heading 9)
constexpr int kMostOutlineRows = 200000;

struct OutlineRow {
    int position = 0;                  // where the paragraph begins in the story
    int level = kBodyLevel;
    QString text;                      // the paragraph's words (a line break shows as a space)
    bool editable = true;              // false where the paragraph holds a field, picture, or other piece the words alone do not stand for
    bool heading() const { return level < kBodyLevel; }
};
// Every paragraph of the text in order (at most kMostOutlineRows).
QVector<OutlineRow> outlineRows(WriteDoc &doc);
// The rows a heading has under it: the ones after row `i` up to the next heading of its level or a higher one (zero for body text).
int outlineSubtreeEnd(const QVector<OutlineRow> &rows, int i);

// Promotes (by < 0) or demotes (by > 0) the paragraphs from `first` to `last` (positions in them) by `by` levels. A heading is moved with the headings under it when `withSubtree`.
// Body text promoted becomes a heading of the level of the heading above it (Heading 1 with none); Heading 9 demoted becomes body text; Heading 1 promoted stays. True if anything changed.
bool outlineShift(WriteDoc &doc, int first, int last, int by, bool withSubtree);
// Puts new words in the paragraph at the position, in the look its first letter has (a line break is a space); one step that undoes. False where the paragraph is not editable.
bool outlineSetText(WriteDoc &doc, int position, const QString &text);
// A new empty paragraph after the one at the position, with the same outline level; its place is put in `newAt`. One step that undoes.
bool outlineInsertAfter(WriteDoc &doc, int position, int *newAt = nullptr);
// Makes the paragraphs a heading of `level` (0 to 8), or body text for kBodyLevel. True if anything changed.
bool outlineSetLevel(WriteDoc &doc, int first, int last, int level);
// Moves the paragraph at `position` (with the headings and text under it, if it is a heading and `withSubtree`) before the paragraph above it, or after the one below. False where it can't move.
// Where the moved paragraph begins afterward is put in `movedTo`.
bool outlineMove(WriteDoc &doc, int position, bool up, bool withSubtree, int *movedTo = nullptr);

// ---- collapsed headings: a view of the text on the screen (nothing is written, printed, or saved)
// The level (0 to 8) of the heading whose paragraph holds the position, -1 for body text.
int headingLevelAt(WriteDoc &doc, int position);
// Whether the heading at the position is collapsed, and whether a collapsed heading hides the paragraph at the position.
bool headingCollapsed(WriteDoc &doc, int position);
bool hiddenByCollapse(WriteDoc &doc, int position);
// Collapses (or opens) the heading whose paragraph holds the position: the paragraphs after it, up to the next heading of its level or a higher one, are left out of the lines. False where there is no
// such heading, or nothing under it to hide (when collapsing).
bool setHeadingCollapsed(WriteDoc &doc, int position, bool collapsed);
// Opens the collapsed headings that hide the position; true if one was opened.
bool expandAround(WriteDoc &doc, int position);
void expandAllHeadings(WriteDoc &doc);
// Works out again which paragraphs the collapsed headings hide, after the text changed (a heading that is no heading now is opened).
void refreshCollapsed(WriteDoc &doc);

} // namespace jp::write
