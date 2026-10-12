#pragma once
// Reading DrawingML, the drawing language of .pptx, .docx, and .xlsx files
// (ECMA-376 Part 1, chapter 20): shapes, pictures, connectors, and groups
// become placed objects (libs/draw), and text bodies become stories.
// Elements are matched by local name in either form of the namespace, so the
// same code reads a slide's p:sp and a document's wps:wsp.

#include "core/items.h"
#include "office/drawingml_color.h"
#include "office/package.h"

#include <QDomElement>
#include <QVector>
#include <functional>

class QTextDocument;

namespace jp::office {

// What reading a drawing needs from around it.
struct DrawingContext {
    const Package *package = nullptr;
    QString part;                 // the part the drawing is in: its relationships name the pictures
    Theme theme;
    ColorMap colors;
    // The theme's a:fmtScheme, for style references (p:style's lnRef,
    // fillRef, effectRef: an index into its line, fill, and effect lists).
    QDomElement formats;
    // Keeps a picture's bytes ("png", "jpeg", "gif", "bmp", "tiff", "emf",
    // "wmf", "svg"); returns its id for PictureItem::imageId or a picture fill.
    std::function<QString(const QByteArray &bytes, const QString &format)> addImage;
    // A new empty story's id, and a story's document by id.
    std::function<QString()> newStory;
    std::function<QTextDocument *(const QString &id)> storyDoc;
};

// The list styles a text body inherits, lowest priority first. Each is an
// element whose children are a:defPPr and a:lvl1pPr ... a:lvl9pPr (a:lstStyle,
// p:titleStyle, p:bodyStyle, p:otherStyle, p:defaultTextStyle); a paragraph
// at level n takes each property from the highest one that sets it in
// a:lvl<n+1>pPr (and its a:defRPr for runs), then its own a:pPr and a:rPr.
using TextStyleStack = QVector<QDomElement>;

// A shape tree's element: p:sp (a shape or text box), p:pic (a picture),
// p:cxnSp (a connector), p:grpSp (a group); null for what isn't read yet
// (p:graphicFrame's tables and charts, content parts), which the caller keeps
// to write back. `inheritsFrom` lists, highest priority first, the elements a
// placeholder takes what it leaves out from (its layout's p:sp, then its slide
// master's): position and size (a:xfrm), geometry, fill, line, and the text
// body's a:bodyPr. The placeholder's role ("ph:<type>[:<idx>]") is the
// caller's to set.
ItemPtr readShape(const QDomElement &e, const DrawingContext &ctx, const TextStyleStack &styles = {},
                  const QVector<QDomElement> &inheritsFrom = {});
// Every shape in a shape tree (p:spTree, or a group's children), in drawing
// order, without placeholder inheritance.
ItemList readShapeTree(const QDomElement &tree, const DrawingContext &ctx, const TextStyleStack &styles = {});

// A text body (p:txBody, a:txBody, wps:txbx's content) into `doc`, replacing
// its text: paragraphs (alignment, indents, spacing, bullets and numbering,
// level), runs (font, size, bold, italic, underline, strike, color, baseline,
// letter spacing), line breaks, and fields (slide number, date). `styles` as
// above. The text box's a:bodyPr (insets, anchoring, a:normAutofit's
// shrinking) is the shape's, read by readShape.
void readTextBody(const QDomElement &txBody, QTextDocument *doc, const DrawingContext &ctx, const TextStyleStack &styles = {});

// A shape's fill and outline from its properties (a:spPr), with its style
// (p:style) for what spPr leaves out.
Fill readFill(const QDomElement &spPr, const QDomElement &style, const DrawingContext &ctx);
Stroke readLine(const QDomElement &spPr, const QDomElement &style, const DrawingContext &ctx);
// A background (p:bg: its p:bgPr fill, or a p:bgRef into the theme's
// background fills) as a fill.
Fill readBackground(const QDomElement &bg, const DrawingContext &ctx);

// The shape id the shape library draws (render/shapes) for a preset
// geometry name ("rect", "roundRect", "rightArrow"...); "" when it has none.
QString shapeForPreset(const QString &prst);

} // namespace jp::office
