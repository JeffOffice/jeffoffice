#pragma once
// What a drawn object of a file (a VML or DrawingML shape, read as wml::Shape) becomes beyond a plain box, picture, or line: the cuts, clear color, and
// border of a picture, lettering on a path, a horizontal rule, and a shape of the drawing format's own kinds.

#include "core/items.h"
#include "wml/wml.h"

namespace jp::write {

// Sets the picture's place in its frame (the whole picture, outside the frame by what the shape cuts), its clear color, and its border from the shape. The
// picture's frame (rect) is set already.
void dressPicture(PictureItem &pic, const office::wml::Shape &s);
// The Text Art of a shape that is lettering on a path (its text set to fill its frame, in the shape's colors and shadow), or none for a shape that is not.
ItemPtr letteringItem(const office::wml::Shape &s);
// The shape of a horizontal rule set in a line of the width given, with the room beside it that sets it across the line as it is aligned, or none for a shape
// that is not a rule.
ItemPtr ruleItem(const office::wml::Shape &s, double textWidth);
// The preset a shape is drawn as: its own kind's (an arc, a bracket) where the drawing format numbers it, else a rectangle or an ellipse.
QString presetOf(const office::wml::Shape &s);

} // namespace jp::write
