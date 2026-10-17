#pragma once
// What the Format pane does to the objects, as plain functions on one object's fill, line, or frame: the pane and an app's Shape Fill, Shape
// Outline, and Shape Effects menus both call them, so both reach the same settings.

#include "core/items.h"
#include "core/texteffects.h"

#include <QTextCharFormat>
#include <functional>

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

// ---- the letters of text (Text Fill, Text Outline, and Text Effects): what each sets on the format of a run
// A solid color (a gradient the letters had goes), none (clear letters), or a gradient of the presets above from the letters' own color.
void setTextFill(QTextCharFormat &cf, const ColorRef &color);
void setTextFillNone(QTextCharFormat &cf);
void setTextFillGradient(QTextCharFormat &cf, int preset, const ColorScheme &scheme);
// A gradient of any kind (its first color is the letters' own, for what can't show a gradient).
void setTextFillGradient(QTextCharFormat &cf, const Fill &gradient, const ColorScheme &scheme);
// The color of the letters, as they show: the color they name, else the brush they were given, else `fallback`.
ColorRef textColorOf(const QTextCharFormat &cf, const ColorRef &fallback);
// An outline in a color (a point wide where the letters have none), a weight (in the color the letters have, else the theme's main color), none.
void setTextOutline(QTextCharFormat &cf, const ColorRef &color);
void setTextOutlineWeight(QTextCharFormat &cf, double points);
void setTextOutlineNone(QTextCharFormat &cf);
// The effects a run has, changed by `change` and put back (an empty list where none is left, which says the style's shadows are off too).
void changeTextEffects(QTextCharFormat &cf, const std::function<void(TextEffects &)> &change);
// The same on a Text Art style's look: its fill, outline, and shadow (the shadow is the first the letters have; the others stay).
void setTextLook(QTextCharFormat &cf, const Fill &fill, const Stroke &stroke, const ShadowFx &shadow, const ColorScheme &scheme);

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
