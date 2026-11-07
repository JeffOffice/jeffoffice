#pragma once
// A diagram (SmartArt) drawn from the drawing a file keeps for it (see libs/office/office/diagram.h for how a frame, its data, and that drawing are linked): the shapes of the
// drawing, in the space of the diagram's frame, set and painted as a slide's are, the letters as outlines, and kept as an SVG (a picture that is printed as vectors and drawn
// on screen as fine as it is zoomed). JeffDeck, JeffWrite, and JeffCrunch all show a diagram through this.

#include "core/document.h"
#include "office/drawingml.h"
#include "office/package.h"
#include "office/theme.h"

#include <QByteArray>
#include <QDomElement>
#include <QSizeF>
#include <QString>

namespace jp {

// The drawing as an SVG the size of its frame (points): the shapes of `spTree`
// (a dsp:spTree) read with `ctx`, whose part is the drawing's, and laid out and
// painted as a slide's are, the letters as outlines. Pictures in it are kept at
// most as fine as a screen across the whole slide shows. `like` is the
// presentation it is in, which says how text is set. `imagesLeftOut` is raised by
// the pictures the document would not keep.
// `shown` (when it is not empty) is the size the picture is to be drawn at, if that is not the size the drawing was made for: the shapes are then scaled with it.
QByteArray diagramSvg(const QDomElement &spTree, const office::DrawingContext &ctx, const office::TextStyleStack &styles, const QSizeF &size, const Document &like,
                      int *imagesLeftOut = nullptr, const QSizeF &shown = QSizeF());

// The most shapes (groups and the shapes in them together) a drawing of a diagram that is not in a presentation may have drawn; the rest are left out.
constexpr int kMostDiagramShapes = 2000;

// The diagram a drawing part holds (`drawingXml`, the part named `drawingPart` in `package`, whose links name the pictures in it; none: its pictures are left out), drawn for a
// frame of size `natural` (the size it was made for, in points) and shown at `shown`. The look of the theme is the document's own. Empty if the part is no drawing, or is damaged,
// or has no shapes that are drawn. A drawing is read within the limits of office::parseXml and kMostDiagramShapes shapes, and groups nested no deeper than the shape reader allows.
QByteArray diagramDrawingSvg(const QByteArray &drawingXml, const QString &drawingPart, const office::Package *package, const office::Theme &theme, const QSizeF &natural,
                             const QSizeF &shown, int *imagesLeftOut = nullptr);

// The box that stands for a diagram whose file keeps no drawing (and no picture of it): a pale box with a dashed line round it that says what it is, in `size` points.
QByteArray diagramPlaceholderSvg(const QSizeF &size);


} // namespace jp
