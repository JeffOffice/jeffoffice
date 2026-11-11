#pragma once
// A barcode (Insert > Barcode) as an object to put on a sheet: a picture of the drawing, which other programs show (the SVG, with a PNG beside it for those that don't draw SVG), whose
// properties hold the marker with the barcode's settings (office/barcodemarker.h), so that this program opens it as a barcode again. It is put on the sheet as any picture is.

#include "core/objectclip.h"

#include <QJsonObject>

namespace jp::crunch {

// The clip of the barcode the settings describe, at its own size (the grid's unit is a pixel at 100%); false, with the reason in *why, for settings that are no barcode the encoder draws.
bool barcodeClip(const QJsonObject &settings, ObjectClip *out, QString *why = nullptr);

} // namespace jp::crunch
