#pragma once
// Shrink by One Page: the work on the document (the window tries the steps and keeps them when a page comes off). Each step makes the effective size of all the text 4 percent smaller (to a
// tenth of a point; text of 6 points or less is left as it is), and the exact line spacing of the paragraphs the same share: the text of the body, its tables, and its footnotes, and (once,
// when the steps are known) the text boxes that are in it. Headers and footers are left alone. None of it is recorded as tracked changes.

#include "core/writedoc.h"

namespace jp::write {

constexpr double kShrinkLeastSize = 6.0;   // the text is never made smaller than this
constexpr double kShrinkShare = 0.96;      // what one step leaves of a size
constexpr int kMostShrinkPages = 300;      // a longer document is refused (the steps are tried on the document itself, a page count each)

// A size after one step.
double shrunkSize(double points);

// What the document has that decides whether a page can be taken off.
struct ShrinkLook {
    bool locked = false;       // a content control keeps its contents from being edited
    int forcedBreaks = 0;      // page breaks, paragraphs that start a page, and section breaks that start one: pages that smaller text can't bring together
    int steps = 0;             // the most steps before the smallest text that is over the least size is at it (0: there is none to make smaller)
};
ShrinkLook shrinkLook(WriteDoc &doc);

// One step on the text of the body and its footnotes, as part of the edit now being made, not tracked. False when nothing was changed.
bool shrinkStep(WriteDoc &doc);
// After the steps were taken back: the stories forget what could be done again (the document's own history is told by WriteDoc::discardRedo).
void forgetShrinkRedo(WriteDoc &doc);
// `steps` steps on every text box of the body, each one step of the history. Returns how many boxes changed.
int shrinkTextBoxes(WriteDoc &doc, int steps);

} // namespace jp::write
