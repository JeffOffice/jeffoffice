#pragma once
// Design > Themes, Colors, and Fonts: the document's theme (the twelve colors, the heading and body fonts, and the effect styles of its theme part) changed as one step of the history. The runs and styles
// that take their color from a theme slot are recolored, so one change restyles the whole document (the body, the headers and footers, the notes, and the drawings); the text that follows the
// theme's fonts follows the new ones. Theme colors that a person set by hand to a plain color are left as they are.

#include "core/writedoc.h"
#include "office/themelib.h"

namespace jp::write {

// The theme part the document keeps, as it is (a theme with the part's own xml); an empty theme (no xml) where the document has none.
office::Theme documentTheme(const WriteDoc &doc);
// Whether the document has a theme part that can change (a file with none can't take one here).
bool canChangeTheme(const WriteDoc &doc);

// Each one step that Undo takes back; false (nothing changed) where the document has no theme, or the theme already is so.
bool applyThemeColors(WriteDoc &doc, const office::ThemeColors &colors);
bool applyThemeFonts(WriteDoc &doc, const office::ThemeFonts &fonts);
// The whole theme: its colors, fonts, and the part's effect styles (a theme file's, or the one the document had when it was opened, for Reset).
bool applyTheme(WriteDoc &doc, const office::Theme &theme);

} // namespace jp::write
