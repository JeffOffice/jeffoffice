#pragma once
// Insert > Barcode in the apps that place objects on a page or a slide: the shared dialog (app/barcodedialog.h), and the barcode it makes put on the page,
// or put in place of the selected barcode, as one step that undoes.

#include <QWidget>

namespace jp {

class Editor;

// With a barcode selected the dialog edits it (the new one takes its place and its identity); else the barcode goes in the middle of the page, or a quarter
// inch on from a barcode already there.
// With `findRoom`, a new barcode goes in the lower right of the page, or the first of a few other places, where nothing else is, when there is room.
void barcodeDialog(QWidget *parent, Editor *ed, bool findRoom = false);

} // namespace jp
