#pragma once
// A barcode (Insert > Barcode) as an object to put on a sheet: a picture of the drawing, which other programs show (the SVG, with a PNG beside it for those that don't draw SVG), whose
// properties hold the marker with the barcode's settings (office/barcodemarker.h), so that this program opens it as a barcode again. It is put on the sheet as any picture is.

#include "core/objectclip.h"

#include <QJsonObject>

namespace jp::crunch {

// The clip of the barcode the settings describe, at its own size (the grid's unit is a pixel at 100%); false, with the reason in *why, for settings that are no barcode the encoder draws.
// With `provisional`, the PNG is a stand-in of one pixel (the SVG is the real one, and the window draws the vectors): the clip is made in a fraction of the time, for a barcode that follows a cell and
// is made again at each change of it. The save makes the PNG then (SheetObjects::addToSave), and a copy of such a barcode is made with the real one.
bool barcodeClip(const QJsonObject &settings, ObjectClip *out, QString *why = nullptr, bool provisional = false);

} // namespace jp::crunch
