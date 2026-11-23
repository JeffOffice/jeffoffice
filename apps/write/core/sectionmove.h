#pragma once
// Moving a section of the text (a heading with everything under it, up to the next heading of its level or a higher one) to another place, as one step of the history.

#include "core/writedoc.h"

#include <QString>

namespace jp::write {

// The paragraphs of the section whose heading paragraph holds `position`: the first is the heading, the last is the one before the next heading of the same or a higher level (or the end
// of the text). False where the paragraph is no heading, or is a heading in a table, a frame, or the endnotes.
struct SectionRange {
    int first = 0, last = 0;   // a position in the first paragraph and in the last
    int level = 0;
    int end = 0;               // the position after the last paragraph, with its break
};
bool sectionAt(WriteDoc &doc, int position, SectionRange *out);
// The position of the heading (a paragraph sectionAt takes) at `position` or the nearest above it, -1 where there is none or `position` is not in the text of the document (the notes at its end are not).
int headingAtOrAbove(WriteDoc &doc, int position);

// Moves the section of the heading at `position` to before the paragraph at `before` (the end of the text when negative), keeping its level. See moveBlocks for what is refused; a
// destination in the section itself does nothing and says nothing. Where the heading is afterward goes in `movedTo`.
bool moveSection(WriteDoc &doc, int position, int before, QString *why = nullptr, int *movedTo = nullptr);
// Moves the section past its neighbor of the same level, up or down (the same level, under the same heading: a section can't leave the heading it is under this way). When it can't, `why`
// says why (also where there is no neighbor).
bool moveSectionBy(WriteDoc &doc, int position, bool up, QString *why = nullptr, int *movedTo = nullptr);

} // namespace jp::write
