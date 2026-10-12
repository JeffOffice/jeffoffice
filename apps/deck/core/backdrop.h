#pragma once
// What is behind a thing on a page: the colors a slide, a layout, or a slide
// master shows at a point, built from its background, the shapes of the
// masters under it, and the objects of the page below the one asked about.
// A change of theme uses it to keep text readable on what it stands on (a
// table cell or shape with a color of its own, a picture, or the slide's own
// background), not only on the slide's background.

#include "core/deck.h"

#include <QHash>
#include <QTransform>
#include <QVector>

namespace jp::deck {

// What shows at a point.
struct Behind {
    QVector<QColor> colors;   // each color that can show there (every stop of a gradient)
    bool picture = false;     // a picture or texture is there: its colors aren't told (`colors` is empty)
    bool known = true;        // false when the page holds too many objects to look through
};

class Backdrop {
public:
    // The backdrop of a page (its colors as `d` resolves them now): a slide, with `base` the layout it stands on, or a layout or slide
    // master, with `base` the page itself.
    Backdrop(const Document &d, const PageBase &page, const MasterPage *base);

    // The colors at `p` (page coordinates) behind `above`, an object of the page (null: above everything the page has),
    // and under `own`, a fill laid over them (the object's own, a table cell's), if any.
    Behind at(const Item *above, const QPointF &p, const Fill *own = nullptr) const;

    // Whether a fill paints anything.
    static bool paints(const Fill &f);

private:
    struct Layer {
        const Item *item = nullptr;
        QTransform inverse;   // page to frame coordinates
        QRectF box;           // the frame's page bounds
        QRectF frame;         // 0, 0, width, height
    };
    // The colors a fill paints, as `d` resolves them; sets `picture` for a picture or texture.
    QVector<QColor> colorsOf(const Fill &f, bool *picture) const;
    const Fill *fillAt(const Layer &layer, const QPointF &local) const;
    bool covers(const Layer &layer, const QPointF &p, QPointF *local) const;

    const Document &m_d;
    Fill m_background;
    QVector<Layer> m_layers;
    QHash<const Item *, int> m_below;   // how many layers lie below each object of the page itself
    mutable qint64 m_work = 0;
};

} // namespace jp::deck
