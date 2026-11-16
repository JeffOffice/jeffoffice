#include "core/picturecrop.h"

#include <algorithm>
#include <cmath>

namespace jp {

namespace {

bool usable(const QSizeF &s) { return std::isfinite(s.width()) && std::isfinite(s.height()) && s.width() > 0 && s.height() > 0; }

}

CropPlan planCropAspect(const QSizeF &frame, const CropShares &now, double ratioWidth, double ratioHeight)
{
    CropPlan plan;
    if (!usable(frame) || !std::isfinite(ratioWidth) || !std::isfinite(ratioHeight) || ratioWidth <= 0 || ratioHeight <= 0) return plan;
    const double showW = 1.0 - now.left - now.right, showH = 1.0 - now.top - now.bottom;
    if (!std::isfinite(showW) || !std::isfinite(showH) || showW < kLeastPictureShown || showH < kLeastPictureShown) return plan;
    const double aspect = frame.width() / frame.height(), target = ratioWidth / ratioHeight;
    plan.shares = now;
    plan.frame = frame;
    if (aspect > target) {
        const double f = target / aspect;   // the share of the width that stays
        const double cut = showW * (1.0 - f) / 2.0;
        plan.shares.left += cut;
        plan.shares.right += cut;
        plan.frame.setWidth(frame.width() * f);
    } else if (aspect < target) {
        const double f = aspect / target;
        const double cut = showH * (1.0 - f) / 2.0;
        plan.shares.top += cut;
        plan.shares.bottom += cut;
        plan.frame.setHeight(frame.height() * f);
    }
    plan.valid = 1.0 - plan.shares.left - plan.shares.right >= kLeastPictureShown && 1.0 - plan.shares.top - plan.shares.bottom >= kLeastPictureShown;
    return plan;
}

CropPlan planCropFill(const QSizeF &frame, const QSizeF &picture)
{
    CropPlan plan;
    if (!usable(frame) || !usable(picture)) return plan;
    const double a = picture.width() / picture.height(), f = frame.width() / frame.height();
    plan.frame = frame;
    if (a > f) plan.shares.left = plan.shares.right = (1.0 - f / a) / 2.0;
    else plan.shares.top = plan.shares.bottom = (1.0 - a / f) / 2.0;
    plan.valid = 1.0 - plan.shares.left - plan.shares.right >= kLeastPictureShown && 1.0 - plan.shares.top - plan.shares.bottom >= kLeastPictureShown;
    return plan;
}

CropPlan planCropFit(const QSizeF &frame, const QSizeF &picture, bool room)
{
    CropPlan plan;
    if (!usable(frame) || !usable(picture)) return plan;
    const double a = picture.width() / picture.height(), f = frame.width() / frame.height();
    plan.frame = frame;
    // How much more than the picture the frame shows, across or down.
    const double more = a > f ? a / f : f / a;
    if (room && (more - 1.0) / 2.0 <= kMostRoomShare) {
        const double share = -(more - 1.0) / 2.0;
        if (a > f) plan.shares.top = plan.shares.bottom = share;
        else plan.shares.left = plan.shares.right = share;
    } else if (a > f) {
        plan.frame.setHeight(frame.width() / a);
    } else {
        plan.frame.setWidth(frame.height() * a);
    }
    plan.valid = true;
    return plan;
}

} // namespace jp
