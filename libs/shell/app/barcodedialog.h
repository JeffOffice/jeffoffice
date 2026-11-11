#pragma once
// Insert > Barcode, the dialog every app shares (see barcodedialog.cpp): it asks for the settings of a barcode, which core/barcodeitem.h makes the
// object from and every file format keeps.

#include <QJsonObject>
#include <QWidget>

namespace jp {

// Shows the dialog. `was` is the settings of the barcode being edited (empty for a new one: the dialog starts from the ones used last). True with the
// settings in *out when OK was pressed on a barcode the encoder can draw; false when the dialog was cancelled.
bool askBarcode(QWidget *parent, const QJsonObject &was, QJsonObject *out);

} // namespace jp
