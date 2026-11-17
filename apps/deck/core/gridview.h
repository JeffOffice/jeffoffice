#pragma once
// A presentation's grid and guides settings, kept with the document: its appData "grid" (an object with only the keys that were set: "spacing" in
// points, "snapGrid", "snapObjects", "showGuides") and Document::viewGuides (the guides, in points from the slide's top left). A .pptx keeps them in
// its view properties part (the grid spacing, the snap and show attributes, and the guide list); a file that has none leaves each one as the program's own.

#include "core/document.h"

#include <optional>

namespace jp::deck {

struct GridView {
    std::optional<double> spacing;
    std::optional<bool> snapGrid, snapObjects, showGuides;
    bool operator==(const GridView &o) const { return spacing == o.spacing && snapGrid == o.snapGrid && snapObjects == o.snapObjects && showGuides == o.showGuides; }
};

constexpr double kDefaultGridSpacing = 6;

GridView gridViewOf(const Document &d);
void setGridView(Document &d, const GridView &g);

// Makes the guides fit a slide of `size`: each position within a slide's width (height) before the slide to twice it (a guide may sit off the slide; one
// farther is put at that limit), none twice, none that is not a number. The count is not limited here; a file that is read is limited by readViewProperties.
void fitGuides(RulerGuides &g, const QSizeF &size);

} // namespace jp::deck
