#pragma once
// A copy of a note for a box that only shows the words (the slide show's Speaker Notes box, and Presenter View's notes where the show has no window's editor): formatted as the note
// is (bold, italic, underline, lists, links), at most `limit` characters of it, and sized as the box says. The note itself is never changed.

#include "core/color.h"

#include <QTextDocument>
#include <memory>

namespace jp {

// `limit` below 0: all of it. The cut is between whole characters (never inside a pair of UTF-16 units or between a letter and its marks); `cut` says whether there was one. The words
// the story sets at its own default size are shown at `points`, and every other size in the same proportion.
// A link is drawn as the printed page draws it: in the scheme's link color (`colors`; null: the standard scheme's), underlined, unless its words have a color of their own.
// A list item the story hangs (its first line starts left of the others) is laid out as the printed page lays it: all its lines start together, and its marker (drawn by ListMarkers) is
// as far left as the story's hang says; `pixelsPerPoint` (the view's resolution over 72) is what turns the indents, which are in points, into the pixels a text view lays them in
// (0: the lists are left as the story has them).
std::unique_ptr<QTextDocument> notesDisplayCopy(const QTextDocument *story, int limit, double points, bool *cut = nullptr, const ColorScheme *colors = nullptr, double pixelsPerPoint = 0);

} // namespace jp
