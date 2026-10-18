#pragma once
// What a drawn object of a file (a VML or DrawingML shape, read as wml::Shape) becomes beyond a plain box, picture, or line: the cuts, clear color, and
// border of a picture, lettering on a path, a horizontal rule, and a shape of the drawing format's own kinds.

#include "core/items.h"
#include "wml/wml.h"

#include <QFont>

namespace jp::write {

// The most lettering on a path that a document draws, and the most of its characters (each is outlined letter by letter, and stroked), and the most a lettering's
// frame measures across and down, in points: what a file asks for past these is left out, or cut to the size.
constexpr int kMostLetteringShapes = 100;
constexpr int kMostLetteringCharacters = 1000;
constexpr double kMostLetteringSide = 2000;

// Sets the picture's place in its frame (the whole picture, outside the frame by what the shape cuts), its clear color, and its border from the shape. The
// picture's frame (rect) is set already.
void dressPicture(PictureItem &pic, const office::wml::Shape &s);
// The lines that are a picture's border where it is not its outline: one for each side the file sets, from the picture's corners (none when all four are set: that is the
// outline).
QVector<std::shared_ptr<LineItem>> borderLines(const office::wml::Shape &s);
// The Text Art of a shape that is lettering on a path (its text set to fill its frame, in the shape's colors and shadow), or none for a shape that is not.
ItemPtr letteringItem(const office::wml::Shape &s);
// The shape of a horizontal rule set in a line of the width given, with the room beside it that sets it across the line as it is aligned, or none for a shape
// that is not a rule. A rule that is alone in its paragraph (`alone`) has room above and below it that make its line as tall as a line of text in the font, with the
// rule in the middle of it.
ItemPtr ruleItem(const office::wml::Shape &s, double textWidth, const QFont &font, bool alone);
// The width of the text of a section, in points (a column's, where it has several: equal ones are taken).
double sectionTextWidth(const office::wml::SectionProps &section);
// Whether a paragraph's contents are drawn objects and spaces only (a paragraph of more than a few hundred pieces is taken to hold more).
bool holdsOnlyObjects(const QVector<office::wml::Inline> &content);
// Whether a picture's other form (the target of its r:pict) is one that is not drawn: a compressed picture (.pcz) of the kind the original program keeps for itself
// shows its picture as missing.
bool isUndrawnPictureTarget(const QString &target);
// The PNG of the mark of a picture that cannot be shown (one small picture, made once), and the picture's look for it: a white frame with an outline, the mark at its corner.
const QByteArray &missingPictureMarkPng();
void showAsMissing(PictureItem &pic);
// The preset a shape is drawn as: its own kind's (an arc, a bracket) where the drawing format numbers it, else a rectangle or an ellipse.
QString presetOf(const office::wml::Shape &s);
// Makes the item the shape of the shape's own kind (see presetOf). An arc of the drawing format is the quarter of an ellipse twice its frame's size, from the
// frame's top left corner to its bottom right, as the format's own arc is drawn: the item's path is that.
void shapeOfKind(ShapeItem &item, const office::wml::Shape &s);

} // namespace jp::write
