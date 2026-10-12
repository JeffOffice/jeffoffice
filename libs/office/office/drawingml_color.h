#pragma once
// DrawingML colors (a:srgbClr and its kin): the color a fill, line, or text
// asks for, once the theme has said what its named colors are and the
// transforms (a:lumMod, a:alpha, ...) have been applied.

#include "office/theme.h"

#include <QColor>
#include <QDomElement>
#include <QString>

namespace jp::office {

// Which theme color each name that drawings use stands for. A slide master
// has one (p:clrMap) and a layout or slide may replace it
// (a:overrideClrMapping), which is how a dark slide gets a light "bg1".
// Without one: bg1 is light 1, tx1 dark 1, bg2 light 2, tx2 dark 2, and the
// accents and hyperlinks are themselves.
struct ColorMap {
    Theme::Color bg1 = Theme::Light1, tx1 = Theme::Dark1, bg2 = Theme::Light2, tx2 = Theme::Dark2;
    Theme::Color accent[6] = {Theme::Accent1, Theme::Accent2, Theme::Accent3, Theme::Accent4, Theme::Accent5, Theme::Accent6};
    Theme::Color hlink = Theme::Hyperlink, folHlink = Theme::FollowedHyperlink;

    // The theme color a schemeClr value names: bg1 and the others through
    // this map, dk1, lt1, dk2, lt2 (and the accents and hyperlinks) as
    // themselves. False for anything else, "phClr" included.
    bool resolve(const QString &name, Theme::Color *slot) const;
};

// The map in p:clrMap or a:overrideClrMapping, from the element's attributes
// (bg1="lt1", ...). A missing or unknown one is the usual. For
// a:masterClrMapping, which means "the master's", use the master's map; a
// null element gives the usual map.
ColorMap readColorMap(const QDomElement &e);

// The color that a color-choice element holds: the element that CONTAINS an
// a:srgbClr, a:schemeClr, a:sysClr, a:prstClr, a:hslClr, or a:scrgbClr (a
// solidFill, a gradient stop, a text run's color). Its transforms are applied
// in the order written: alpha, alphaMod, alphaOff; hue, hueMod, hueOff, sat,
// satMod, satOff, lum, lumMod, lumOff (in HSL); red, green, blue with Mod and
// Off (in sRGB); shade and tint (in linear RGB: shade scales each channel,
// tint moves it toward white); comp, inv, gray. Others (gamma, invGamma, and
// what a later edition adds) are ignored. `phClr` is what a schemeClr
// "phClr" (the placeholder color, set where a style is used) stands for. An
// invalid color if the element holds none, or one that can't be told.
QColor readColor(const QDomElement &choiceParent, const Theme &theme, const ColorMap &map, const QColor &phClr = QColor());

} // namespace jp::office
