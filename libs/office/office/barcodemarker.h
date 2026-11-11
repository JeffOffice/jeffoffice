#pragma once
// The marker that keeps a barcode editable in the files of other programs. A barcode (Insert > Barcode) is saved as a picture every program shows (an SVG with a PNG beside it, or
// the PNG alone), and this marker, which most programs pass over, holds the settings it was made from and a fingerprint of the picture, so that JeffOffice opens it as a barcode
// again. In a drawing (a presentation's slide, a sheet's drawing, a document's drawing) it is an extension of the picture's non-visual properties:
//
//   <a:extLst><a:ext uri="{5B6C1E0A-2F4D-4C7B-9A3E-8D1F6A2B7C40}">
//     <jo:barcode xmlns:jo="https://jeffoffice.com/ns/barcode" version="1" settings="{...}" png-sha256="..." svg-sha256="..."/></a:ext></a:extLst>
//
// inside p:cNvPr, xdr:cNvPr, wp:docPr, or pic:cNvPr; in an open document, attributes of the picture's draw:frame (see below). `settings` is the JSON object of
// core/barcodeitem.h, as text. The marker is honored only where the picture in the file is the one that was written: the SHA-256 of the PNG's bytes (and of the SVG's, where the
// file has one) must be the marker's. A picture changed by another program, or put in place of ours, is a plain picture. A marker whose settings are no barcode the
// encoder draws, or are too long, is passed over too.

#include <QByteArray>
#include <QDomElement>
#include <QJsonObject>
#include <QString>
#include <QXmlStreamWriter>

namespace jp::office {

constexpr char kBarcodeNamespace[] = "https://jeffoffice.com/ns/barcode";
constexpr char kBarcodeExtUri[] = "{5B6C1E0A-2F4D-4C7B-9A3E-8D1F6A2B7C40}";
// The most barcodes of a document that are honored as barcodes (the rest are plain pictures): each is laid out and drawn.
constexpr int kMostBarcodesHonored = 2000;

// What a marker says: the settings (empty for none), and the fingerprints of the pictures it was written with (the SVG's is empty where there was none).
struct BarcodeMarker {
    QJsonObject settings;
    QString pngHash, svgHash;
    bool isEmpty() const { return settings.isEmpty(); }
};

// The SHA-256 of bytes, in lowercase hex.
QString barcodeHash(const QByteArray &bytes);
// Whether the pictures in the file are the ones the marker was written with. `svg` is null when the file's picture has no SVG; a PNG that is not the marker's is no match, and an SVG
// the file has must be the marker's.
bool barcodePicturesMatch(const BarcodeMarker &marker, const QByteArray &png, const QByteArray *svg);

// The settings as the text the marker holds (compact JSON), and the settings of such a text: empty unless it is a JSON object of at most kMostBarcodeSettingsBytes (core/barcodeitem.h)
// that the encoder draws (the layout is kept, so that using the barcode does not encode it again).
QString barcodeSettingsText(const QJsonObject &settings);
QJsonObject barcodeSettingsFromText(const QString &text);

// The extension list or the extension alone, written as text with the `a:` prefix (the caller declares it), or the list by a writer. `svgHash` may be empty.
QByteArray barcodeMarkerXml(const QJsonObject &settings, const QString &pngHash, const QString &svgHash = QString());
QByteArray barcodeMarkerExtXml(const QJsonObject &settings, const QString &pngHash, const QString &svgHash = QString());
void writeBarcodeMarker(QXmlStreamWriter &w, const QJsonObject &settings, const QString &pngHash, const QString &svgHash = QString());
// The marker's extension alone, written as the inside of an a:extLst a writer has open (where another extension is written beside it).
void writeBarcodeMarkerExt(QXmlStreamWriter &w, const QJsonObject &settings, const QString &pngHash, const QString &svgHash = QString());
// `inner`, the children of a picture's non-visual properties, with the marker's extension in it: in the extension list it has (replacing ours, beside any others, which are kept as
// they were), else in a new one. `declarations` are the namespace declarations `inner` is read under (the prefixes of a drawing).
QByteArray withBarcodeMarker(const QByteArray &inner, const QByteArray &extXml, const QByteArray &declarations);

// The marker a picture's non-visual properties hold, or none.
BarcodeMarker readBarcodeMarker(const QDomElement &cNvPr);
// The same from XML read under `declarations` (the prefixes a drawing declares): the first marker anywhere in it, whatever prefixes it uses.
BarcodeMarker readBarcodeMarkerXml(const QByteArray &xml, const QByteArray &declarations = QByteArray());
// An open document's frame holds it as attributes in JeffOffice's own namespace (urn:jeffoffice:names:extension:1.0): jo:barcode (the settings) and jo:barcode-sha256 (the PNG's).
inline BarcodeMarker barcodeOdfMarker(const QString &settings, const QString &pngHash) { return BarcodeMarker{barcodeSettingsFromText(settings), pngHash, QString()}; }

} // namespace jp::office
