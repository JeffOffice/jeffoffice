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
// The largest default theme file that is read; a larger one is not used.
constexpr qint64 kMaxDefaultThemeBytes = qint64(50) << 20;
// How many times the file has been read since the program began: it is read once, and again when the file changes (a count for tests).
int defaultThemeReadsForTest();
// A new blank presentation: the title slide alone, with the default theme and slide size when there are any.
std::unique_ptr<Document> newDefaultPresentation();

} // namespace jp::deck
