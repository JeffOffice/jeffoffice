#pragma once
// What a shape's properties say about how it looks: colors, fill, outline,
// shadow, and a hyperlink ([MS-ODRAW] 2.2.2 and 2.3).

#include "core/items.h"
#include "office/odraw_props.h"

namespace jp::office::odraw {

// The color of a color property (an OfficeArtCOLORREF): the RGB it holds, the
// host's scheme color, or a system color, which may stand for the shape's own
// fill, line, or shadow color. The property's default when it has none.
QColor colorProperty(const Props &props, const OdrawContext &ctx, quint16 id);
// A picture's brightness and contrast as the items have them (a percent each
// way, -100 to 100), from the shape's properties; 0 when it sets none.
double pictureBrightness(const Props &props);
double pictureContrast(const Props &props);
// The shape's fill; `filledByDefault` is what a shape with no say in the
// matter is (a text box isn't filled, a rectangle is).
Fill readFill(const Props &props, const OdrawContext &ctx, bool filledByDefault, const QRectF &shape = QRectF());
Stroke readStroke(const Props &props, const OdrawContext &ctx, bool lineByDefault);
// Sets the shadow of `fx` when the shape has one.
void readShadow(const Props &props, const OdrawContext &ctx, Effects &fx);
// The address of a hyperlink (an IHlink: the URL or file, and a place in it).
QString hyperlinkOf(const QByteArray &hlink);

} // namespace jp::office::odraw
