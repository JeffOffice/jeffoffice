#pragma once
// A diagram (SmartArt) in a presentation file. Its frame (a graphic frame
// whose graphicData is the diagram namespace) names four parts of the file by
// the slide's links: the data, the layout, the quick style, and the colors
// (r:dm, r:lo, r:qs, r:cs). Beside them the presentation program keeps the
// diagram as it drew it, in a drawing part the slide links to as well (the
// data part names that link, in dsp:dataModelExt's relId): shapes like a
// slide's, in the space of the frame, each with the rectangle its text is set
// in (dsp:txXfrm). Showing the diagram is showing that drawing.

#include "core/document.h"
#include "office/drawingml.h"

#include <QByteArray>
#include <QDomElement>
#include <QSizeF>
#include <QString>

namespace jp::deck {

// The namespace of the drawing's elements (dsp:), and of the frame's (dgm:).
inline constexpr QLatin1String kDiagramDrawingNs("http://schemas.microsoft.com/office/drawing/2008/diagram");
inline constexpr QLatin1String kDiagramNs("http://schemas.openxmlformats.org/drawingml/2006/diagram");

struct DiagramLinks {
    QString data, layout, style, colors;   // the ids of the slide's links; empty for one the frame doesn't name
    bool complete() const { return !data.isEmpty() && !layout.isEmpty() && !style.isEmpty() && !colors.isEmpty(); }
};

// Whether a graphic frame holds a diagram, and the links it names.
bool isDiagramFrame(const QDomElement &graphicFrame);
DiagramLinks diagramFrameLinks(const QDomElement &graphicFrame);

// The id of the slide's link to the drawing, as the data part names it; empty if it names none. And the data part naming another link.
QString diagramDrawingLinkId(const QByteArray &dataXml);
QByteArray withDiagramDrawingLinkId(const QByteArray &dataXml, const QString &id);

// The drawing as an SVG the size of its frame (points): the shapes of `spTree`
// (a dsp:spTree) read with `ctx`, whose part is the drawing's, and laid out and
// painted as a slide's are, the letters as outlines. Pictures in it are kept at
// most as fine as a screen across the whole slide shows. `like` is the
// presentation it is in, which says how text is set. `imagesLeftOut` is raised by
// the pictures the document would not keep.
QByteArray diagramSvg(const QDomElement &spTree, const office::DrawingContext &ctx, const office::TextStyleStack &styles, const QSizeF &size, const Document &like,
                      int *imagesLeftOut = nullptr);

} // namespace jp::deck
