#pragma once
// A diagram in a file. Its frame (a graphic frame whose graphicData is the diagram namespace) names four parts of the file by the links of the part
// it is in: the data, the layout, the quick style, and the colors (r:dm, r:lo, r:qs, r:cs). Beside them the program that made the diagram keeps it as it drew it, in a drawing
// part the same part links to as well (the data part names that link, in dsp:dataModelExt's relId): shapes like a slide's, in the space of the frame, each with the rectangle
// its text is set in (dsp:txXfrm). Showing the diagram is showing that drawing (libs/page/render/diagramdrawing.h draws it).

#include "office/package.h"

#include <QByteArray>
#include <QDomElement>
#include <QString>
#include <QVector>

#include <functional>

namespace jp::office {

// The namespace of the drawing's elements (dsp:), and of the frame's (dgm:).
inline constexpr QLatin1String kDiagramDrawingNs("http://schemas.microsoft.com/office/drawing/2008/diagram");
inline constexpr QLatin1String kDiagramNs("http://schemas.openxmlformats.org/drawingml/2006/diagram");

struct DiagramLinks {
    QString data, layout, style, colors;   // the ids of the links; empty for one the frame doesn't name
    bool complete() const { return !data.isEmpty() && !layout.isEmpty() && !style.isEmpty() && !colors.isEmpty(); }
};

// Whether a graphic frame (a presentation's p:graphicFrame, a sheet's xdr:graphicFrame) holds a diagram, and the links it names.
bool isDiagramFrame(const QDomElement &graphicFrame);
DiagramLinks diagramFrameLinks(const QDomElement &graphicFrame);

// The id of the link to the drawing, as the data part names it; empty if it names none. And the data part naming another link.
QString diagramDrawingLinkId(const QByteArray &dataXml);
QByteArray withDiagramDrawingLinkId(const QByteArray &dataXml, const QString &id);

// The most bytes of a data part that are looked through for the drawing's link, and of a drawing part that is read.
constexpr qint64 kMostDiagramDataBytes = qint64(64) << 20;
constexpr qint64 kMostDiagramDrawingBytes = qint64(64) << 20;

// The drawing part of a diagram, by the links of the part its frame is in (`links`): the data part the frame names with `dataId` (`partData` gives a part's bytes, empty for
// none), the link that part names, and the part that link leads to, if all of that is there and is a drawing. Empty if any is missing, or the link is not one to a drawing (a data
// part that names its own link, say).
QString diagramDrawingPartOf(const QVector<Relationship> &links, const std::function<QByteArray(const QString &part)> &partData, const QString &dataId);
QString diagramDrawingPartOf(const Package &package, const QString &part, const QString &dataId);

} // namespace jp::office
