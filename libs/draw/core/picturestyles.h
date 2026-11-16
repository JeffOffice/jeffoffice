#pragma once
// The Picture Styles gallery's looks, for every app: a crop shape, a border, and effects (shadow, reflection, glow, soft edges, bevel, and a turn in
// space), each made of what the drawing engine already draws. A style sets only those three things; the picture's size, place, crop, and colors stay.

#include "core/itemsfwd.h"

namespace jp {

// How many styles there are, a style's short name that does not change (for files and tests), and its name as shown (translated).
int pictureStyleCount();
QString pictureStyleId(int k);
QString pictureStyleName(int k);
// The style with this id, or -1.
int pictureStyleIndex(const QString &id);
// Gives the picture the look of style `k` (0 is plain: no border, no effects, a rectangle). False for a number that is no style.
bool applyPictureStyle(PictureItem *pic, int k);

// Reset Picture: takes off the picture's look (its shape, border, effects, and the changes made to its colors), as it came in. The crop is taken off too, so
// that the frame shows the whole picture around the same middle; with `andSize`, the frame is also made the size the picture has in the file (96 pixels
// to the inch of `px`), around the same middle. Its name, alt text, place, rotation, and link stay. False when nothing changes.
bool resetPictureLook(PictureItem *pic, const QSize &px, bool andSize);

// Change Picture: `pic` now shows the picture `imageId` (of pixel size `px`) in its place with its size, crop, shape, border, effects, and color changes kept. A
// picture of the same proportions keeps the crop as it was; one of other proportions fills the frame instead (the frame stays and the picture is cut to it).
void changePictureImage(PictureItem *pic, const QString &imageId, const QSize &px);

} // namespace jp
