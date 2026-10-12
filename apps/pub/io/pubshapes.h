#pragma once
// Preset shapes in .pub files: which of JeffPub's shapes Publisher has as a
// shape type of its own (core/shapetypes.h), and how Publisher draws pictures.

#include "core/shapetypes.h"

#include <QString>

namespace jp {

// Publisher's shape number for a preset, or -1 when Publisher has no such
// shape or draws it differently (the shape is then saved as a freeform
// outline).
inline int pubShapeType(const QString &preset) { return drawsAsShapeType(preset) ? drawingShapeType(preset) : -1; }

// The preset for one of Publisher's shape numbers, or an empty string.
inline QString presetForPubShapeType(int type)
{
    const QString preset = presetForDrawingShapeType(type);
    return drawsAsShapeType(preset) ? preset : QString();
}

// Publisher's Text Art shape number for a Text Art transform (plain text
// when unknown), and back (an empty string when the number isn't Text Art).
inline int pubTextArtType(const QString &transform) { return textArtShapeType(transform); }
inline QString textArtTransformForPubType(int type) { return textArtForShapeType(type); }

// .pub files have no sepia setting: a sepia picture is saved recolored to
// this brown, and a picture recolored to it reads back as sepia.
constexpr unsigned kPubSepia = 0x704214;   // RGB

// Publisher draws a picture used as a shape's fill (a page's background is
// one) this many points past the shape on every side, clipped to it; a
// picture in a picture frame of its own is drawn to the frame.
constexpr double kPubPictureFillBleed = 2.5;

} // namespace jp
