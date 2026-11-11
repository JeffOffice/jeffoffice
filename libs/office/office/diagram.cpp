#include "office/diagram.h"
#include "office/package.h"

#include "office/xml.h"

namespace jp::office {

namespace {

QDomElement aChild(const QDomElement &e, const char *name) { return child(e, ns::A, QLatin1String(name)); }

// Where the data part names the drawing's link: the first dsp:dataModelExt with a relId. The part may be large, so this reads the bytes in place.
bool findDrawingLinkId(const QByteArray &xml, qsizetype *valueAt, qsizetype *valueEnd)
{
    static const QByteArray tag("dataModelExt");
    qsizetype from = 0;
    // A part that has one at all has it in its extension list; a few are looked at, not every one.
    for (int tries = 0; tries < 16; ++tries) {
        const qsizetype at = xml.indexOf(tag, from);
        if (at < 0) return false;
        const qsizetype close = xml.indexOf('>', at);
        if (close < 0) return false;
        const QByteArray element = QByteArray::fromRawData(xml.constData() + at, close - at);
        const qsizetype name = element.indexOf(" relId=\"");
        if (name >= 0) {
            const qsizetype start = at + name + 8;
            const qsizetype stop = xml.indexOf('"', start);
            if (stop > start && stop < close) {
                *valueAt = start;
                *valueEnd = stop;
                return true;
            }
        }
        from = close;
    }
    return false;
}

} // namespace

bool isDiagramFrame(const QDomElement &graphicFrame)
{
    if (graphicFrame.isNull() || graphicFrame.localName() != QLatin1String("graphicFrame")) return false;
    const QDomElement data = aChild(aChild(graphicFrame, "graphic"), "graphicData");
    return attr(data, QStringLiteral("uri")) == kDiagramNs;
}

DiagramLinks diagramFrameLinks(const QDomElement &graphicFrame)
{
    DiagramLinks links;
    if (!isDiagramFrame(graphicFrame)) return links;
    for (QDomElement c = aChild(aChild(graphicFrame, "graphic"), "graphicData").firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (c.localName() != QLatin1String("relIds") || c.namespaceURI() != kDiagramNs) continue;
        links.data = attrNs(c, ns::R, QStringLiteral("dm"));
        links.layout = attrNs(c, ns::R, QStringLiteral("lo"));
        links.style = attrNs(c, ns::R, QStringLiteral("qs"));
        links.colors = attrNs(c, ns::R, QStringLiteral("cs"));
        break;
    }
    return links;
}

QString diagramDrawingLinkId(const QByteArray &dataXml)
{
    qsizetype at = 0, end = 0;
    return findDrawingLinkId(dataXml, &at, &end) ? QString::fromUtf8(dataXml.mid(at, end - at)) : QString();
}

QByteArray withDiagramDrawingLinkId(const QByteArray &dataXml, const QString &id)
{
    qsizetype at = 0, end = 0;
    if (!findDrawingLinkId(dataXml, &at, &end)) return dataXml;
    QByteArray out = dataXml;
    out.replace(at, end - at, id.toUtf8());
    return out;
}

QString diagramDrawingPartOf(const QVector<Relationship> &links, const std::function<QByteArray(const QString &part)> &partData, const QString &dataId)
{
    if (dataId.isEmpty()) return QString();
    auto link = [&](const QString &id) {
        for (const Relationship &r : links)
            if (r.id == id) return r;
        return Relationship();
    };
    const Relationship data = link(dataId);
    if (data.id.isEmpty() || data.external || !data.type.endsWith(QLatin1String("/diagramData"))) return QString();
    const QByteArray dataXml = partData(data.target);
    if (dataXml.isEmpty() || dataXml.size() > kMostDiagramDataBytes) return QString();
    const QString drawingId = diagramDrawingLinkId(dataXml);
    if (drawingId.isEmpty()) return QString();
    const Relationship drawing = link(drawingId);
    if (drawing.id.isEmpty() || drawing.external || !drawing.type.endsWith(QLatin1String("/diagramDrawing"))) return QString();
    return drawing.target;
}

QString diagramDrawingPartOf(const Package &package, const QString &part, const QString &dataId)
{
    const QString target = diagramDrawingPartOf(package.relationships(part), [&package](const QString &name) { return package.hasPart(name) && package.partSize(name) <= kMostDiagramDataBytes ? package.part(name) : QByteArray(); }, dataId);
    return !target.isEmpty() && package.hasPart(target) && package.partSize(target) <= kMostDiagramDrawingBytes ? target : QString();
}

} // namespace jp::office
