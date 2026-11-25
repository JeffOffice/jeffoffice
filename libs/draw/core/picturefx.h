#pragma once
// The picture filters: the effects that read a pixel's neighbors (sharpen and soften, color temperature, and the artistic effects). They are kept beside the
// picture as blip effects (core/blipeffect.h), never in its bytes, and worked out in whole numbers only, so a picture comes out the same on every system. A picture
// of more pixels than kPictureFxWorkPixels is made smaller first (by whole blocks, averaged), and the filtered copy is the one drawn and the one a file gets. The work is linear in the
// pixels, whatever the picture's shape, and every filter looks at a flag between rows so that it can be stopped.

#include "core/blipeffect.h"

#include <QImage>
#include <QString>
#include <QVector>
#include <atomic>

namespace jp {

// The pixels a filter works on (a larger picture is averaged down to at most this many before the first filter).
constexpr qint64 kPictureFxWorkPixels = 4'000'000;
// What a pixel of the work copy takes of each filter, in nanoseconds (measured on 4 million pixels, rounded up), the work above which the apps show a busy box (over half a second
// is expected), and the most filter work a picture is given: the filters one person can put on a picture (one of each kind, two of the dearest) come to well under it; a longer list,
// which only a file can hold, is worked through in order until it is used up, and the rest are left out (blipFiltersLeftOut says how many).
constexpr qint64 kPictureFxBusyWork = 500LL * 1000 * 1000;
constexpr qint64 kMaxBlipFilterWork = 1000LL * 1000 * 1000;

// The artistic effects, as BlipEffect::style holds them.
enum class ArtisticStyle { Blur, PencilGrayscale, GlowEdges, Mosaic, Last = Mosaic };

// Whether the effect is one of the filters (Sharpen, Temperature, Artistic).
bool isBlipFilter(const BlipEffect &e);
// The filters of `effects` in order, and the rest in order.
QVector<BlipEffect> blipFilters(const QVector<BlipEffect> &effects);
QVector<BlipEffect> blipColorEffects(const QVector<BlipEffect> &effects);
// Whether any of the filters is one people use to hide what is in a picture (Blur and Mosaic): such a picture is never drawn on a screen before they are done.
bool blipFiltersHide(const QVector<BlipEffect> &filters);
// What a filter does, in words for the person (an effect that is not a filter: empty).
QString blipFilterName(const BlipEffect &e);

// What a pixel takes of the filter (0 for an effect that is not one).
qint64 blipFilterCost(const BlipEffect &e);

// Where a filter counts the steps it takes (one for each pixel read or written, and each pixel of a window started), for tests.
struct FilterCount {
    qint64 steps = 0;
};

// The size `pictureFxWorkCopy` makes of a picture of `size` pixels.
QSize pictureFxWorkSize(const QSize &size);
// `image` averaged down by whole blocks to at most kPictureFxWorkPixels (as it is when it is not larger). With `stop` set to true the copy ends early and is not finished.
QImage pictureFxWorkCopy(const QImage &image, const std::atomic<bool> *stop = nullptr);
// A small picture of one color: the average of `image` (its clearness too), to stand in for a picture a filter that hides it (Blur, Mosaic) could not be worked on.
QImage pictureFxFlatPatch(const QImage &image);
// `image` (32 bits) with the filter applied. With `stop` set to true the work ends early (before the next row) and what comes back is not finished. `count` (if given) is added to.
QImage applyBlipFilter(const QImage &image, const BlipEffect &e, const std::atomic<bool> *stop = nullptr, FilterCount *count = nullptr);

} // namespace jp
