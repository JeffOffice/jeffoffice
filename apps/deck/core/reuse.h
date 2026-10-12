#pragma once
// Reusing a slide of another presentation: a copy of the slide in this one,
// with all that is its own (its objects and their text, pictures, movies and
// sounds, charts, notes, and the transition and animation it keeps), under
// new ids all through, so nothing in the two presentations shares one.
//
// Either the slide takes this presentation's look, or keeps its own:
// - without its formatting, it stands on this presentation's layout of the
//   same type (as the slide before it has one, else any), its placeholders
//   going to the layout's places by type and index, their text in the look of
//   the layout's (what the writer emphasized stays), the header and footer
//   as this presentation shows them, and no background of its own;
// - with it, its layout and slide master come along with their theme, named as
//   they were, once for each (one that looks the same as a layout or master
//   here, or that an earlier reuse brought, is used instead), and the theme
//   fonts its text names become the fonts they were in the other presentation.

#include "core/document.h"

#include <QString>

namespace jp::deck {

// Puts a copy of slide `slide` of `from` in `to` as slide `at` (past the last
// slide: the last place), in the section of the slide before it. Returns its
// index in `to`, or -1 when `from` has no such slide.
int insertSlideFrom(Document &to, int at, const Document &from, int slide, bool keepFormatting);

// A slide's title for a list: the first line of its title placeholder, else
// the title the page has; empty for none.
QString slideTitle(const Document &d, int slide);

} // namespace jp::deck
