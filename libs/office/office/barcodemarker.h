#pragma once
// The marker that keeps a barcode editable in the files of other programs. A barcode (Insert > Barcode) is saved as a picture every program shows (an
// SVG with a PNG beside it), and this marker, which the others pass over, holds the settings it was made from, so that JeffOffice opens it as a barcode
// again. In a drawing (a presentation's slide, a sheet's drawing, a document's drawing) it is an extension of the picture's non-visual properties:
//
//   <a:extLst><a:ext uri="{5B6C1E0A-2F4D-4C7B-9A3E-8D1F6A2B7C40}">
//     <jo:barcode xmlns:jo="https://jeffoffice.com/ns/barcode" version="1" settings="{...}"/></a:ext></a:extLst>
//
// inside p:cNvPr, xdr:cNvPr, wp:docPr, or pic:cNvPr; in an open document, the same attributes on the picture's draw:frame (see odf below). `settings` is
// the JSON object of core/barcodeitem.h, as text. A marker whose settings are no barcode the encoder draws (or are too long) is passed over: the
// picture is just a picture.

#include <QByteArray>
#include <QDomElement>
#include <QJsonObject>
#include <QString>
#include <QXmlStreamWriter>

namespace jp::office {

constexpr char kBarcodeNamespace[] = "https://jeffoffice.com/ns/barcode";
constexpr char kBarcodeExtUri[] = "{5B6C1E0A-2F4D-4C7B-9A3E-8D1F6A2B7C40}";

// The settings as the text the marker holds (compact JSON), and the settings of such a text: empty unless it is a JSON object of at most
// kMostBarcodeSettingsBytes (core/barcodeitem.h) that the encoder draws.
QString barcodeSettingsText(const QJsonObject &settings);
QJsonObject barcodeSettingsFromText(const QString &text);

// The extension list, written as text with the `a:` prefix (the caller declares it) or by a writer.
QByteArray barcodeMarkerXml(const QJsonObject &settings);
void writeBarcodeMarker(QXmlStreamWriter &w, const QJsonObject &settings);
// The settings a picture's non-visual properties (p:cNvPr and its like) hold, or none.
QJsonObject barcodeMarkerOf(const QDomElement &cNvPr);
// The same from the XML inside an element (wp:docPr's children, say), whatever prefixes it uses.
QJsonObject barcodeMarkerOfXml(const QByteArray &inner);

// An open document's frame holds it as the attributes jo:barcode (the settings) and jo:barcode-version, with xmlns:jo declared as the namespace above.
constexpr char kBarcodeOdfAttribute[] = "barcode";
// The settings an open document's frame has in its attribute value (empty for none).
inline QJsonObject barcodeOdfSettings(const QString &attributeValue) { return barcodeSettingsFromText(attributeValue); }

} // namespace jp::office
