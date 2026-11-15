#pragma once
// The Page Number gallery every app with printed pages shares: the designs (data), the picture each one is drawn as in the menu, and the facts an app needs to put one into its own
// model: what its words are, how big and where its box goes, and what its rule is. An app supplies only how a design is placed in its own file; the designs come from here.
//
//   Top of Page and Bottom of Page: a plain number at the left, center, or right; "Page X of Y"; a bold large number; a number with a rule above or below; a number in a circle, a
//     rounded box, or on a tab or bar in the theme's accent; "Page X" with a thin line.
//   Page Margins: a number in the left or right margin, upright or turned, in a circle, on a tab, or on a vertical bar.
//   Current Position: the plain, X of Y, bold, and circled looks put in at the cursor.

#include "app/widgets.h"

#include <QColor>
#include <QMarginsF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QVector>
#include <functional>
#include <optional>

class QPainter;

namespace jp {

enum class PageNumberArea { Top, Bottom, Margin, Cursor };
enum class PageNumberLook { Plain, OfCount, Bold, RuleAbove, RuleBelow, Circle, RoundedBox, Tab, Bar, PageLine };
// Across the page for the top and the bottom (and a tab's edge); the margin it is in for a margin design.
enum class PageNumberSide { Left, Center, Right };

struct PageNumberDesign {
    QString id;      // "top.plain.left", "bottom.circle.right", "margin.left.bar", "cursor.bold"
    QString label;   // translated
    PageNumberArea area = PageNumberArea::Top;
    PageNumberLook look = PageNumberLook::Plain;
    PageNumberSide side = PageNumberSide::Center;
    bool turned = false;   // a margin design whose number is turned a quarter, reading upward
};

// Every design, in the order the gallery shows them: the top, the bottom, the margins, then the current position.
QVector<PageNumberDesign> pageNumberDesigns();
std::optional<PageNumberDesign> pageNumberDesignById(const QString &id);
// The name the gallery gives a group of designs.
QString pageNumberAreaName(PageNumberArea area);

// What a design says, in pieces: words, the page's number, or the document's page count.
struct PageNumberPiece {
    enum Kind { Text, Page, Count } kind = Text;
    QString text;
};
QVector<PageNumberPiece> pageNumberPieces(const PageNumberDesign &d);

// How the number is set: its size in points (0: the text's own), whether it is bold, and whether it is white on the accent (a design with a box that is filled).
struct PageNumberType {
    double size = 0;
    bool bold = false;
    bool onAccent = false;
};
PageNumberType pageNumberType(const PageNumberDesign &d);

// The rule of a design that has one, as a side of its paragraph: none, above, or below the number ("Page X" with a thin line has one below).
enum class PageNumberRule { None, Above, Below };
PageNumberRule pageNumberRule(const PageNumberDesign &d);

// A design set in a box of its own (a circle, a rounded box, a tab or a bar, and every design of the margins): the shape's preset ("ellipse", "roundRect", or "rect") and its place
// on a page of this size with these margins, in points. The text is turned in a box of a turned design. False (nothing set) for a design that is only text.
struct PageNumberBox {
    QString preset;
    QRectF rect;
    bool turned = false;
};
bool pageNumberHasBox(const PageNumberDesign &d);
PageNumberBox pageNumberBox(const PageNumberDesign &d, const QSizeF &page, const QMarginsF &margins);

// Whether a design can be said in the left, center, or right part of a printed header or footer (text with the page's number and count, and a size and bold): the designs without a box,
// a rule, or a margin. The apps that keep no drawn header (JeffCrunch) offer these only.
bool pageNumberIsTextOnly(const PageNumberDesign &d);

// A design drawn as a small page with the number where it goes, in the accent given.
void paintPageNumberDesign(QPainter *p, const QRectF &rc, const PageNumberDesign &d, const QColor &accent);

// The gallery's items: the designs `accepts` takes (all, when none is given), grouped by area, each drawn in `accent`. The item's id is the design's.
QVector<GalleryItem> pageNumberGalleryItems(const QColor &accent, const std::function<bool(const PageNumberDesign &)> &accepts = {});
// The size each picture is drawn at.
QSize pageNumberItemSize();

} // namespace jp
