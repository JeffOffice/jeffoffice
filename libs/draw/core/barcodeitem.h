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

// A barcode as an SVG picture of its own size in points (the quiet zone, the bars and digits), at whatever size the group has now, and the same as PNG
// (`pixels` on the longer side, at most 4000) for programs that don't draw SVG. Empty for an item that is no barcode.
QByteArray barcodeSvg(const GroupItem &g);
QByteArray barcodePng(const GroupItem &g, int pixels = 1200);

} // namespace jp
