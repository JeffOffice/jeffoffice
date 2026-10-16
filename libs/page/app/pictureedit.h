#pragma once
// Picture edits that need no window: what Crop's commands do to a picture's frame and to the picture inside it (PictureItem::imgRect is
// where the whole picture sits, in the frame's own coordinates: what lies outside the frame is the cropped part).

#include "core/document.h"

namespace jp {

// Clear Crop: the frame grows to show the whole picture, around the same middle.
void clearPictureCrop(PictureItem *p);

// Aspect Ratio: the frame is cut to the largest rectangle of that width to height inside it, around its middle; the picture stays where it
// is on the page.
void cropPictureToAspect(PictureItem *p, double width, double height);

} // namespace jp
