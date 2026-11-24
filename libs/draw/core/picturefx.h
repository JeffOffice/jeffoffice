#pragma once
// The picture filters: the effects that read a pixel's neighbors (sharpen and soften, color temperature, and the artistic effects). They are kept beside the
// picture as blip effects (core/blipeffect.h), never in its bytes, and worked out in whole numbers only, so a picture comes out the same on every system. A picture
// of more pixels than kPictureFxWorkPixels is made smaller first (by whole blocks, averaged), and the filtered copy is the one drawn and the one a file gets.

#include "core/blipeffect.h"

#include <QImage>
#include <QString>
#include <QVector>
#include <atomic>

namespace jp {

// The pixels a filter works on (a larger picture is averaged down to at most this many before the first filter).
constexpr qint64 kPictureFxWorkPixels = 4'000'000;
// What a pixel takes of one filter (as kBlipPixelCost: nanoseconds on a busy machine), and the work above which the apps show a busy box.
constexpr int kBlipFilterCost = 60;
constexpr qint64 kPictureFxBusyWork = 150LL * 1000 * 1000;

// The artistic effects, as BlipEffect::style holds them.
enum class ArtisticStyle { Blur, PencilGrayscale, GlowEdges, Mosaic, Last = Mosaic };

// Whether the effect is one of the filters (Sharpen, Temperature, Artistic).
bool isBlipFilter(const BlipEffect &e);
// The filters of `effects` in order, and the rest in order.
QVector<BlipEffect> blipFilters(const QVector<BlipEffect> &effects);
QVector<BlipEffect> blipColorEffects(const QVector<BlipEffect> &effects);
// What a filter does, in words for the person (an effect that is not a filter: empty).
QString blipFilterName(const BlipEffect &e);

// `image` averaged down by whole blocks to at most kPictureFxWorkPixels (as it is when it is not larger).
QImage pictureFxWorkCopy(const QImage &image);
// `image` (32 bits) with the filter applied. With `stop` set to true the work ends early and what comes back is not finished.
QImage applyBlipFilter(const QImage &image, const BlipEffect &e, const std::atomic<bool> *stop = nullptr);

} // namespace jp
