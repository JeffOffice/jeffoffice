#pragma once
// The drawing format's shape numbers: the binary OfficeArt drawings in .ppt,
// .doc, .xls, and .pub files number their preset shapes, and these are the
// ones JeffOffice draws too.

#include <QString>

namespace jp {

// The shape number for a preset, or -1 when the format has no such shape.
int drawingShapeType(const QString &preset);
// The preset for a shape number, or an empty string.
QString presetForDrawingShapeType(int type);
// Whether the preset's standard form looks exactly like the numbered shape's
// at every proportion; the others differ in their default proportions or
// handles.
bool drawsAsShapeType(const QString &preset);

// The Text Art shape number for a Text Art transform (plain text when
// unknown), and back (an empty string when the number isn't Text Art).
int textArtShapeType(const QString &transform);
QString textArtForShapeType(int type);

} // namespace jp
