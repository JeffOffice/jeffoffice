#pragma once
// Compress Pictures for the apps with pages (JeffPub and JeffDeck): asks (the shared box), then makes each picture of the document, or of the selection, smaller as the target asks, in one
// step that undoes.

#include <QString>

class QWidget;

namespace jp {

class Editor;

// `allText`: the words of the "every picture" box, which name the document ("Apply to all pictures in the presentation"). Tells how much was saved when it is done.
void compressPicturesDialog(QWidget *parent, Editor *ed, const QString &allText);
// The same with the choices already made (no box, no word of how much was saved); returns the bytes saved.
qint64 compressPictures(Editor *ed, double ppi, bool all, bool deleteCropped);

} // namespace jp
