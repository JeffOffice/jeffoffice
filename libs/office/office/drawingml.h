#pragma once
// Reading DrawingML, the drawing language of .pptx, .docx, and .xlsx files
// (ECMA-376 Part 1, chapter 20): shapes, pictures, connectors, and groups
// become placed objects (libs/draw), and text bodies become stories.
// Elements are matched by local name in either form of the namespace, so the
// same code reads a slide's p:sp and a document's wps:wsp.

#include "core/items.h"
#include "office/drawingml_color.h"
#include "office/package.h"

#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QVector>
#include <algorithm>
#include <functional>

class QTextDocument;

namespace jp::office {

// What a whole import may keep of each kind of thing, shared by every read of
// it (DrawingContext::budget), however the file spreads them over parts it
// lists, or lists again: a read takes what it keeps, and leaves out what is
// past it. `ranOut` says which kinds that happened to, for the importer's note.
struct DrawingBudget {
    enum Kind {
        Objects,      // shapes, pictures, connectors, groups, tables, and frames kept as written
        Stories,      // text bodies kept in a story of their own (a shape's or a text box's)
        Cells,        // table cells, each with a story of its own
        Paragraphs,   // paragraphs of all the text bodies
        Runs,         // runs, fields, and line breaks of all the text bodies
        Letters,      // letters of all the text bodies
        KindCount
    };
    qint64 left[KindCount] = {};
    bool ranOut[KindCount] = {};
    bool longBody = false;   // a text body was longer than one may be, however much is left, and was cut

    // `n` of a kind, if that much is left; else nothing is taken, and the kind is marked as run out.
    bool take(Kind kind, qint64 n = 1)
    {
        if (n > left[kind]) {
            ranOut[kind] = true;
            return false;
        }
        left[kind] -= n;
        return true;
    }
    // As much of `n` as is left, taken: below `n` when the kind runs out.
    qint64 takeUpTo(Kind kind, qint64 n)
    {
        const qint64 got = std::max<qint64>(0, std::min(n, left[kind]));
        if (got < n) ranOut[kind] = true;
        left[kind] -= got;
        return got;
    }
};

// What reading a drawing needs from around it.
struct DrawingContext {
    const Package *package = nullptr;
    QString part;                 // the part the drawing is in: its relationships name the pictures
    // The theme's part: the pictures of its fill styles (a background
    // referring to a picture style) are named by its relationships.
    QString themePart;
    // The parts of the XML documents a shape inherits from (a slide
    // placeholder's layout and master), by document: a picture fill it
    // inherits is named by that part's relationships.
    QVector<QPair<QDomDocument, QString>> inheritedParts;
    // The page's background as it shows (its own, else its layout's or
    // master's), for a shape filled with it (p:sp useBgFill).
    Fill background;
    Theme theme;
    ColorMap colors;
    // The theme's a:fmtScheme, for style references (p:style's lnRef,
    // fillRef, effectRef: an index into its line, fill, and effect lists).
    QDomElement formats;
    // Keeps a picture's bytes ("png", "jpeg", "gif", "bmp", "tiff", "emf",
    // "wmf", "svg"); returns its id for PictureItem::imageId or a picture fill.
    std::function<QString(const QByteArray &bytes, const QString &format)> addImage;
    // The pictures kept so far, by part and relationship (and recoloring), and
    // the movies and sounds by the part they are in. Kept by the caller across
    // every read of one file, so that a picture its shapes use again, on the
    // page or the next, is handed to addImage once. Unset, each read keeps its own.
    QHash<QString, QString> *images = nullptr;
    // What the whole import may keep (above); unset, each read has only its own limits.
    DrawingBudget *budget = nullptr;
    // Keeps the movie or sound a picture plays, with its content type and
    // file extension; returns its id for PictureItem::mediaId. Unset, a
    // picture that plays one keeps only an address it links to outside.
    std::function<QString(const QByteArray &bytes, const QString &contentType, const QString &extension)> addMedia;
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
// The effects an a:effectLst in `spPr` (or the run properties of Text Art) names: a shadow, a glow, soft edges.
Effects readEffects(const QDomElement &spPr, const QDomElement &style, const DrawingContext &ctx);
// A background (p:bg: its p:bgPr fill, or a p:bgRef into the theme's
// background fills) as a fill.
Fill readBackground(const QDomElement &bg, const DrawingContext &ctx);

// The shape id the shape library draws (render/shapes) for a preset
// geometry name ("rect", "roundRect", "rightArrow"...); "" when it has none.
QString shapeForPreset(const QString &prst);

} // namespace jp::office
