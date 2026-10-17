#pragma once
// Insert > Text Art: the gallery of styles and a new Text Art in one of them,
// which every app with pages shares.

#include "app/widgets.h"
#include "core/items.h"

#include <QTextCharFormat>
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

// The look of the style `styleId` names (its fill, outline, and shadow; not its font or its warp) set on the letters of a run, as the Text Art
// Styles of a shape's text give it; false, with the format as it was, for a style there is none of.
bool applyTextArtStyleToRun(QTextCharFormat &cf, const QString &styleId, const ColorScheme &scheme);

} // namespace jp
