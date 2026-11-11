#include "office/barcodemarker.h"

#include "core/barcodeitem.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QXmlStreamReader>

namespace jp::office {

namespace {
const char kMainNs[] = "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"";
// The wrapper element a slice of XML is read under: the declarations, and the drawing namespace's `a` unless they have it.
QByteArray wrapper(const QByteArray &declarations)
{
    return "<x " + declarations + (declarations.contains("xmlns:a=") ? QByteArray() : " " + QByteArray(kMainNs)) + ">";
}

QByteArray escaped(const QString &text) { return text.toHtmlEscaped().toUtf8(); }

QByteArray attributesOf(const QJsonObject &settings, const QString &pngHash, const QString &svgHash)
{
    QByteArray a = " version=\"1\" settings=\"" + escaped(barcodeSettingsText(settings)) + "\"";
    if (!pngHash.isEmpty()) a += " png-sha256=\"" + pngHash.toUtf8() + "\"";
    if (!svgHash.isEmpty()) a += " svg-sha256=\"" + svgHash.toUtf8() + "\"";
    return a;
}
} // namespace

QString barcodeHash(const QByteArray &bytes) { return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex()); }

bool barcodePicturesMatch(const BarcodeMarker &marker, const QByteArray &png, const QByteArray *svg)
{
    if (marker.pngHash.isEmpty() || png.isEmpty() || barcodeHash(png) != marker.pngHash) return false;
    if (svg && (marker.svgHash.isEmpty() || barcodeHash(*svg) != marker.svgHash)) return false;
    return true;
}

QString barcodeSettingsText(const QJsonObject &settings) { return QString::fromUtf8(QJsonDocument(settings).toJson(QJsonDocument::Compact)); }

QJsonObject barcodeSettingsFromText(const QString &text)
{
    const QByteArray bytes = text.toUtf8();
    if (bytes.isEmpty() || bytes.size() > kMostBarcodeSettingsBytes) return {};
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) return {};
    const QJsonObject settings = doc.object();
    // (Only settings the encoder draws: a marker with the data gone, or a type it doesn't know, is no barcode. The layout is kept for what draws it.)
    const barcode::Layout l = barcodeLayout(settings);
    return l.error.isEmpty() && !l.bars.isEmpty() ? settings : QJsonObject();
}

QByteArray barcodeMarkerExtXml(const QJsonObject &settings, const QString &pngHash, const QString &svgHash)
{
    return "<a:ext uri=\"" + QByteArray(kBarcodeExtUri) + "\"><jo:barcode xmlns:jo=\"" + QByteArray(kBarcodeNamespace) + "\"" + attributesOf(settings, pngHash, svgHash) + "/></a:ext>";
}

QByteArray barcodeMarkerXml(const QJsonObject &settings, const QString &pngHash, const QString &svgHash)
{
    return "<a:extLst>" + barcodeMarkerExtXml(settings, pngHash, svgHash) + "</a:extLst>";
}

void writeBarcodeMarkerExt(QXmlStreamWriter &w, const QJsonObject &settings, const QString &pngHash, const QString &svgHash)
{
    w.writeStartElement(QStringLiteral("a:ext"));
    w.writeAttribute(QStringLiteral("uri"), QLatin1String(kBarcodeExtUri));
    w.writeStartElement(QStringLiteral("jo:barcode"));
    w.writeAttribute(QStringLiteral("xmlns:jo"), QLatin1String(kBarcodeNamespace));
    w.writeAttribute(QStringLiteral("version"), QStringLiteral("1"));
    w.writeAttribute(QStringLiteral("settings"), barcodeSettingsText(settings));
    if (!pngHash.isEmpty()) w.writeAttribute(QStringLiteral("png-sha256"), pngHash);
    if (!svgHash.isEmpty()) w.writeAttribute(QStringLiteral("svg-sha256"), svgHash);
    w.writeEndElement();
    w.writeEndElement();
}

void writeBarcodeMarker(QXmlStreamWriter &w, const QJsonObject &settings, const QString &pngHash, const QString &svgHash)
{
    w.writeStartElement(QStringLiteral("a:extLst"));
    writeBarcodeMarkerExt(w, settings, pngHash, svgHash);
    w.writeEndElement();
}

QByteArray withBarcodeMarker(const QByteArray &inner, const QByteArray &extXml, const QByteArray &declarations)
{
    // Where the first extension list of the properties is, and where our extension is in it, found by reading the XML (offsets in characters of the text).
    const QString text = QString::fromUtf8(inner);
    const QString open = QString::fromLatin1(wrapper(declarations));
    QXmlStreamReader x(open + text + QStringLiteral("</x>"));
    const qint64 base = open.size();
    qint64 listStart = -1, listBodyStart = -1, listEnd = -1, closeTag = -1, extStart = -1, extEnd = -1;
    int depth = 0;
    bool selfClosed = false;
    qint64 mark = 0;
    while (!x.atEnd()) {
        const qint64 before = x.characterOffset();
        x.readNext();
        if (x.isStartElement()) {
            ++depth;
            if (depth == 2 && x.name() == QLatin1String("extLst") && listStart < 0) {
                listStart = before;
                listBodyStart = x.characterOffset();
            } else if (depth == 3 && listStart >= 0 && listEnd < 0 && extStart < 0 && x.name() == QLatin1String("ext") && x.attributes().value(QLatin1String("uri")) == QLatin1String(kBarcodeExtUri)) {
                extStart = before;
                mark = 3;
            }
        } else if (x.isEndElement()) {
            if (depth == 3 && mark == 3 && extEnd < 0) extEnd = x.characterOffset();
            if (depth == 2 && listStart >= 0 && listEnd < 0 && x.name() == QLatin1String("extLst")) {
                listEnd = x.characterOffset();
                closeTag = before;
                selfClosed = before == listBodyStart && text.mid(int(listStart - base), int(listEnd - listStart)).endsWith(QLatin1String("/>"));
            }
            --depth;
        }
    }
    if (x.hasError()) return inner + "<a:extLst>" + extXml + "</a:extLst>";   // (inner is not well formed here: the extension is added in a list of its own)
    const QString ext = QString::fromUtf8(extXml);
    if (listStart < 0) return (text + QStringLiteral("<a:extLst>") + ext + QStringLiteral("</a:extLst>")).toUtf8();
    if (extStart >= 0 && extEnd > extStart) return (text.left(int(extStart - base)) + ext + text.mid(int(extEnd - base))).toUtf8();
    if (selfClosed) return (text.left(int(listStart - base)) + QStringLiteral("<a:extLst>") + ext + QStringLiteral("</a:extLst>") + text.mid(int(listEnd - base))).toUtf8();
    return (text.left(int(closeTag - base)) + ext + text.mid(int(closeTag - base))).toUtf8();
}

BarcodeMarker readBarcodeMarker(const QDomElement &cNvPr)
{
    for (QDomElement list = cNvPr.firstChildElement(); !list.isNull(); list = list.nextSiblingElement()) {
        if (list.localName() != QLatin1String("extLst")) continue;
        for (QDomElement ext = list.firstChildElement(); !ext.isNull(); ext = ext.nextSiblingElement()) {
            if (ext.attribute(QStringLiteral("uri")) != QLatin1String(kBarcodeExtUri)) continue;
            for (QDomElement m = ext.firstChildElement(); !m.isNull(); m = m.nextSiblingElement())
                if (m.localName() == QLatin1String("barcode") && m.namespaceURI() == QLatin1String(kBarcodeNamespace))
                    if (const QJsonObject s = barcodeSettingsFromText(m.attribute(QStringLiteral("settings"))); !s.isEmpty())
                        return {s, m.attribute(QStringLiteral("png-sha256")), m.attribute(QStringLiteral("svg-sha256"))};
        }
    }
    return {};
}

BarcodeMarker readBarcodeMarkerXml(const QByteArray &xml, const QByteArray &declarations)
{
    if (!xml.contains(kBarcodeExtUri)) return {};
    QXmlStreamReader x(wrapper(declarations) + xml + "</x>");
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement() && x.name() == QLatin1String("barcode") && x.namespaceUri() == QLatin1String(kBarcodeNamespace))
            if (const QJsonObject s = barcodeSettingsFromText(x.attributes().value(QLatin1String("settings")).toString()); !s.isEmpty())
                return {s, x.attributes().value(QLatin1String("png-sha256")).toString(), x.attributes().value(QLatin1String("svg-sha256")).toString()};
    }
    return {};
}

} // namespace jp::office
