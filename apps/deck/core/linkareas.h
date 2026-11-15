#pragma once
// Where the links of a slide are: each object with a link (its box), and each line of linked text in a text box (a shape's linked text is the shape's whole box). Both the show
// (what a click follows) and the PDF (what a click on a page reaches) take their areas from here.

#include "core/document.h"
#include "render/renderer.h"

#include <QRectF>
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
QVector<LinkArea> linkAreas(const Document &d, LayoutCache &cache, int slide, const RenderOptions &opt, bool withHover = false);

// How many times areas have been worked out on this thread since the program began (a count for tests of cost).
qint64 linkAreaRunsForTest();

} // namespace jp::deck
