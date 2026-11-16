#pragma once
// What Crop's commands do to a picture that a file keeps as the shares of the whole picture cut off each side (DrawingML's srcRect), for the apps whose pictures are not
// page items: Aspect Ratio, Fill, and Fit, worked out once here from the frame and the picture's own size. (The page items' own crops are in app/pictureedit.h.) A negative
// share leaves room round the picture. Sizes are in any one unit.

#include <QSizeF>

namespace jp {

struct CropShares {
    double left = 0, top = 0, right = 0, bottom = 0;
};

// The crop a command makes and the frame that goes with it; `valid` is false when the sizes can't be used or nothing is left of the picture.
struct CropPlan {
    bool valid = false;
    CropShares shares;
    QSizeF frame;
};

// The most of the picture that may be cut off (the pair of a side keeps at least this much), and the most room round it a share may leave.
constexpr double kLeastPictureShown = 0.02;
constexpr double kMostRoomShare = 1.0;

// Aspect Ratio: the part that shows is cut, from its middle, to the shape width : height, and the frame shrinks with it so that the picture keeps its scale.
CropPlan planCropAspect(const QSizeF &frame, const CropShares &now, double ratioWidth, double ratioHeight);
// Fill: the frame stays and the picture covers all of it, cut evenly on the two sides that reach past it.
CropPlan planCropFill(const QSizeF &frame, const QSizeF &picture);
// Fit: the whole picture shows inside the frame. With `room`, the frame stays and the picture has room round it on two sides (a negative share; more room than the picture's own size
// is not made); without, or when that is too much, the frame shrinks to the picture's shape and no room is left.
CropPlan planCropFit(const QSizeF &frame, const QSizeF &picture, bool room);

} // namespace jp
