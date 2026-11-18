#pragma once
// How the words of a text box run (across, or turned a quarter either way) and how a text box is drawn with the mouse: what every app shares, so that a file's value and a drag
// mean the same in each (see docs/features/jeffwrite.md).

#include "core/export.h"

#include <QPointF>
#include <QSizeF>
#include <QString>

#include <optional>
#include <utility>

namespace jp {

enum class TextDirection { Horizontal, Rotate90, Rotate270 };

// The body's `vert` value of a drawing file: "horz", "vert", "vert270".
JO_COMMON_EXPORT const char *textDirectionVert(TextDirection direction);
// The direction a `vert` value gives: "horz", "vert", "eaVert" (turned 90 here), "vert270". Any other value is not known (the caller reads Horizontal and keeps the value as it was).
JO_COMMON_EXPORT std::optional<TextDirection> textDirectionFromVert(const QString &vert);
// The quarter turns the in-place editor lays the words out with: 0, 1 (clockwise), 3.
JO_COMMON_EXPORT int textDirectionQuarterTurns(TextDirection direction);
JO_COMMON_EXPORT TextDirection textDirectionFromQuarterTurns(int quarterTurns);
// The name a command shows ("Horizontal", "Rotate all text 90°", "Rotate all text 270°").
JO_COMMON_EXPORT QString textDirectionName(TextDirection direction);

// In an ODF file the direction is the writing mode of the box's style: `style:writing-mode="tb-rl"` turns it 90 degrees, and `loext:writing-mode="bt-lr"` 270 (the value other programs
// write for text that reads upward, in their extension namespace). The attributes to write (with a leading space, none for Horizontal: the caller writes its own mode), and what
// the values read from a style give: `styleMode` is style:writing-mode, `extensionMode` loext:writing-mode (either may be empty), `turnedBack` the jo:turned-back of files written before.
JO_COMMON_EXPORT QString textDirectionOdfAttributes(TextDirection direction);
JO_COMMON_EXPORT TextDirection textDirectionFromOdf(const QString &styleMode, const QString &extensionMode, bool turnedBack = false);

// The corners of a text box drawn by dragging from one point of the page to another (points; put in order): a drag of under 4 points both ways gives the `usual` size from `from`; any other
// has each side at least 18 points (a box that can hold a line); `keepProportions` (Shift) makes it as wide as it is tall. The preview and the box made are both this.
JO_COMMON_EXPORT std::pair<QPointF, QPointF> textBoxDragEnds(const QPointF &from, const QPointF &to, bool keepProportions, const QSizeF &usual);

} // namespace jp
