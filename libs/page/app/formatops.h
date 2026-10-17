#pragma once
// What the Format pane does to the objects, as plain functions on one object's fill, line, or frame: the pane and an app's Shape Fill, Shape
// Outline, and Shape Effects menus both call them, so both reach the same settings.

#include "core/items.h"

namespace jp {

class Editor;

namespace formatops {

// Whether an object has a fill of its own, and a line of its own. A line takes no fill; a table's are its cells'.
bool takesFill(const Item &it);
bool takesLine(const Item &it);

// ---- fills
// Solid in a color; the transparency the fill had stays.
void setSolid(Fill &f, const ColorRef &color);
// Gradient presets: from the color to a lighter one, running down, up, right, or left; across the corner; and out from the middle.
int gradientCount();
QString gradientName(int i);
void applyGradient(Fill &f, int i, const ColorScheme &scheme);

// ---- lines
// A color (the line is made solid, and a weight given if it had none).
void setOutlineColor(Stroke &s, const ColorRef &color);
void setOutlineNone(Stroke &s);
// Arrowhead styles for a line's ends: none, an arrow at the end, an arrow at both ends, an open arrow at the end, and a dot at the end.
int arrowStyleCount();
QString arrowStyleName(int i);
void applyArrowStyle(Stroke &s, int i);

// ---- geometry
// Whether an object is a shape that another geometry from the library can go onto (not a text box, a line, a picture, or artwork).
bool takesGeometry(const Item &it);
// Gives a shape a geometry of the library: its text, fill, line, effects, place, size, and turn stay; the values of its handles and any
// edited points do not. False (and nothing changes) for an object that takes none, or a shape the library doesn't have.
bool setGeometry(Item &it, const QString &shapeId);

// ---- size
// Whether a corner dragged, and a size typed, keep the object's proportions.
bool keepsProportions(const Item &it);
// Gives the frame a size (within what a slide can hold), its top left corner staying where it is; what is in it (a picture's crop, a table's
// columns, a group's objects) goes with it, and the group it is in is fitted to it.
void resizeFrame(Editor *ed, Item &it, QSizeF size);
// Moves the frame by a distance, and fits the group it is in.
void moveFrame(Editor *ed, Item &it, double dx, double dy);

} // namespace formatops
} // namespace jp
