#pragma once
// Replace Fonts: every font the document names outright, and swapping one for another everywhere it is named: the runs of the text (the body, the headers and footers, and the notes, with
// the words in tables), the fonts of East Asian and complex scripts that a run names, the styles (and the conditional parts of table styles), the style sheet's defaults, the numbers of lists,
// and the text of shapes and text boxes. The theme's own fonts change only when picked (as "+Heading" or "+Body"), and a slot a theme attribute overrides names no font. A
// run that follows the theme's font (Heading or Body) is left to the theme and is not listed. All of it is one step of the history.

#include "app/replacefonts.h"
#include "core/writedoc.h"

#include <QString>
#include <QVector>

namespace jp::write {

// The fonts the document names, most used first, with how many places of content name each (a run, a drawing's run); a font only styles, the style sheet's defaults, or the numbers of lists
// name is listed with no uses ("styles only"). The theme's heading and body fonts are separate entries ("+Heading", "+Body") when text or a style follows them.
QVector<FontUse> fontsUsed(WriteDoc &doc);
// Puts `to` in the place of `from` (without regard to case) everywhere it is named, as one step that Undo takes back; how many places changed (no step is made when none did). It looks at
// the document once: it does not scan for the font first.
int replaceFont(WriteDoc &doc, const QString &from, const QString &to);

} // namespace jp::write
