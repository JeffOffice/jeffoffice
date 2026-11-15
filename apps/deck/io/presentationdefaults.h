#pragma once
// What new blank presentations start from: the theme and the slide size that Set as Default kept (Design > Set as default theme, and Slide Size > Set as default), else the
// ones a new presentation always has.

#include "core/deck.h"

#include <QSizeF>
#include <QString>
#include <memory>

namespace jp::deck {

// The file that holds the default theme (a presentation with the theme and no slides' words), in the app's data folder.
QString defaultThemePath();
// Keeps the theme `current` has as the default one. False, with `error` told, when the file can't be written.
bool setDefaultTheme(const Document &current, QString *error = nullptr);
// Keeps a slide size as the default one, and forgets it.
void setDefaultSlideSize(const QSizeF &size);
void clearDefaultSlideSize();
// A new blank presentation: the title slide alone, with the default theme and slide size when there are any.
std::unique_ptr<Document> newDefaultPresentation();

} // namespace jp::deck
