#pragma once
// A barcode of Insert > Barcode as a placed object: a group of the white quiet zone and the bars (with their digits) as one vector outline, which keeps
// the settings it was made from (GroupItem::barcode) so it can be edited again. The settings are a JSON object that the dialog fills in and every
// file format saves; this makes the encoder's options, the group, and a vector picture of it from them.

#include "core/barcode.h"
#include "core/items.h"

#include <QByteArray>
#include <QJsonObject>
#include <QPainterPath>
#include <QString>

namespace jp {

// The bars and digits as one outline (barcode-local points). The digits are letter outlines, the same wherever the file is opened.
QPainterPath barcodePath(const barcode::Layout &l);
ItemPtr barcodeItem(const barcode::Layout &l, const QPointF &topLeft, bool whiteBackground, const QString &description, const QJsonObject &settings);

// The encoder's options for settings (as the dialog keeps them: "type" 0 to 6, "data", "currency", "price", "priceMode", "addOn", "magnification" and
// "height" in percent, "text", "check39", "qrLevel").
barcode::Options barcodeOptions(const QJsonObject &settings);
// Whether the settings ask for the white quiet zone.
bool barcodeWhite(const QJsonObject &settings);
// The alternative text of a barcode, in English whatever the language shown ("Barcode: ISBN 978-1-64002-163-1").
QString barcodeDescription(const QJsonObject &settings, const barcode::Layout &l);
// The barcode the settings describe, its top left corner at `topLeft`; none (with the reason in *error) when the settings are no barcode the encoder draws.
ItemPtr barcodeFromSettings(const QJsonObject &settings, const QPointF &topLeft, QString *error = nullptr);

// Whether an item is a barcode (a group that holds its settings).
bool isBarcodeItem(const Item *it);
// The most bytes of settings a file may hold for a barcode (more is no barcode of ours).
constexpr int kMostBarcodeSettingsBytes = 16384;

// The barcode the settings describe, laid out by the encoder, kept by a hash of the settings so that the same settings (a document of many copies, a selection that changes, a
// marker read twice) are encoded once. barcodeEncodes() counts the times the encoder really ran (for tests).
barcode::Layout barcodeLayout(const QJsonObject &settings);
int barcodeEncodes();

// A barcode as an SVG picture of its own size in points (the quiet zone, the bars and digits), at the size the group has now but not turned or flipped (see barcodeTurn), and the
// same as PNG for programs that don't draw SVG. The PNG has `pixels` on its longer side; with 0 it has at least 3 pixels to a module (4 for a QR code), in whole pixels, and at most
// kMostBarcodePngSide on a side (*capped says it was held to that: the SVG stays exact). Empty for an item that is no barcode.
constexpr int kMostBarcodePngSide = 4000;
QByteArray barcodeSvg(const GroupItem &g);
QByteArray barcodePng(const GroupItem &g, int pixels = 0, bool *capped = nullptr);

// The frame of a barcode before it is turned (the union of its parts' own frames), how far it is turned and whether it is flipped, and the same set on a barcode.
QRectF barcodeFrame(const GroupItem &g);
void barcodeTurn(const GroupItem &g, double *rotation, bool *flipH, bool *flipV);
void setBarcodeTurn(GroupItem &g, double rotation, bool flipH, bool flipV);

} // namespace jp
