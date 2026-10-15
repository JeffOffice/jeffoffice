#pragma once
// What the fills, outlines, and effects of DrawingML shapes have in common:
// their elements (a:solidFill, a:ln, a:effectLst ...), and the style a shape
// refers to in the theme (p:style's fillRef, lnRef, and effectRef) for what
// its own properties leave out. drawingml_shape.cpp reads the shapes
// themselves; readFill, readLine, and readBackground (drawingml.h) are here.

#include "office/drawingml.h"

#include <QDomDocument>
#include <QHash>
#include <QSizeF>

namespace jp::office {

struct StyleContext {
    const DrawingContext *ctx = nullptr;
    // What the color phClr stands for: the color of the style's reference.
    QColor placeholder;
    // What a:grpFill is: the fill of the group the shape is in.
    const Fill *groupFill = nullptr;
    // The shape's size, if it is known: a gradient written as scaled follows it.
    QSizeF frame;
    // The pictures added so far, by part and relationship, so that one
    // used again isn't kept twice.
    QHash<QString, QString> *images = nullptr;
    // Reading one of the theme's styles: its pictures' relationships are the theme part's.
    bool fromTheme = false;
};

QDomElement childNamed(const QDomElement &e, const char *local);

// The picture that relationship `rId` of the drawing's part links to, kept
// with ctx.addImage; empty if the link isn't to a picture this can read.
QString loadImage(const QString &rId, const StyleContext &sc, const QDomElement &from = QDomElement());
// The color effects the children of an a:blip list (a:lum, a:grayscl, a:biLevel, a:clrChange, a:clrRepl, a:duotone, a:hsl, a:tint), all of them,
// in the order they are listed and no more than kMaxBlipEffects.
QVector<BlipEffect> readBlipEffects(const QDomElement &blip, const StyleContext &sc);
// "png", "jpeg", "gif", "bmp", "tiff", "emf", "wmf", or "svg" for what the
// bytes are (by how they begin, whatever the part is called); empty if none.
QString imageFormat(const QByteArray &bytes);

// Whether `properties` (an a:spPr or a:grpSpPr) has a fill of its own, which
// includes saying there is none.
bool hasOwnFill(const QDomElement &properties);

// The fill, line, and effects of a shape: from the first of `properties` (the
// shape's a:spPr, then those of what it inherits from) that says, else from
// the style's reference. An outline is made from the style's, changed by
// each of the properties' a:ln from the last to the first.
Fill resolveFill(const QVector<QDomElement> &properties, const QDomElement &style, const StyleContext &sc);
Stroke resolveLine(const QVector<QDomElement> &properties, const QDomElement &style, const StyleContext &sc);
Effects resolveEffects(const QVector<QDomElement> &properties, const QDomElement &style, const StyleContext &sc);

// The list styles for a shape's text: `base`, with what the shape's style
// says of its text (p:style's fontRef: a color, and the heading font if
// "major") as one more, above the others but for the shape's own list
// (`own`, if `base` ends in it). A shape with an accent fill has its text in
// the fontRef's color, usually white. The element made is kept in `keep`.
TextStyleStack withFontReference(const TextStyleStack &base, const QDomElement &style, const QDomElement &own, QDomDocument *keep);

} // namespace jp::office
