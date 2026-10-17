#pragma once
// Shape Format > Shape Styles: the Quick Styles gallery. Each style is a treatment (an outline alone, or a subtle, a moderate, or an intense
// fill, line, and shadow) in one of the theme's six accent colors, and is a reference to the theme's format styles (a StyleRef, what a file
// calls p:style): the look it gives comes from the theme of the slide master the slide stands on, so the same style looks as that theme makes
// it, and is saved as the reference with the properties that differ from it.

#include "core/deck.h"
#include "office/drawingml_styleref.h"

#include <memory>

namespace jp::deck {

struct ShapeStyle {
    QString id;      // "moderate:accent2"
    QString name;    // "Moderate Effect - Accent 2"
    QString group;   // "Moderate Effect": the row it is in
    StyleRef ref;
};

// The styles, a row for each treatment (outline only, subtle, moderate, intense) and a column for each of the theme's accents.
QVector<ShapeStyle> shapeStyles();
// The style with this id, or one that is off (ref.on false).
ShapeStyle shapeStyleById(const QString &id);

// The slide master whose theme the page being edited (or the master view that is open) stands on; null when there is none.
const MasterPage *themeMasterOf(const Document &d, int page, const QString &masterView);
// What that master's theme says its styles are, with the colors of its background (a dark slide master reads "tx1" as its light 1).
std::shared_ptr<office::StyleResolver> styleResolverOf(const Document &d, const MasterPage *master);
// The theme's accent colors (1 to 6) as that master shows them.
QColor accentOf(const Document &d, const MasterPage *master, int accent);

// Whether an object takes a style: a shape, a text box, or a line.
bool takesShapeStyle(const Item &it);
// Gives an object the look a style gives on this theme (its fill and line and shadow; a line keeps its arrowheads, and a line has no fill),
// and the style as its reference. Its text that has no color of its own, or the color the style it had gave, takes the new style's text color.
void applyShapeStyle(Document &d, Item &it, const StyleRef &ref, const office::StyleResolver &resolver);

} // namespace jp::deck
