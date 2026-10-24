#pragma once
// The objects of a sheet drawn over its cells (charts, pictures, shapes): where each lies from its anchor and the rows and columns the sheet has now, a picture of each kept at the
// size it is shown at, and the one under the mouse chosen. What an object looks like is not this layer's: each object brings a function that draws it at a size (the window's makes
// a chart from the cells it reads, or a picture or shape through the drawing code the presentations use), and the layer calls it when the object has no picture of the size, or its
// cells changed (invalidate). Drawing a frame costs the objects on the screen: their places are kept until the sizes of rows and columns change, a picture is drawn from the
// cache, and at most kMaxRendersPerPaint pictures are made during a paint (the rest follow in the time between events, or prepareAll()).
//
// Bounded: the objects (kMaxObjects), the bytes of pictures held (kCacheBytes: the least recently shown go first), the pixels of one picture (kMaxImagePixels: a larger object is
// drawn from a picture of fewer pixels, scaled up), and the renders in a paint. The layer holds no cells and no file.

#include "grid/layout.h"
#include "grid/overlay.h"
#include "sheet/drawing.h"

#include <QImage>
#include <QObject>
#include <QTimer>

#include <functional>
#include <vector>

namespace jp::grid {

// Where an object is, in the grid's units (pixels at 100%) from the first row and column: its anchor on the rows and columns now.
struct ObjectBox {
    double x = 0, y = 0, w = 0, h = 0;
    bool isEmpty() const { return w < 0.5 || h < 0.5; }
    bool isNothing() const { return w < 0.5 && h < 0.5; }
};

// The box of an anchor. A two-cell anchor runs from its first place to its second (a hidden row or column between them has no size, so an object whose rows are all hidden is empty);
// a one-cell anchor has the size it says; an absolute one is where it says whatever the rows and columns are. A place in a cell is no further into it than the cell is wide or high.
ObjectBox anchorBox(const jp::office::sheet::DrawingAnchor &anchor, const Axis &rows, const Axis &cols);

// What an object is asked to draw: an image of `pixels`, for a grid at `zoom`. The object itself, `units` big at 100% (the grid's units), is in `inner` of the image (in its pixels): the image
// is larger by the object's margin all round, for a stroke that is wider than the object's edge, so that nothing is cut off.
struct RenderRequest {
    QSize pixels;
    double zoom = 1;
    QSizeF units;
    QRectF inner;
};

struct LayerObject {
    int id = 0;
    jp::office::sheet::DrawingAnchor anchor;
    jp::office::sheet::ObjectKind kind = jp::office::sheet::ObjectKind::Shape;
    QString name;
    // A line has no width or no height: it is still drawn (an object that is empty both ways is not). `margin` (the grid's units at 100%) is drawn around the object's box.
    bool thin = false;
    double margin = 0;
    // Draws the object. Called on the window's thread. An empty image is a failure (the object is drawn as an outline).
    std::function<QImage(const RenderRequest &)> render;
};

class ObjectLayer : public QObject, public GridOverlay {
    Q_OBJECT
public:
    static constexpr int kMaxObjects = jp::office::sheet::kMaxWorkbookObjects;
    static constexpr int kMaxRendersPerPaint = 6;
    static constexpr qint64 kCacheBytes = qint64(192) << 20;
    static constexpr qint64 kMaxImagePixels = 16000000;

    explicit ObjectLayer(QObject *parent = nullptr);
    ~ObjectLayer() override;

    // Gives the layer its objects (back to front); the pictures, the selection, and what was deleted are forgotten. More than kMaxObjects are left out.
    void setObjects(std::vector<LayerObject> objects);
    int count() const { return int(m_entries.size()); }
    const LayerObject &object(int index) const { return m_entries[size_t(index)].object; }
    // The layout the layer places by when it draws between paints (the view's; it must outlive the layer's use of it).
    void attach(const GridLayout *layout) { m_layout = layout; }
    // The object fills the part of the grid that scrolls (a chart sheet): the first object that is not deleted, at the size of the window.
    void setFillsView(bool fills);
    bool fillsView() const { return m_fills; }

    // An object is taken away (a person pressed Delete) or brought back (undone): it is not drawn or hit while it is.
    void setDeleted(int id, bool deleted);
    bool isDeleted(int id) const;
    // The object's anchor changed (cells were inserted or deleted before it): it is placed again.
    void setAnchor(int id, const jp::office::sheet::DrawingAnchor &anchor);
    // Its cells changed: the picture is made again, a moment later (the old one is drawn until it is, and a calculation that tells many changes makes one picture). invalidateAll for every object.
    void invalidate(int id);
    void invalidateAll();
    // The object is picked, or none (-1).
    int selectedId() const { return m_selected; }
    void select(int id);
    // What happens when Delete is pressed with an object picked; unset, the key is not taken.
    void setDeleteHook(std::function<void(int id)> hook) { m_deleteHook = std::move(hook); }

    // Where the object is on the screen now, in the part of the grid it is in at the point `at` of the viewport... or the part given: empty if it is not shown there.
    QRect deviceRect(int id, Pane pane) const;
    // The object at a point of the viewport (the topmost), or -1.
    int objectAt(const QPoint &point) const;

    // Makes every picture that the screen needs now, however many (for a test, or a window that is saved as a picture); otherwise they are made in the time between events.
    void prepareAll();
    // Pictures the screen needs that are not made (or are out of date): a window waits for 0 before it is shot.
    int pendingRenders() const;

    // What was done, counted (a test reads these; none is a time).
    struct Stats {
        qint64 renders = 0;        // calls of the objects' draw functions
        qint64 drawn = 0;          // pictures put on the screen
        qint64 placed = 0;         // places worked out from anchors
        qint64 evicted = 0;        // pictures let go of for the budget
        qint64 placeholders = 0;   // objects drawn as an outline for want of a picture
    };
    const Stats &stats() const { return m_stats; }
    qint64 cacheBytes() const { return m_bytes; }

    void paint(QPainter *painter, const GridLayout &layout, Pane pane, const QRect &clip) override;
    bool press(const QPoint &point, const GridLayout &layout, Qt::KeyboardModifiers modifiers) override;
    bool key(QKeyEvent *event) override;
    void layoutChanged() override;

Q_SIGNALS:
    void selectionChanged(int id);

private:
    struct Entry {
        LayerObject object;
        mutable ObjectBox box;     // where the anchor puts it now (worked out by ensureBoxes)
        bool deleted = false;
        QImage image;
        QSize requested;           // the size the picture was made for (device pixels)
        double zoom = 0;
        bool stale = false;        // its cells changed after the picture was made
        bool failed = false;       // the draw function gave no picture for this size
        quint64 lastUsed = 0;
    };
    void ensureBoxes(const GridLayout &layout) const;
    QRect rectIn(const GridLayout &layout, Pane pane, const Entry &e) const;
    QSize sizeWanted(const QRect &rect, double dpr) const;
    bool missing(const Entry &e, const QSize &wanted, double zoom) const;
    void schedule(int millis);
    // Makes the pictures the screen needs (those with none first); `all` leaves out the time limit of a step. True if some are left.
    bool makePictures(bool all);
    void render(Entry &e, const QSize &wanted, double zoom, const RenderRequest &request);
    void evict();
    void step();
    // The rectangle drawn for the object (its box and the margin), and what it asks of the object's draw function when the picture is `wanted` pixels.
    QRect drawRect(const QRect &box, const Entry &e, double zoom) const;
    RenderRequest requestFor(const Entry &e, const QRect &box, const QSize &wanted, double zoom) const;
    static QSize capped(const QSize &wanted);

    std::vector<Entry> m_entries;
    const GridLayout *m_layout = nullptr;
    mutable std::vector<qint64> m_signature;   // of the rows and columns the boxes were worked out for
    mutable bool m_boxesValid = false;
    bool m_fills = false;
    int m_selected = -1;
    quint64 m_frame = 0;
    qint64 m_bytes = 0;
    double m_dpr = 1.0;
    QTimer m_timer;
    mutable Stats m_stats;
    std::function<void(int)> m_deleteHook;
};

} // namespace jp::grid
