#include "office/barcodemarker.h"

#include "core/barcodeitem.h"

#include <QJsonDocument>
#include <QXmlStreamReader>

namespace jp::office {

QString barcodeSettingsText(const QJsonObject &settings) { return QString::fromUtf8(QJsonDocument(settings).toJson(QJsonDocument::Compact)); }

QJsonObject barcodeSettingsFromText(const QString &text)
{
    const QByteArray bytes = text.toUtf8();
    if (bytes.isEmpty() || bytes.size() > kMostBarcodeSettingsBytes) return {};
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) return {};
    const QJsonObject settings = doc.object();
    // (Only settings the encoder draws: a marker with the data gone, or a type it doesn't know, is no barcode.)
    const barcode::Layout l = barcode::make(barcodeOptions(settings));
    return l.error.isEmpty() && !l.bars.isEmpty() ? settings : QJsonObject();
}

QByteArray barcodeMarkerXml(const QJsonObject &settings)
{
    return "<a:extLst><a:ext uri=\"" + QByteArray(kBarcodeExtUri) + "\"><jo:barcode xmlns:jo=\"" + QByteArray(kBarcodeNamespace) + "\" version=\"1\" settings=\""
           + barcodeSettingsText(settings).toHtmlEscaped().toUtf8() + "\"/></a:ext></a:extLst>";
}

void writeBarcodeMarker(QXmlStreamWriter &w, const QJsonObject &settings)
{
    w.writeStartElement(QStringLiteral("a:extLst"));
    w.writeStartElement(QStringLiteral("a:ext"));
    w.writeAttribute(QStringLiteral("uri"), QLatin1String(kBarcodeExtUri));
    w.writeStartElement(QStringLiteral("jo:barcode"));
    w.writeAttribute(QStringLiteral("xmlns:jo"), QLatin1String(kBarcodeNamespace));
    w.writeAttribute(QStringLiteral("version"), QStringLiteral("1"));
    w.writeAttribute(QStringLiteral("settings"), barcodeSettingsText(settings));
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndElement();
}

QJsonObject barcodeMarkerOf(const QDomElement &cNvPr)
{
    for (QDomElement list = cNvPr.firstChildElement(); !list.isNull(); list = list.nextSiblingElement()) {
        if (list.localName() != QLatin1String("extLst")) continue;
        for (QDomElement ext = list.firstChildElement(); !ext.isNull(); ext = ext.nextSiblingElement()) {
            if (ext.attribute(QStringLiteral("uri")) != QLatin1String(kBarcodeExtUri)) continue;
            for (QDomElement m = ext.firstChildElement(); !m.isNull(); m = m.nextSiblingElement())
                if (m.localName() == QLatin1String("barcode") && m.namespaceURI() == QLatin1String(kBarcodeNamespace))
                    if (const QJsonObject s = barcodeSettingsFromText(m.attribute(QStringLiteral("settings"))); !s.isEmpty()) return s;
        }
    }
    return {};
}

QJsonObject barcodeMarkerOfXml(const QByteArray &inner)
{
    if (!inner.contains(kBarcodeExtUri)) return {};
    // (The children are read inside an element that declares the prefixes a file would: a:, r:, and the like are not needed to find the marker.)
    QXmlStreamReader x("<x xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">" + inner + "</x>");
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement() && x.name() == QLatin1String("barcode") && x.namespaceUri() == QLatin1String(kBarcodeNamespace))
            if (const QJsonObject s = barcodeSettingsFromText(x.attributes().value(QLatin1String("settings")).toString()); !s.isEmpty()) return s;
    }
    return {};
}

} // namespace jp::office
