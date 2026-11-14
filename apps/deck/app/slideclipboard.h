#pragma once
// Whole slides on the system clipboard: Copy and Cut in the slides pane and
// the Slide Sorter put the chosen slides there as a small presentation file
// under a type of our own (so a paste works in another JeffDeck window, with
// the pictures, movies, and sounds they hold), and Paste takes them back
// through Reuse Slides' copy (core/reuse.h), with or without their formatting.

#include "core/document.h"

#include <QList>

namespace jp::deck {

// Puts copies of slides `slides` (indexes in `d`, in the order they have) on the clipboard.
void copySlidesToClipboard(const Document &d, const QList<int> &slides);
bool slidesOnClipboard();
// Puts the slides on the clipboard in `d` as slides `at` on (past the last
// slide: the end), in their order, and returns how many. Without
// `keepFormatting` they take `d`'s layouts and theme.
int pasteSlidesFromClipboard(Document &d, int at, bool keepFormatting);

} // namespace jp::deck
