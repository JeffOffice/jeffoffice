#pragma once
// A small SVG reader for vector artwork: icons, and SVG pictures turned into
// shapes. It reads paths and the basic shapes, groups, <use>, transforms,
// style sheets and fill and stroke (a gradient fills with its first color).
// Text, embedded pictures and foreign content are left out and `skipped`
// says so; clip paths cut the shapes they cover, and masks, filters and
// patterns are ignored.

#include "core/items.h"

#include <QColor>
#include <QPainterPath>
#include <QRectF>
#include <QVector>

class QPainter;
class QSvgRenderer;

namespace jp::svg {

struct Element {
    QPainterPath path;              // in the drawing's coordinates, transforms applied
    QColor fill, stroke;            // invalid = none; alpha = opacity
    double strokeWidth = 1;         // in the drawing's coordinates
    Qt::PenCapStyle cap = Qt::FlatCap;
    Qt::PenJoinStyle join = Qt::MiterJoin;
    QVector<double> dashes;         // dash and gap lengths in the drawing's coordinates; empty = solid
    double dashOffset = 0;
};

struct Drawing {
    QRectF viewBox;                 // the coordinates the elements use
    QSizeF size;                    // the drawing's own size, in points
    QVector<Element> elements;
    bool skipped = false;           // had text or pictures this reader leaves out, or too many parts
    bool overLimit = false;         // stopped at a limit (the walk's own budget, or those of ReadLimits): what is in `elements` is only the start of the drawing
    qint64 steps = 0;               // the nodes walked, <use> repeats included
    bool isValid() const { return !viewBox.isEmpty(); }
};

// What a read may cost, for a drawing that is not trusted (0 is no limit of the caller's): the points of the outlines made (counted as each is added, <use> copies and cut
// strokes included), the parts made, and the nodes walked. The read stops at the first one passed, with `overLimit` set.
struct ReadLimits {
    qint64 points = 0;
    int elements = 0;
    qint64 steps = 0;
};

// `currentColor` is the color a drawing asks for with "currentColor".
Drawing read(const QByteArray &svg, const QColor &currentColor = Qt::black, const ReadLimits *limits = nullptr);
// How many nodes read() has walked on this thread since the program began, <use> repeats included: a count for tests of cost (the walk has a budget).
qint64 walkStepsForTest();
// SVG path data ("M2 8.5c0 2.3 1.5 4 3 5.5l7 7Z").
QPainterPath pathData(const QString &d);
// The drawing as editable artwork shapes (ShapeItem::isArt) filling
// `frame`: a shape for each element, or for each run of elements drawn alike
// when `merge` is set (an icon is then one shape). Several shapes come in a
// group. Colors equal to `current` become `currentRef`, so an icon drawn in
// a scheme color follows the color scheme. Null when nothing is drawn.
ItemPtr shapes(const Drawing &d, const QRectF &frame, bool merge, const QColor &current = QColor(), const ColorRef &currentRef = ColorRef());
// An SVG picture as artwork shapes in the picture's place, turned and
// flipped as it is (its crop, border and effects are left behind).
// `partial` tells whether text or pictures inside it were left out.
ItemPtr pictureShapes(const QByteArray &svg, const PictureItem &pic, bool *partial);
// Draws `svg`, which `renderer` has loaded, stretched over `bounds`. Qt's
// renderer draws masks, filters and patterns but leaves out clip-path, so a
// drawing with clipping is drawn in runs of parts under the same clip, each
// run cut to its clip as the painter's own clip (kept as a vector in PDFs
// and prints), in the order the parts were drawn.
// `maxRuns` (when given) is the most runs a clipped drawing is cut in (each is a drawing of the whole of it again).
void paint(QSvgRenderer &renderer, const QByteArray &svg, QPainter *p, const QRectF &bounds, int maxRuns = 0);
// As `paint`, but the drawing keeps its shape in `bounds` as its preserveAspectRatio says (centered, whole, by default; cut to the bounds for "slice"; stretched for "none").
void paintFitted(QSvgRenderer &renderer, const QByteArray &svg, QPainter *p, const QRectF &bounds, int maxRuns = 0);
// Where `paintFitted` puts the drawing's box (`box`, its viewBox) in `bounds`; `*cut` says whether the drawing is cut to the bounds.
QRectF fittedBox(const QByteArray &svg, const QRectF &box, const QRectF &bounds, bool *cut = nullptr);
// Says the words of the drawing's text with no ink (drawUnseenText), over `bounds` as `paint` stretches the drawing onto them, for a PDF made of the
// drawing: Qt's renderer draws the text as outlines, which a PDF holds as paths, and this keeps the words in it as text that can be found and copied.
// Text that is hidden (display none, hidden visibility, in defs) is left out, and so is any that falls outside `only` (in the painter's coordinates,
// when it is given), a few thousand of them at most.
void paintText(const QByteArray &svg, QPainter *p, const QRectF &bounds, const QRectF &only = QRectF());
// The most an SVG kept as a graphic may be, and what it may cost to draw: the nodes walked (<use> repeats included) and the points of its outlines.
constexpr qint64 kMostGraphicBytes = qint64(4) << 20;
constexpr qint64 kMostDrawSteps = 50000;
constexpr qint64 kMostDrawPoints = 1000000;
constexpr int kMostDrawElements = 50000;
constexpr int kMostClipRuns = 16;
// Whether the first of `bytes` are an SVG's: after a byte order mark, white space, an XML declaration, and comments, the root element is svg. (No more than the start is read.)
bool startsLikeSvg(const QByteArray &bytes);
// Whether an SVG refers to anything outside itself or carries a picture inside itself, the way a reader would find it (the text is parsed: attributes written with character
// references count): a file, an address, a style sheet, a definition of its own (DOCTYPE), a <use> or <image> or fill that is not a part of the SVG, a picture as data. False
// for an SVG that can't be parsed.
bool reachesOutside(const QByteArray &svg);
enum class Vet { Ok, TooLarge, Unreadable, Outside, TooComplex };
// Whether `svg` may be drawn as a graphic: within kMostGraphicBytes, an SVG that parses, not reaching outside itself, and no more to draw than the limits above (<use> that repeats
// a group that repeats another is counted as the repeats it makes, and stops at the limit: nothing is drawn).
Vet vetGraphic(const QByteArray &svg);
// An SVG elliptical arc from p0 to p1, as cubic Béziers.
void arcTo(QPainterPath &path, QPointF p0, double rx, double ry, double phiDeg, bool large, bool sweep, QPointF p1);

} // namespace jp::svg
