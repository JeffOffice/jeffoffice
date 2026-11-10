#pragma once
// Pictures in the text: Insert Picture (from a file's bytes, or an image from the clipboard), and what is done to a picture once it is there: its size, how the text goes round it
// (in line with the text, square, tight, above and below, behind or in front of the text), its place, and its alt text. A picture is a character of the story that holds the
// drawing element of its run as XML; every command writes that element again and gives the character the form the pages show, in one change of the character's format, so each is
// one step that undoes and the file saved says what the pages show. Only pictures that are one plain drawing (a picture in line, or one anchored) can be changed; any other
// (a picture of the older drawing format, a group) is kept as it is.

#include "core/writedoc.h"

#include <QImage>
#include <QMarginsF>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QStringList>
#include <QVector>
#include <QTextCursor>
#include <functional>
#include <optional>
#include <utility>

namespace jp::write {

enum class PictureWrap { Inline, Square, Tight, TopAndBottom, BehindText, InFrontOfText };
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
};

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
PictureResult insertPicture(WriteDoc &doc, QTextCursor &c, const QByteArray &bytes, const QString &name = QString(), PictureWrap wrap = PictureWrap::Inline, const QString &alt = QString());
// The same for an image (from the clipboard), kept as PNG.
PictureResult insertPictureImage(WriteDoc &doc, QTextCursor &c, const QImage &image, PictureWrap wrap = PictureWrap::Inline);

// Changes to the picture at `position` of the body text; each is one step, and false (nothing done) when the character is no picture that can be changed or the change would
// make no difference. Sizes are in points and held to the least and most above.
bool setPictureSize(WriteDoc &doc, int position, double width, double height);
// The size and the place (the picture's top left corner on the page) in one step; with no place, only the size; with no size (0), only the place.
bool setPictureBox(WriteDoc &doc, int position, double width, double height, const std::optional<QPointF> &pageTopLeft, bool keepGlue = false);
bool setPictureWrap(WriteDoc &doc, int position, PictureWrap wrap);
// Moves the floating picture at `position` in the layers as `order` says, as one step that undoes (the pictures it passes are given heights to keep their order). False where it does
// not move (an inline picture, or one already there); `canOrderPicture` says whether it would.
bool setPictureOrder(WriteDoc &doc, int position, PictureOrder order);
bool canOrderPicture(WriteDoc &doc, int position, PictureOrder order);
bool setPictureAltText(WriteDoc &doc, int position, const QString &alt);

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
// Takes off the cut, the corrections, the recolor, and the border: the picture as it came in, at the scale it has (so its frame grows by what the cut took off).
bool resetPicture(WriteDoc &doc, int position);
// Another picture in the place of this one, in the same frame width, with the same place, wrap, and alt text; its cuts and colors are not kept. The result is as insertPicture's.
PictureResult changePictureFile(WriteDoc &doc, int position, const QByteArray &bytes);

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

// The kinds of chart Insert offers (and a chart's kind, as nearly as these say it).
enum class ChartForm { Column, ColumnStacked, ColumnPercent, Bar, BarStacked, Line, LineMarkers, Pie, Doughnut, Area, AreaStacked, Scatter };
const QVector<ChartForm> &chartForms();
// What a chart plots: the words under the points (or a scatter's x numbers), the name of each series, and the numbers (a list for each series, a number for each category).
struct ChartData {
    QStringList categories, seriesNames;
    QVector<QVector<double>> values;
};
// The most categories and series a chart made here has.
constexpr int kMostChartCategories = 200, kMostChartSeries = 24;
// Three categories and two series, to begin with.
ChartData sampleChartData();
// What a chart of the text is: its kind, its title (its words; shown or not), its legend (shown, and where: "r", "l", "t", "b"), whether its points are labeled, and its data when the
// chart holds the numbers itself (a chart that reads cells of a workbook it is kept with can be changed in all but its data).
struct ChartInfo {
    bool valid = false;
    ChartForm form = ChartForm::Column;
    bool titleShown = false;
    QString title;
    bool legendShown = false;
    QString legendPosition;
    bool dataLabels = false;
    bool dataEditable = false;
    ChartData data;
    // The axes: whether each is shown (a pie has none), whether the value axis has major and minor gridlines and the category axis major ones, the words of each axis's title (empty for
    // none), and the style number (1 to 48).
    bool hasAxes = false, categoryAxisShown = false, valueAxisShown = false;
    bool valueGridlines = false, valueMinorGridlines = false, categoryGridlines = false;
    QString categoryAxisTitle, valueAxisTitle;
    int style = 2;
};
enum class ChartAxisKind { Category, Value };
ChartInfo chartAt(const WriteDoc &doc, int position);
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

} // namespace jp::write
