#pragma once
// A diagram as the tests of the apps need one: the drawing part a file keeps beside the diagram's data, the data part that names it, and the three parts
// (layout, style, colors) the frame also names. Three shapes: a rectangle in the first accent color, a rounded one in blue, and a green one. Every byte is built here.

#include <QByteArray>
#include <QString>

namespace diagramtest {

inline QString dspNs() { return QStringLiteral("http://schemas.microsoft.com/office/drawing/2008/diagram"); }
inline QString dgmNs() { return QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/diagram"); }
inline QString drawingRelType() { return QStringLiteral("http://schemas.microsoft.com/office/2007/relationships/diagramDrawing"); }
inline QString relBase() { return QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/"); }

inline QString diagramShapeXml(qint64 x, qint64 y, qint64 cx, qint64 cy, const QString &geometry, const QString &fill, const QString &text)
{
    const QString place = QStringLiteral("<a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/>").arg(x).arg(y).arg(cx).arg(cy);
    return QStringLiteral("<dsp:sp modelId=\"{AAAAAAAA-0000-4000-8000-%1}\"><dsp:nvSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvSpPr/></dsp:nvSpPr>"
                          "<dsp:spPr><a:xfrm>%2</a:xfrm><a:prstGeom prst=\"%3\"><a:avLst/></a:prstGeom>%4<a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill></a:ln></dsp:spPr>"
                          "<dsp:txBody><a:bodyPr lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"ctr\"><a:noAutofit/></a:bodyPr><a:lstStyle/><a:p><a:pPr algn=\"ctr\"/>"
                          "<a:r><a:rPr lang=\"en-US\" sz=\"2400\"/><a:t>%5</a:t></a:r></a:p></dsp:txBody><dsp:txXfrm>%2</dsp:txXfrm></dsp:sp>")
        .arg(QStringLiteral("%1").arg(qHash(text) % 1000000000000ULL, 12, 10, QLatin1Char('0')), place, geometry, fill, text);
}

// The drawing part: its shapes fill 3,657,600 by 1,371,600 EMU (4 by 1.5 inches).
inline QByteArray diagramDrawingXml(const QString &extraShapes = QString())
{
    const QString head = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><dsp:drawing xmlns:dgm=\"%1\" xmlns:dsp=\"%2\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                                        "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><dsp:spTree><dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr/>")
                             .arg(dgmNs(), dspNs());
    const QString accent = QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill>");
    const QString blue = QStringLiteral("<a:solidFill><a:srgbClr val=\"336699\"/></a:solidFill>");
    const QString green = QStringLiteral("<a:solidFill><a:srgbClr val=\"20A040\"/></a:solidFill>");
    return (head + diagramShapeXml(0, 457200, 1066800, 914400, QStringLiteral("rect"), accent, QStringLiteral("First")) + diagramShapeXml(1295400, 457200, 1066800, 914400, QStringLiteral("roundRect"), blue, QStringLiteral("Second"))
            + diagramShapeXml(2590800, 457200, 1066800, 914400, QStringLiteral("rect"), green, QStringLiteral("Third")) + extraShapes + QStringLiteral("</dsp:spTree></dsp:drawing>"))
        .toUtf8();
}

// The data part, naming the drawing's link `drawingRelId` (empty: it names none, as an older file's does not).
inline QByteArray diagramDataXml(const QString &drawingRelId)
{
    const QString ext = drawingRelId.isEmpty() ? QString()
                                               : QStringLiteral("<dgm:extLst><a:ext uri=\"http://schemas.microsoft.com/office/drawing/2008/diagram\"><dsp:dataModelExt xmlns:dsp=\"%1\" relId=\"%2\" minVer=\"%3\"/></a:ext></dgm:extLst>")
                                                     .arg(dspNs(), drawingRelId, dgmNs());
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><dgm:dataModel xmlns:dgm=\"%1\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">"
                          "<dgm:ptLst><dgm:pt modelId=\"{1}\" type=\"doc\"><dgm:prSet loTypeId=\"urn:test:layout\"/><dgm:spPr/><dgm:t><a:bodyPr/><a:p><a:r><a:t>First</a:t></a:r></a:p></dgm:t></dgm:pt></dgm:ptLst>"
                          "<dgm:cxnLst/><dgm:bg/><dgm:whole/>%2</dgm:dataModel>")
        .arg(dgmNs(), ext)
        .toUtf8();
}

inline QByteArray diagramOtherXml(const char *root)
{
    return (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><dgm:%1 xmlns:dgm=\"%2\" uniqueId=\"urn:test:%1\"><dgm:title val=\"\"/><dgm:desc val=\"\"/></dgm:%1>").arg(QLatin1String(root), dgmNs())).toUtf8();
}

} // namespace diagramtest
