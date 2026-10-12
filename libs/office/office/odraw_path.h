#pragma once
// The outlines that shapes of binary drawings carry or imply: a freeform's
// vertices and segments, and the routes of bent and curved connectors.

#include "office/odraw_props.h"

#include <QPainterPath>
#include <QSizeF>

namespace jp::office::odraw {

// The path of a freeform, from the shape's own coordinate space (its geometry
// rectangle) scaled into a frame of `size`; empty when it has no usable one.
QPainterPath freeformPath(const Props &props, const QSizeF &size);
// The route of a bent or curved connector (shape numbers 33 to 40) in a frame of `size`.
QPainterPath connectorPath(int shapeType, const Props &props, const QSizeF &size);

} // namespace jp::office::odraw
