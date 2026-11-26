#pragma once
// A copy of a note for a box that only shows the words (the slide show's Speaker Notes box, and Presenter View's notes where the show has no window's editor): formatted as the note
// is (bold, italic, underline, lists, links), at most `limit` characters of it, and sized as the box says. The note itself is never changed.

#include <QTextDocument>
#include <memory>

namespace jp {

// `limit` below 0: all of it. The cut is between whole characters (never inside a pair of UTF-16 units or between a letter and its marks); `cut` says whether there was one. The words
// the story sets at its own default size are shown at `points`, and every other size in the same proportion.
std::unique_ptr<QTextDocument> notesDisplayCopy(const QTextDocument *story, int limit, double points, bool *cut = nullptr);

} // namespace jp
