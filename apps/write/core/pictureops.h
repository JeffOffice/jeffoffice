#pragma once
// Pictures in the text: Insert Picture (from a file's bytes, or an image from the clipboard), and what is done to a picture once it is there: its size, how the text goes round it
// (in line with the text, square, tight, above and below, behind or in front of the text), its place, and its alt text. A picture is a character of the story that holds the
// drawing element of its run as XML; every command writes that element again and gives the character the form the pages show, in one change of the character's format, so each is
// one step that undoes and the file saved says what the pages show. Only pictures that are one plain drawing (a picture in line, or one anchored) can be changed; any other
// (a picture of the older drawing format, a group) is kept as it is.

#include "core/picturestyles.h"
#include "core/style.h"
#include "core/wrapoutline.h"
#include "core/writedoc.h"
#include "sheet/chartops.h"

#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QMarginsF>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QStringList>
#include <QVector>
#include <QPolygonF>
#include <QTextCursor>
#include <functional>
#include <optional>
#include <utility>

namespace jp::write {

enum class PictureWrap { Inline, Square, Tight, TopAndBottom, BehindText, InFrontOfText };
// Where the text goes beside a square or tight wrap: on both sides of the picture, only to its left or its right, or on the larger side.
enum class PictureWrapSide { Both, Left, Right, Largest };
// Where a floating picture goes among the floating pictures of the story (their layers: the ones behind the text, then the ones in front of it): to the very front or back of its
// layer, or one step forward or backward, which past the end of the layer takes it behind (or in front of) the text.
enum class PictureOrder { BringToFront, SendToBack, BringForward, SendBackward };

// What a drawn object of the text is: a picture, a shape of the drawing format (a rectangle, an arrow, a star, and the rest), a text box, a line, a chart, or a diagram.
enum class DrawnKind { Picture, Shape, TextBox, Line, Chart, Diagram };

// The most bytes of a picture's file that are taken, the most pixels it may hold, and the longest side; and the least and most a picture is made, in points, on a side.
constexpr qint64 kMostPictureBytes = qint64(48) << 20;
constexpr qint64 kMostPicturePixels = 40000000;
constexpr int kMostPictureSide = 30000;
constexpr double kLeastPictureSize = 2, kMostPictureSize = 2000;
// How far a picture may be moved from where its place is measured, in points.
constexpr double kMostPictureOffset = 5000;
// The longest alt text kept.
constexpr int kMostAltTextLength = 2000;

enum class PictureResult {
    Ok,
    Unreadable,    // the bytes are no picture the program can read
    TooLarge,      // more bytes, pixels, or a longer side than the limits above
    External,      // an SVG that refers to anything outside itself
    Refused,       // not where a picture can go (a header, a footer, a note, a kept element), or the document holds the most pictures and objects it can show
};

// The most bytes of an SVG file, and the most the pictures of a document come to together.
constexpr qint64 kMostSvgBytes = qint64(4) << 20;

// The bytes of a picture file, read no further than a picture may be: only a regular file is read (a pipe, a device, and a directory are not), and one longer than kMostPictureBytes is
// TooLarge and read no further. Unreadable for a file that can't be read.
PictureResult readPictureFile(const QString &path, QByteArray *bytes);
// A size held to kLeastPictureSize and kMostPictureSize on both sides together, so that its ratio stays what it was.
void clampKeepingRatio(double *width, double *height);
// The drawing with the id of its wp:docPr changed.
QByteArray withDrawingId(const QByteArray &raw, int id);
// Alt text as it is kept: no character XML refuses, whole pairs of surrogates kept and lone ones dropped, at most kMostAltTextLength characters (a pair is not cut).
QString cleanAltText(const QString &text);

// What a picture of the text is.
struct PictureInfo {
    bool valid = false;        // the character is a picture
    bool editable = false;     // the program can change it (see above)
    bool floating = false;
    PictureWrap wrap = PictureWrap::Inline;
    PictureWrapSide wrapSide = PictureWrapSide::Both;
    double distTop = 0, distBottom = 0, distLeft = 0, distRight = 0;   // the room the wrapping text leaves, in points
    QPolygonF wrapOutline;          // the outline a tight wrap follows, as shares of the picture's frame (empty: the frame itself)
    double width = 0, height = 0;   // points
    double left = 0, top = 0;       // a picture set at a place: from where it is measured (points)
    bool placeIsOffset = false;     // `left` and `top` are offsets (else the picture is aligned, and they are 0)
    QString alt;
    bool decorative = false;   // marked as adding nothing a screen reader should read
    DrawnKind kind = DrawnKind::Picture;
    // A shape's own look (as it is drawn: a style's theme colors worked out): its preset's name, the fill and the line (the color, as RRGGBB, is empty when there is none), the
    // weight of the line in points, how it is dashed (the drawing format's name), and how far it is turned, in degrees.
    QString preset;
    QString fillColor, lineColor, lineDash;
    double lineWeight = 0, rotation = 0;
    // A picture's cut edges as shares of the whole picture, its border, and its brightness and contrast (each from -1 to 1), and whether it is shown in gray.
    double cropLeft = 0, cropTop = 0, cropRight = 0, cropBottom = 0;
    bool grayscale = false, blackWhite = false;
    double brightness = 0, contrast = 0;
    // The shape a picture is cut to (a preset name; "rect" for none), where a floating object is aligned ("left", "center", "right", "inside", "outside" across; "top", "center",
    // "bottom" down; empty when it is set by an offset), and a shape's gradient fill (the two end colors as RRGGBB, and the angle in degrees; empty colors for none).
    QString cropShape = QStringLiteral("rect");
    QString alignAcross, alignDown;
    bool alignedToPage = false;
    QString gradientFrom, gradientTo;
    double gradientAngle = 0;
    bool pictureFill = false;   // a shape filled with a picture
    QJsonObject barcode;        // a barcode of Insert > Barcode: the settings its marker holds (empty for any other picture)
    bool svg = false;           // a graphic: the picture is an SVG (kept with a PNG beside it for the programs that can't draw one)
};

// The layout Set as Default Layout keeps for the pictures put in afterward: the kind of wrap, the side the text goes on, and the room round the picture in points. It is the program's
// (one setting, not the document's); where none was kept, a picture goes in line, as it always did.
struct PictureWrapDefault {
    PictureWrap wrap = PictureWrap::Inline;
    PictureWrapSide side = PictureWrapSide::Both;
    double top = 0, bottom = 0, left = 0, right = 0;
    bool operator==(const PictureWrapDefault &o) const { return wrap == o.wrap && side == o.side && top == o.top && bottom == o.bottom && left == o.left && right == o.right; }
};
PictureWrapDefault pictureWrapDefault();
void setPictureWrapDefault(const PictureWrapDefault &layout);
// The wrap, side, and room of a picture, as a layout to keep.
PictureWrapDefault pictureWrapDefaultOf(const PictureInfo &info);

// The picture at a place of the story (the character just after `position`); `valid` is false for any other character.
PictureInfo pictureAt(const WriteDoc &doc, const QTextDocument *story, int position);
// The character format of the character just after `position`.
QTextCharFormat charFormatAt(const QTextDocument *story, int position);
// Whether the character just after `position` is a picture (a quick test for the pages).
bool isPictureAt(const QTextDocument *story, int position);
// Whether it is a drawn object of any kind (a picture, a shape, a text box, a line): one drawing of the drawing format, which pictureAt describes and the changes below make.
bool isDrawnObjectAt(const QTextDocument *story, int position);

// A picture from the bytes of a picture file (PNG, JPEG, GIF, BMP, and TIFF are kept as they are; any other form the program can read is kept as PNG) at the cursor, in place of a
// selection, as one step that undoes. It is made as large as the picture asks (96 pixels to the inch) but no wider than the column or taller than the page's text. `name` is the file's
// name (the picture's name in the file); `alt` its alt text.
PictureResult insertPicture(WriteDoc &doc, QTextCursor &c, const QByteArray &bytes, const QString &name = QString(), PictureWrap wrap = PictureWrap::Inline, const QString &alt = QString(), const PictureWrapDefault *layout = nullptr);
// An online picture (Insert > Online Pictures): in line at the cursor like a picture of a file, with `title` as its alt text and, when `credit` is not empty, a paragraph under it with the
// credit line, in small italic letters; one step that undoes. The caret is left after the credit, and the picture is at the place the cursor was.
PictureResult insertOnlinePicture(WriteDoc &doc, QTextCursor &c, const QByteArray &bytes, const QString &title, const QString &credit);
// The same for an image (from the clipboard), kept as PNG.
PictureResult insertPictureImage(WriteDoc &doc, QTextCursor &c, const QImage &image, PictureWrap wrap = PictureWrap::Inline, const PictureWrapDefault *layout = nullptr);

// Changes to the picture at `position` of the body text; each is one step, and false (nothing done) when the character is no picture that can be changed or the change would
// make no difference. Sizes are in points and held to the least and most above.
bool setPictureSize(WriteDoc &doc, int position, double width, double height);
// The size and the place (the picture's top left corner on the page) in one step; with no place, only the size; with no size (0), only the place.
bool setPictureBox(WriteDoc &doc, int position, double width, double height, const std::optional<QPointF> &pageTopLeft, bool keepGlue = false);
bool setPictureWrap(WriteDoc &doc, int position, PictureWrap wrap);
// Where the text goes beside a square or tight wrap (a picture in line, behind, or in front of the text has no side: false). One step; false when it makes no difference.
bool setPictureWrapSide(WriteDoc &doc, int position, PictureWrapSide side);
// The room the wrapping text leaves round the picture, each at most kMostWrapDistance points. One step; false when it makes no difference or a value is not usable.
using jp::kMostWrapDistance;
bool setPictureWrapDistances(WriteDoc &doc, int position, double top, double bottom, double left, double right);
// The outline a tight wrap follows (shares of the picture's frame; see core/wrapoutline.h for what is kept: at most kMostWrapPoints, none outside the reach). A square wrap becomes a
// tight one; an empty outline gives the frame back. One step; false when it makes no difference.
bool setPictureWrapOutline(WriteDoc &doc, int position, const QPolygonF &outline);
// The kinds of drawn object whose wrap has an outline to edit (Edit Wrap Points): a picture, a shape, and a text box.
inline bool hasWrapPoints(DrawnKind kind) { return kind == DrawnKind::Picture || kind == DrawnKind::Shape || kind == DrawnKind::TextBox; }
// The outline Edit Wrap Points starts from: the picture's own if it has one, else the frame's corners and the middles of its sides.
QPolygonF startingWrapOutline(const PictureInfo &info);
// Moves the floating picture at `position` in the layers as `order` says, as one step that undoes (the pictures it passes are given heights to keep their order). False where it does
// not move (an inline picture, or one already there); `canOrderPicture` says whether it would.
bool setPictureOrder(WriteDoc &doc, int position, PictureOrder order);
bool canOrderPicture(WriteDoc &doc, int position, PictureOrder order);
bool setPictureAltText(WriteDoc &doc, int position, const QString &alt);

// ---- Insert > Barcode

// A barcode (core/barcodeitem.h makes it from `settings`) at the cursor, in place of a selection, as a floating picture (square wrap, at the paragraph) of its own size, as one step
// that undoes. It is kept as an SVG with a PNG beside it, as other programs show a picture, and the marker with its settings (office/barcodemarker.h) is in the drawing's properties.
// Unreadable for settings that are no barcode the encoder draws.
PictureResult insertBarcode(WriteDoc &doc, QTextCursor &c, const QJsonObject &settings);
// The same in the line, as a character of the text (a merge makes one for each record's value, at the place the field was).
PictureResult insertBarcodeInline(WriteDoc &doc, QTextCursor &c, const QJsonObject &settings);
// The barcode at `position` made from other settings, in the same place and at the same width, as one step; false (nothing done) where the character is no barcode or the settings are none.
bool changeBarcode(WriteDoc &doc, int position, const QJsonObject &settings);

// ---- The Selection Pane's objects

// A floating drawn object of the body text (a picture, shape, text box, line, chart, or diagram that is set at a place), as the Selection Pane lists it.
struct FloatingObject {
    int position = 0;
    QString name;              // its own name (wp:docPr's name), empty when it has none
    bool hidden = false;       // wp:docPr's hidden: the object stays in the document and is not drawn
    bool behind = false;       // in the layer behind the text
    bool floating = true;      // set at a place (else in line with the text: it has no layer, and cannot be moved in the order)
};
// The drawn objects in order: the floating ones, the one in front first (the layer in front of the text from its highest to its lowest, then the layer behind it), and then those in line with the text.
QVector<FloatingObject> floatingObjects(WriteDoc &doc);
// Names the object (empty takes its name away) and hides or shows it, each one step; false where nothing changes. Hiding is saved as the file format's own mark, so other programs keep it.
bool setObjectName(WriteDoc &doc, int position, const QString &name);
bool setObjectHidden(WriteDoc &doc, int position, bool hidden);
// Hides or shows every floating object, as one step.
bool setAllObjectsHidden(WriteDoc &doc, bool hidden);
// Moves the floating object `by` places in the list floatingObjects gives (negative: toward the front), as one step however far it goes; a place past the front layer is behind the text. False for an object in line.
bool moveFloatingObject(WriteDoc &doc, int position, int by);
// Mark as Decorative: the picture is marked as adding nothing to the page's meaning, and its description is taken off; off, the mark goes.
bool setPictureDecorative(WriteDoc &doc, int position, bool decorative);
// Sets a picture at a place on the page, measured from its top left corner (points): it becomes one that is set at a place (square, if it was in line).
// (A connector that is glued lets go when moved or sized by hand, unless `keepGlue` says it moves with the shapes it is glued to.)
bool setPicturePlace(WriteDoc &doc, int position, double pageX, double pageY, bool keepGlue = false);
// Moves an inline picture to another place in the text (`to`, a position outside the picture): one step.
bool movePictureInText(WriteDoc &doc, int position, int to);

// ---- Picture adjustments

enum class PictureRecolor { None, Grayscale, BlackAndWhite };
// Cuts the picture's edges: each a share of the whole picture (0 up to just under 1; a negative one leaves room round the picture), the pair on a side kept under 0.98. The frame
// changes with the cut so that the picture keeps its scale, as the other edits do. One step; false when it makes no difference or the shares are not usable.
bool setPictureCrop(WriteDoc &doc, int position, double left, double top, double right, double bottom);
// How bright and how contrasting the picture is drawn, each from -1 to 1 (0 is as it is).
bool setPictureCorrections(WriteDoc &doc, int position, double brightness, double contrast);
bool setPictureRecolor(WriteDoc &doc, int position, PictureRecolor recolor);
// Takes off the cut, the corrections, the recolor, the border, the shape it is cut to, and the effects: the picture as it came in, at the scale it has (so its frame grows by what the cut took off).
bool resetPicture(WriteDoc &doc, int position);
// Reset Picture & Size: the same, and the picture is the size it has in its file (96 pixels to the inch, held to the sizes a picture may be), the frame growing or shrinking at its corner.
bool resetPictureAndSize(WriteDoc &doc, int position);
// Picture Styles: the picture takes the look of style `style` of the shared gallery (core/picturestyles.h: the shape it is cut to, its border, and its effects), as one step. Its size, place,
// cut, and colors stay. False for no picture that can be changed, or a style that is none, or one the picture already has.
bool setPictureStyle(WriteDoc &doc, int position, int style);
// Picture Effects: `change` sets the picture's effects (a shadow, reflection, glow, soft edges, a bevel, and a turn in space, as the shared drawing engine has them) from what they are; one step.
// The effects the picture's file holds that the engine has no setting for stay. False when `change` makes no difference or the object is no picture.
bool setPictureEffects(WriteDoc &doc, int position, const std::function<void(Effects &)> &change);
// Another picture in the place of this one, with the same place, wrap, alt text, colors, border, shape, and effects; of the old one's proportions it keeps its size and cut, any other has no cut and the frame keeps its width and takes its proportions. The result is as insertPicture's.
PictureResult changePictureFile(WriteDoc &doc, int position, const QByteArray &bytes);
// ---- SVG graphics (the Graphics Format tab)

// The most shapes Convert to Shape makes from one graphic; a graphic with more is refused with a note.
constexpr int kMostGraphicShapes = 10000;
// How deep the groups of an SVG may nest for its colors to be changed (a deeper one is refused); Convert to Shape reads no deeper than the shared reader does, and says what it left out.
constexpr int kMostGraphicNesting = 32;
// Whether an SVG may be drawn as a graphic (as the vetting of graphics and the page's own look at an SVG's cost say); a file's SVG that is not is not drawn (its PNG is). Kept by bytes.
bool graphicDrawable(const QByteArray &svg);
// The SVG of the graphic at `position` (empty for any other object).
QByteArray graphicSvgAt(const WriteDoc &doc, int position);
// The graphic at `position` drawn small (240 pixels on its longer side, in the shape of its frame) on a clear ground, for the tiles of the Graphics Styles gallery; null for any other object.
QImage graphicSample(const WriteDoc &doc, int position);
// Another SVG in the place of the graphic at `position`: the frame (its size and place) stays, the new graphic is drawn in it, and a PNG made of it goes beside it for the programs that
// can't draw one. One step. Refused (with a note in doc.pictureNotice) for an SVG that reaches outside itself or is too large; Unreadable where it can't be drawn.
PictureResult changeGraphic(WriteDoc &doc, int position, const QByteArray &svg);
// The colors of a graphic: each part of the drawing's fills (and its lines) is `Keep`, one color, or none. The SVG is written again with them, and the PNG beside it is drawn again.
struct GraphicColors {
    enum Mode { Keep, Color, None };
    Mode fill = Keep, outline = Keep;
    QColor fillColor, outlineColor;
};
bool setGraphicColors(WriteDoc &doc, int position, const GraphicColors &colors);
// The SVG with the colors put in (empty where it can't be: too large, too deep, or not an SVG). A fill that was taken off, or a line that was not there, is kept in the file so that a later
// color is put back where it was.
QByteArray svgWithColors(const QByteArray &svg, const GraphicColors &colors);
enum class GraphicConvert { Done, NotGraphic, Nothing, TooMany, Outside, Refused };
struct GraphicConversion {
    GraphicConvert result = GraphicConvert::Refused;
    int shapes = 0;        // how many shapes the graphic became
    bool partial = false;  // the graphic held text or pictures, which are left out
};
// Convert to Shape: the graphic at `position` becomes a group of drawing shapes (one shape of the drawing format for each part of the drawing, with its fill and line; clip paths cut the
// shapes they cover), or one shape where the drawing is a single part, in the same frame (place, size, turn, and alt text stay). One step. Past kMostGraphicShapes it is refused.
GraphicConversion convertGraphicToShapes(WriteDoc &doc, int position);
// The places of the body text's pictures (a picture that is a picture of the file: not a shape, a text box, a chart, or a group), in the order of the text.
QVector<int> picturePositions(const WriteDoc &doc);
// Compress Pictures: each picture at `positions` is made no more detailed than `ppi` pixels to the inch of the size it is shown at, and kept only when that makes it smaller; with
// `deleteCropped`, the parts a picture is cut away from are taken out of the file's picture and the cut is gone, the frame and what shows staying as they were. All is one step that
// undoes. The picture's own file part is let go of at save when nothing else names it. Returns the bytes saved (0 when nothing was smaller; no step is made then).
qint64 compressPictures(WriteDoc &doc, double ppi, const QVector<int> &positions, bool deleteCropped);

// ---- Shapes and text boxes

// How a new shape looks: a fill (none when empty) and a line (none when its weight is 0), as RRGGBB, with the line's weight in points and its dash (the drawing format's name:
// solid, dash, sysDot, and the rest).
struct ShapeLook {
    QString fill = QStringLiteral("3B78B8"), line = QStringLiteral("27547F"), dash = QStringLiteral("solid");
    double weight = 1.5;
};
// The names of the shapes Insert offers (the drawing format's preset names), in the groups of the gallery.
struct ShapeGroup {
    QString name;
    QStringList presets;
};
const QVector<ShapeGroup> &shapeGallery();
// A shape of this kind (a preset name from shapeGallery) at the cursor, in place of a selection, set at the cursor with the text going round it (or as `wrap` says), as one step
// that undoes. `size` (points; empty for the usual) is held to the least and most a picture's is. Refused outside the body text. A line or an arrow is as wide as `size` and
// as high as it says (0 for a flat one).
PictureResult insertShape(WriteDoc &doc, QTextCursor &c, const QString &preset, const QSizeF &size = QSizeF(), const ShapeLook &look = ShapeLook(), PictureWrap wrap = PictureWrap::Square);
// A shape drawn by dragging from one point of the page to another (points): the two corners it is made between (a rectangle's, put in order), or, for a line, its two ends; with
// `keepProportions` (Shift) a shape is as wide as it is tall and a line goes along a multiple of 45 degrees. A drag of less than 4 points either way gives the shape's usual size from `from`.
std::pair<QPointF, QPointF> shapeDragEnds(const QString &preset, const QPointF &from, const QPointF &to, bool keepProportions);
// The shape put in at the cursor, set at the place shapeDragEnds gives (on the page of its paragraph), floating in front of the text unless `wrap` says another way, as one step that undoes.
PictureResult insertShapeAt(WriteDoc &doc, QTextCursor &c, const QString &preset, const QPointF &from, const QPointF &to, bool keepProportions = false, PictureWrap wrap = PictureWrap::InFrontOfText);
// For the tests: while on, setting a shape's place after insertShapeAt put it in fails, to see that the shape is taken out again.
void failShapePlaceForTests(bool on);

// An icon of the shared set (a name of jp::iconCatalog, such as "heart") at the cursor, in place of a selection: one shape, `size` points square (an inch at first) and drawn in the color
// scheme's main color, whose lines thicken and thin when its size is changed (setPictureSize, setPictureBox). It is a shape of the drawing format like any other (saved to `.docx` and `.odt`
// and read again), set at the cursor with the text going round it (or as `wrap` says), and one step that undoes. Unreadable for a name that is no icon; Refused as for a shape.
PictureResult insertIcon(WriteDoc &doc, QTextCursor &c, const QString &name, double size = 72, PictureWrap wrap = PictureWrap::Square);

// A text box design: how it looks and what it holds at first (paragraphs of a size, a style, and a color of their own).
struct TextBoxDesign {
    QString id;                 // a short name that does not change ("simple", "sidebar", "quote")
    QString name;               // as shown
    QSizeF size{216, 72};
    ShapeLook look;
    PictureWrap wrap = PictureWrap::Square;
    QMarginsF insets{7.2, 3.6, 7.2, 3.6};
    // The text: each paragraph's words, its size in points, whether it is italic or bold, its color, and how it is aligned (left, center, right).
    struct Paragraph {
        QString text;
        double size = 11;
        bool bold = false, italic = false;
        QString color;
        QString align = QStringLiteral("left");
    };
    QVector<Paragraph> paragraphs;
    // Where the text sits in the box: top, center, or bottom.
    QString anchor = QStringLiteral("t");
    bool vertical = false;      // the text runs top to bottom
};
const QVector<TextBoxDesign> &textBoxDesigns();
// A text box of the design at the cursor, one step that undoes (PictureResult as above); a design of the simple kind with `design.paragraphs` empty holds one empty paragraph.
PictureResult insertTextBox(WriteDoc &doc, QTextCursor &c, const TextBoxDesign &design);
// Linked text boxes (the words of the first box go on in each box that follows it): whether the box at `from` can be followed by the one at `to` (an empty box that is in no chain, and the
// first is the end of its chain), makes it so (one step), whether a box is in a chain and whether another follows it, and Break Link: the boxes after this one become boxes of their own, empty
// (one step; false if none follows).
bool canLinkTextBoxFrom(const WriteDoc &doc, int position);   // a text box that a box can follow: it is in no chain, or is the last of one
bool canLinkTextBoxes(const WriteDoc &doc, int from, int to);
PictureResult linkTextBoxes(WriteDoc &doc, int from, int to);
bool isLinkedTextBox(const WriteDoc &doc, int position);
bool hasLinkedTextBoxAfter(const WriteDoc &doc, int position);
bool unlinkTextBox(WriteDoc &doc, int position);
// Text boxes pasted or duplicated into the text (the drawings between the places `from` and `to`) are given chain numbers of their own, so that a pasted chain is a chain of its own and does not
// take the members of the one it was copied from, and a pasted member whose first box was not pasted with it is a box of its own. Part of the edit that pasted them.
void renumberPastedTextBoxChains(WriteDoc &doc, int from, int to);
// The words of a text box, a paragraph each, or none when what it holds is more than words (a table, a picture, a field, a list): then it can't be edited as words.
std::optional<QStringList> textBoxText(const WriteDoc &doc, int position);
// Puts these paragraphs in the text box: each has the first paragraph's alignment and the first run's look. False when the box holds more than words, or the words are already so.
bool setTextBoxText(WriteDoc &doc, int position, const QStringList &paragraphs);
// The words of a text box or shape as a story to be typed in (with every look the words have: the runs' fonts, sizes, colors, and the paragraphs' alignments and lists), or none for an object
// that takes no words (a picture, a line, a group, text art). A shape with no text box yet has an empty story, its words centered as a shape's are. Written back with setTextBoxStory: one step
// that undoes (false when the words are as they were, or are past the limits).
// (`tooBig`, if given, says a box refused for holding more paragraphs than the editor takes; `refused`, a story that is not put in for want of room or for a reason the document gave: the person is told.)
std::unique_ptr<QTextDocument> textBoxStory(WriteDoc &doc, int position, bool *tooBig = nullptr);
bool setTextBoxStory(WriteDoc &doc, int position, QTextDocument *story, bool *refused = nullptr);
// How the object sets its words: where they sit up and down ("t", "ctr", or "b"), whether the lines break at its width, whether they shrink to fit, and the room kept round them, in points.
struct TextBoxLook {
    QString anchor = QStringLiteral("t");
    bool wrap = true, autofit = false;
    QMarginsF insets = QMarginsF(7.2, 3.6, 7.2, 3.6);
};
TextBoxLook textBoxLook(const WriteDoc &doc, int position);

// The yellow handles of a shape (a rounded rectangle's corner, an arrow's head, a star's points, a callout's tail): each is the number of the adjustment it sets and where it sits, in points
// from the frame's top left corner (before the turn). None for a shape with no adjustments, a shape that is flipped, a picture, or any other object.
struct ShapeHandle {
    int index = 0;
    QPointF at;
};
QVector<ShapeHandle> shapeAdjustHandles(const WriteDoc &doc, int position);
// Moves handle `index` to `at` (the frame's own points, before the turn; the handle stays inside what it may reach), written as the shape's adjustment values; one step. False where there is
// no such handle or it does not move.
bool setShapeAdjustment(WriteDoc &doc, int position, int index, const QPointF &at);

// Edit Points: the points of a shape's outline, in the frame's points from its top left corner (before the turn): the corners of a freeform, or the points a preset's outline is drawn through (a
// curve's point is where it ends). Empty for anything that is not a shape with one outline, a shape that is flipped, or one with more than 2,000 points.
QVector<QPointF> shapePoints(const WriteDoc &doc, int position);
// Why Edit Points is not offered for the shape at a place (a shape drawn in more than one part, a flipped one, or one with too many points), or empty when it is or the object is no shape. The
// caps: a drawing of more than 256 KB is never parsed for its points or handles, and a shape of more than 2,000 points is not offered Edit Points.
QString editPointsProblem(const WriteDoc &doc, int position);
// How many times a drawing was parsed for its handles or points; the tests count these.
quint64 shapeParseCount();
// Whether the shape's outline is already a freeform's (a file's custom geometry, or one Edit Points made).
bool isFreeformShape(const WriteDoc &doc, int position);
// Moves point `index` to `to` (its curve handles with it); a preset becomes a freeform with the first change. Delete takes a point away (a shape keeps at least three); Insert puts a point on the
// outline nearest `near`, within `tolerance` points of it, and `inserted` gets its number. Each is one step; false where nothing is done.
bool moveShapePoint(WriteDoc &doc, int position, int index, const QPointF &to);
bool deleteShapePoint(WriteDoc &doc, int position, int index);
bool insertShapePoint(WriteDoc &doc, int position, const QPointF &near, double tolerance, int *inserted = nullptr);

// The effects a shape or text box has: a shadow, a glow, a reflection, soft edges, and a bevel, each put in without taking away the others (and the shape's other 3-D settings stay); None takes off
// these five. One step each; false where nothing changes or the object is no shape. shapeEffectAt gives the first it finds.
enum class ShapeEffect { None, Shadow, Glow, Reflection, SoftEdges, Bevel };
ShapeEffect shapeEffectAt(const WriteDoc &doc, int position);
// Whether the shape has that effect (it may have several: setShapeEffect puts one in and leaves the others).
bool shapeHasEffect(const WriteDoc &doc, int position, ShapeEffect effect);
bool setShapeEffect(WriteDoc &doc, int position, ShapeEffect effect);
// Distribute: the floating objects at these places (each with its rectangle on the page, in points) are spread so that the gaps between them across (or down) are the same, the first and last
// where they are; at least three; one step. False where there are fewer, or the gaps are the same already.
bool distributeObjects(WriteDoc &doc, const QVector<std::pair<int, QRectF>> &objects, bool horizontal);

// Changes to the look of the shape or text box at `position`, each one step; false when it is no such object or the change makes no difference. A fill or a line given as empty
// is none; `weight` is in points (held to 0.25 to 100), and `dash` the drawing format's name for how the line is dashed.
bool setShapeFill(WriteDoc &doc, int position, const QString &rgb);
bool setShapeLine(WriteDoc &doc, int position, const QString &rgb, double weight, const QString &dash);
// Fills the shape or text box with a picture (the bytes of a picture file, as insertPicture takes them), stretched over it; one step. Refused for any other object.
PictureResult setShapePictureFill(WriteDoc &doc, int position, const QByteArray &bytes);
// Turns the object to this many degrees clockwise (any number: it is held to 0 up to 360).
bool setShapeRotation(WriteDoc &doc, int position, double degrees);
// Fills the shape with a gradient from one color to the other (RRGGBB), along an angle in degrees (0 across, from left to right, 90 down); one step.
bool setShapeGradient(WriteDoc &doc, int position, const QString &from, const QString &to, double angle);
// Cuts the picture to a shape of the gallery (its preset name; "rect" takes the cut off); the picture's frame and size stay. One step.
bool setPictureCropShape(WriteDoc &doc, int position, const QString &preset);
// Aspect Ratio: the part of the picture that shows is cut, from its middle, to the shape width : height, and the frame shrinks with it so that the picture keeps its scale. One step;
// false when it makes no difference or nothing would be left.
bool setPictureCropAspect(WriteDoc &doc, int position, double width, double height);
// Fill (the frame stays and the picture covers all of it, cut evenly on the sides that reach past it) and Fit (the whole picture shows in the frame, with room round it, or the frame
// shrinks to its shape where the room would be more than the picture's own size). One step; false when it makes no difference or the picture's size can't be read.
bool setPictureCropFill(WriteDoc &doc, int position, bool fill);
// Where a floating object is lined up: across to the left, middle, or right, or down to the top, middle, or bottom, of the margins or the page (the other direction is kept). An object
// that is in line with the text becomes a floating one (in front of the text).
enum class ObjectAlign { Left, Center, Right, Top, Middle, Bottom };
bool alignObject(WriteDoc &doc, int position, ObjectAlign align, bool toPage);
// Another shape of the gallery in its place, its size and look kept.
bool setShapePreset(WriteDoc &doc, int position, const QString &preset);

// ---- Connectors

// A connector of this kind (straightConnector1, bentConnector3, or curvedConnector3) from the object at `from` to the one at `to`, each where `fromRect` and `toRect` say (points, on the
// page), glued: it runs between the nearest connection sites of the two (an ellipse has eight, any other shape four) and is saved with the file format's connection elements, so that
// other programs keep it joined too. One step that undoes; `position` gets its place in the text. Refused for a line, a chart, an object in line with the text, or the same object twice.
PictureResult connectObjects(WriteDoc &doc, int from, const QRectF &fromRect, int to, const QRectF &toRect, const QString &preset = QStringLiteral("straightConnector1"), int *position = nullptr);
// The objects at these places (each with its new rectangle, in points on the page) have moved or changed size: every connector glued to one of them runs again between the sites it was
// glued to. `rectOf` gives the rectangle of any other object (false when it has none). One step; how many connectors changed.
int rerouteConnectors(WriteDoc &doc, const QVector<std::pair<int, QRectF>> &moved, const std::function<bool(int, QRectF *)> &rectOf);
// Whether the object at `position` is a connector that is glued to something.
bool isConnectorGlued(const WriteDoc &doc, int position);

// ---- Text Art

// Words bent into a shape (a wave, an arch, a circle, and the rest), set in letters with a fill and an outline of their own, as a text box of the drawing format that names the warp.
// `styleId` is one of the styles of the shared Text Art gallery (the plain one when empty or unknown); `warp` is one of the shared text art transforms (its style's own when empty).
struct TextArtSpec {
    QString text;     // a line a paragraph
    QString styleId;
    QString warp;
    double size = 48; // points, of the letters before they are stretched to the box
};
// A Text Art at the cursor in place of a selection, set at the cursor with the text going round it (or as `wrap` says), one step that undoes.
PictureResult insertTextArt(WriteDoc &doc, QTextCursor &c, const TextArtSpec &spec = TextArtSpec(), PictureWrap wrap = PictureWrap::Square);
// The words and the warp of the text art at `position` (the style is not kept in a file); none for any other object.
std::optional<TextArtSpec> textArtAt(const WriteDoc &doc, int position);
// The style (its fill, outline, and font) of the text art at `position` is another of the gallery's, the words and the warp staying; and its warp is another of the shared transforms
// ("plain" for none), the look staying. One step each; false when the object is no text art, the name is unknown, or nothing would change.
bool setTextArtStyle(WriteDoc &doc, int position, const QString &styleId);
bool setTextArtWarp(WriteDoc &doc, int position, const QString &transform);

// ---- Groups

// A drawing canvas at the cursor: a frame (a group that holds one empty rectangle with a pale dashed line, set above and below the text) for shapes and pictures that belong together;
// select it and the shapes, and Group joins them into it. One step that undoes; refused outside the body text.
PictureResult insertDrawingCanvas(WriteDoc &doc, QTextCursor &c, const QSizeF &size = QSizeF(432, 216));
// Whether the drawn object at `position` is a group (a set of shapes, text boxes, lines, and pictures that move and size as one), and whether it could be a member of one: set at a
// place (not in line with the text), and a plain shape, text box, line, or picture (a chart, or a group, is not).
bool isGroupAt(const WriteDoc &doc, int position);
bool canGroupObject(const WriteDoc &doc, int position);
// Joins the objects at `positions` (two or more, each where `pageRects` says, in points on the page they are on: the view knows) into one group, set in front of the text at the
// corner of their common rectangle, as one step. The group takes the first object's place in the text; the others are taken out. False (nothing done) where one cannot be a member.
bool groupObjects(WriteDoc &doc, const QVector<int> &positions, const QVector<QRectF> &pageRects, int *groupPosition = nullptr);
// Takes the group at `position` apart: each member is an object of its own at the place it had on the page (the group's top left corner is `pageTopLeft`), as one step. A turned or
// flipped group, and one with a group inside it, is not taken apart. `positions` gets the members' places in the text.
bool ungroupObject(WriteDoc &doc, int position, const QPointF &pageTopLeft, QVector<int> *positions = nullptr);

// ---- Charts

// What a chart is made of and what Insert offers are the shared ones (sheet/chartops.h): the kinds, the data, and what a chart says of itself.
using office::sheet::ChartAxisKind;
using office::sheet::ChartData;
using office::sheet::ChartForm;
using office::sheet::ChartInfo;
using office::sheet::chartForms;
using office::sheet::kMostChartCategories;
using office::sheet::kMostChartSeries;
using office::sheet::sampleChartData;
ChartInfo chartAt(const WriteDoc &doc, int position);
// The workbook kept beside the chart at `position` (the part its c:externalData names); empty for a chart with none. And the chart's numbers changed when they are cells of that workbook (the cells
// and the values the chart stores, from the grid; one step that undoes; false when it can't be done or changes nothing).
QByteArray chartWorkbookAt(const WriteDoc &doc, int position);
bool setChartWorkbookData(WriteDoc &doc, int position, const ChartData &data);
// A chart of the kind at the cursor (in line with the text), in place of a selection, one step that undoes; `size` (points; empty for the usual 360 by 216) is held to the least and most
// a picture's is. Unreadable for data that is empty, or has more than the most categories or series, or numbers that are not numbers.
PictureResult insertChart(WriteDoc &doc, QTextCursor &c, ChartForm form, const ChartData &data = sampleChartData(), const QSizeF &size = QSizeF());
// Changes to the chart at `position`, one step each; false when it is no chart that can be changed (a chart the program does not write whole is refused, with a notice) or the change makes
// no difference. Its look is kept: its title, legend, colors, and page.
bool setChartForm(WriteDoc &doc, int position, ChartForm form);
bool setChartTitle(WriteDoc &doc, int position, bool shown, const QString &text);
// `legendPosition` is "r", "l", "t", or "b"; empty takes the legend off.
bool setChartLegend(WriteDoc &doc, int position, const QString &legendPosition);
bool setChartDataLabels(WriteDoc &doc, int position, bool shown);
bool setChartData(WriteDoc &doc, int position, const ChartData &data);
// The axes (false for a chart that has none, such as a pie, or when nothing changes): an axis shown or not, the gridlines (major and minor, as the axis has them), the words of an axis's title
// (empty takes it off), and the style (1 to 48).
bool setChartAxis(WriteDoc &doc, int position, ChartAxisKind axis, bool shown);
bool setChartGridlines(WriteDoc &doc, int position, ChartAxisKind axis, bool major, bool minor);
bool setChartAxisTitle(WriteDoc &doc, int position, ChartAxisKind axis, const QString &text);
bool setChartStyle(WriteDoc &doc, int position, int style);
// The chart at `position` as the shared chart editor reads it (false for no chart that can be read), the theme it is colored by, and a change of the model as one step that undoes: `edit` is given the
// chart; false when the chart can't be changed (one the program does not write whole is refused, with a notice) or the edit made no difference.
bool readChartAt(const WriteDoc &doc, int position, office::Chart *chart);
office::Theme chartThemeOf(const WriteDoc &doc);
bool changeChartAt(WriteDoc &doc, int position, const std::function<void(office::Chart *)> &edit);

} // namespace jp::write
