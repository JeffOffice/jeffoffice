#pragma once
// Where the links of a slide are: each object with a link (its box), and each line of linked text in a text box (a shape's linked text is the shape's whole box). Both the show
// (what a click follows) and the PDF (what a click on a page reaches) take their areas from here.

#include "core/document.h"
#include "render/renderer.h"

#include <QRectF>
#include <QSet>
#include <QString>
#include <QVector>

namespace jp::deck {

struct LinkArea {
    QRectF rect;           // in the slide's own units, from its top left
    QString link;          // as the model holds it (see office/hyperlink.h)
    const Item *item = nullptr;   // the object it is on
    bool hover = false;           // the link is the object's mouse-over action, not a click's
};

// The areas of slide `slide`, the bottom ones first. Hidden objects have none. The text is laid out with `opt`, the options the slide is drawn with, so that the layout the
// drawing made is the one used (none is made again).
// With `withHover` the objects' mouse-over actions come too, marked, after the click areas of the object they are on.
// With `triggers`, the objects whose numbers (Item::sourceId) are in it have an area too, with the link triggerLink(number): a click on one starts the effects that wait for it.
QVector<LinkArea> linkAreas(const Document &d, LayoutCache &cache, int slide, const RenderOptions &opt, bool withHover = false, const QSet<int> &triggers = QSet<int>());

// The link an object's trigger area has, and the number in such a link (0 for any other link).
QString triggerLink(int shape);
int triggerOfLink(const QString &link);

// Which of the objects whose numbers are in `triggers` have a click area on the slide: not hidden, with a box that is not empty, on the slide, and with no link of its own (where both are, the link wins the click).
QSet<int> clickableTriggerShapes(const Document &d, LayoutCache &cache, int slide, const QSet<int> &triggers);

// How many times areas have been worked out on this thread since the program began (a count for tests of cost).
qint64 linkAreaRunsForTest();

} // namespace jp::deck
