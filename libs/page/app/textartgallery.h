#pragma once
// Insert > Text Art: the gallery of styles and a new Text Art in one of them,
// which every app with pages shares.

#include "app/widgets.h"
#include "core/items.h"

#include <QVector>
#include <memory>

namespace jp {

class Editor;

// The gallery's items, one for each style: "Aa" drawn in it, in the colors of
// the editor's document.
QVector<GalleryItem> textArtGalleryItems(Editor *ed);
// A new Text Art in the style the item `styleId` names (the plain style when no
// style has that name), with the program's words to start from.
std::shared_ptr<TextArtItem> newTextArt(const QString &styleId);

} // namespace jp
