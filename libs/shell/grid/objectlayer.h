#pragma once
// The objects of a sheet drawn over its cells (charts, pictures, shapes): where each lies from its anchor and the rows and columns the sheet has now, a picture of each kept at the
// size it is shown at, and the one under the mouse chosen. What an object looks like is not this layer's: each object brings a function that draws it at a size (the window's makes a
// picture or shape through the drawing code the presentations use), or, for a chart, a function that takes what the picture needs on the window's thread (the cells it reads) and gives a
// job that makes the picture on a worker thread, from that alone. The layer asks for a picture when the object has none of the size, or its cells changed (invalidate).
//
// A paint draws and makes nothing: it puts on the screen the pictures there are (an old one scaled while a new one is on its way, else an outline) and asks for the rest to be made in the
// time between events, a few at a time (at most kMaxJobs charts on the worker at once, and a slice of the others by the clock). prepareAll() makes every picture the screen needs, however
// long it takes (a test, a picture of the window).
//
// Bounded: the objects (kMaxObjects); the bytes of pictures (kCacheBytes, a hard limit: when what is on the screen would not fit, every picture is made at half the size (then a quarter)
// until it does, and the least recently shown are let go of past it), the pixels of one picture (kMaxImagePixels), the charts being made at once (kMaxJobs), and the work of a paint, which is
// a look at each object. The layer holds no cells and no file.

#include "grid/layout.h"
#include "grid/overlay.h"
#include "sheet/drawing.h"

#include <QImage>
#include <QObject>
#include <QThreadPool>
#include <QTimer>

#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>
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

// The place in a row or column that a position (the grid's units from the first) is in: the row or column, and how far into it in EMU. A position past the last is the end of it.
jp::office::sheet::AnchorPoint anchorPointAt(const Axis &rows, const Axis &cols, double x, double y);

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
    // A click picks the object only where it has ink (a filled or stroked place; a shape with no fill passes the click to the cells under it), not anywhere in its box.
    bool hitInk = false;
    // Draws the object on the window's thread. An empty image is a failure (the object is drawn as an outline).
    std::function<QImage(const RenderRequest &)> render;
    // Or in two parts: on the window's thread, takes what the picture needs and gives a job that makes it from that alone, on a worker thread (the job must not touch the window's data,
    // widgets, or pixmaps). Used instead of `render` when set. A null job is a failure.
    std::function<std::function<QImage()>(const RenderRequest &)> prepare;
    // Called once on the window's thread before the picture is first asked for: the object may settle what it knows (its margin, whether it is thin) and, if it has nothing to draw, say so
    // by clearing `render` and `prepare` (it is then drawn as nothing).
    std::function<void(LayerObject *)> settle;
};

class ObjectLayer : public QObject, public GridOverlay {
    Q_OBJECT
public:
    static constexpr int kMaxObjects = jp::office::sheet::kMaxWorkbookObjects;
    static constexpr int kMaxJobs = 2;
    static constexpr qint64 kCacheBytes = qint64(192) << 20;
    static constexpr qint64 kMaxImagePixels = 8000000;

    explicit ObjectLayer(QObject *parent = nullptr);
    ~ObjectLayer() override;

    // Gives the layer its objects (back to front); the pictures, the selection, and what was deleted are forgotten. More than kMaxObjects are left out.
    void setObjects(std::vector<LayerObject> objects);
    int count() const { return int(m_entries.size()); }
    const LayerObject &object(int index) const { return m_entries[size_t(index)].object; }
    // The layout the layer places by when it draws between paints (the view's; it must outlive the layer's use of it).
    void attach(const GridLayout *layout) { m_layout = layout; }
    // The object fills the part of the grid that scrolls (a chart sheet): the first object that is not deleted, at the size of the window, on a white page.
    void setFillsView(bool fills);
    bool fillsView() const { return m_fills; }

    // Puts one more object over the others (a chart a person just made); more than kMaxObjects are left out. The pictures of the others are kept.
    void addObject(LayerObject object);

    // Puts the object at a place in the order they are drawn in (0 is the back); the others keep their order.
    void moveObject(int id, size_t to);

    // An object is taken away (a person pressed Delete) or brought back (undone): it is not drawn or hit while it is.
    void setDeleted(int id, bool deleted);
    bool isDeleted(int id) const;
    // The object's anchor changed (cells were inserted or deleted before it): it is placed again.
    void setAnchor(int id, const jp::office::sheet::DrawingAnchor &anchor);
    // Where the layer last put the object (grid units), even if the rows and columns have changed since: false in `placed` if it never has. What an object that keeps its size when cells are
    // resized is measured by, just before the sizes change.
    ObjectBox lastBox(int id, bool *placed = nullptr) const;
    // Where the anchor puts the object on the layout's rows and columns now (placing it if need be); false in `ok` if the layer has no layout yet or no such object.
    ObjectBox boxNow(int id, bool *ok = nullptr) const;
    // Its cells changed: the picture is made again, a moment later (the old one is drawn until it is, and a calculation that tells many changes makes one picture). invalidateAll for every object.
    void invalidate(int id);
    void invalidateAll();
    // The object is picked, or none (-1).
    int selectedId() const { return m_selected; }
    void select(int id);
    // What happens when Delete is pressed with an object picked; unset, the key is not taken.
    void setDeleteHook(std::function<void(int id)> hook) { m_deleteHook = std::move(hook); }

    // ---- moving and sizing with the mouse and the keys
    // The anchor that puts an object where `box` is (the grid's units) on the rows and columns now, as `editAs` says it follows them (two cells, one, or none); false if the layer has no layout.
    bool anchorAtBox(const ObjectBox &box, jp::office::sheet::AnchorKind editAs, jp::office::sheet::DrawingAnchor *out) const;
    // The picked object moved (or sized, with `resize`) by keys: by `step` units, or to the next edge of a cell with `snap`. The new place is told by objectMoved. False if none is picked or it can't be.
    bool nudge(int dx, int dy, bool resize, bool snap, double step = 6);
    // An object is being dragged (a press on it, or on a handle of the one picked, and the mouse moved); the drag ends at the button's release, or is given up (Escape).
    bool dragging() const { return m_drag.mode == DragMode::Move || m_drag.mode == DragMode::Resize; }
    void cancelDrag();
    // Whether the object may be moved by a person (not one that fills the view, or that is taken away).
    bool canMove(int id) const;
    // The squares of the picked object's handles on the screen in a part of the grid, by place (0 top left, going clockwise: top, top right, right, bottom right, bottom, bottom left, left).
    static constexpr int kHandles = 8;
    static constexpr int kHandleSize = 7;
    QRect handleRect(int handle, const QRect &objectRect) const;
    // The handle at a point (-1 for none) for the object that is picked.
    int handleAt(const QPoint &point) const;

    // Where the object is on the screen now, in the part of the grid given: empty if it is not shown there.
    QRect deviceRect(int id, Pane pane) const;
    // The object at a point of the viewport (the topmost whose ink, or whose box for a picture or chart, is under it), or -1.
    int objectAt(const QPoint &point) const;

    // Makes every picture that the screen needs now, however many and however long (waits for the charts on the worker); otherwise they are made in the time between events.
    void prepareAll();
    // Pictures the screen needs that are not made, are out of date, or are being made: a window waits for 0 before it is shot.
    int pendingRenders() const;
    // Whether the layer has work in hand: a step to take, or a chart on the worker.
    bool busy() const { return m_timer.isActive() || m_inflight > 0; }

    // What was done, counted (a test reads these; none is a time).
    struct Stats {
        qint64 renders = 0;        // pictures made (on the window's thread, or by a job)
        qint64 jobs = 0;           // of them, jobs given to the worker
        qint64 drawn = 0;          // pictures put on the screen
        qint64 placed = 0;         // places worked out from anchors
        qint64 evicted = 0;        // pictures let go of for the budget
        qint64 placeholders = 0;   // objects drawn as an outline for want of a picture
        qint64 paintRenders = 0;   // pictures made, or given to the worker, inside a paint (none: a paint makes nothing)
        qint64 examined = 0;       // objects looked at to find what is on the screen (the index gives those near it: not every object of the sheet)
        qint64 shrunk = 0;         // times pictures were made smaller for the budget
    };
    const Stats &stats() const { return m_stats; }
    qint64 cacheBytes() const { return m_bytes; }
    // The factor the pictures are made smaller by so that those on the screen fit the budget (1, 1/2, 1/4...).
    double shrink() const { return m_shrink; }

    void paint(QPainter *painter, const GridLayout &layout, Pane pane, const QRect &clip) override;
    bool press(const QPoint &point, const GridLayout &layout, Qt::KeyboardModifiers modifiers) override;
    void setMirrored(bool mirrored) override { m_mirrored = mirrored; }
    bool key(QKeyEvent *event) override;
    void layoutChanged() override;
    bool mouseMove(const QPoint &point, const GridLayout &layout, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers) override;
    bool mouseRelease(const QPoint &point, const GridLayout &layout, Qt::KeyboardModifiers modifiers) override;
    std::optional<Qt::CursorShape> cursorAt(const QPoint &point, const GridLayout &layout) override;
    bool contextMenu(const QPoint &point, const QPoint &global, const GridLayout &layout) override;
    bool doubleClick(const QPoint &point, const GridLayout &layout) override;

Q_SIGNALS:
    void selectionChanged(int id);
    // A person moved or sized the object (with the mouse or the keys): the anchor that puts it where they left it.
    void objectMoved(int id, const jp::office::sheet::DrawingAnchor &anchor);
    // A right click on an object (it is picked), at a point of the screen.
    void contextMenuRequested(int id, const QPoint &global);
    // A double click on an object.
    void objectActivated(int id);

private Q_SLOTS:
    void deliver();

private:
    enum class DragMode : quint8 { None, Pending, Move, Resize };
    struct Drag {
        DragMode mode = DragMode::None;
        int id = -1;
        int handle = -1;
        QPoint from;           // where the button went down (the layout's places)
        ObjectBox start;       // the box then
        ObjectBox now;
        bool changed = false;
        size_t entry = 0;
    };
    bool m_mirrored = false;   // the painter is mirrored: pictures are turned back
    struct Entry {
        LayerObject object;
        mutable ObjectBox box;
        mutable bool placed = false;
        bool settled = false;
        bool deleted = false;
        QImage image;
        QSize requested;           // the size the picture was made for (device pixels)
        double zoom = 0;
        bool stale = false;        // its cells changed after the picture was made
        bool failed = false;       // the draw function gave no picture for this size
        bool pending = false;      // a job is making it
        quint64 token = 0;         // of the job that is
        qint64 reserved = 0;       // bytes set aside for that picture
        quint64 lastUsed = 0;
    };
    struct Result {
        quint64 generation = 0;
        size_t index = 0;
        quint64 token = 0;
        QSize requested;
        double zoom = 0;
        QImage image;
    };
    ObjectBox dragBox(const Drag &d, const QPoint &point, Qt::KeyboardModifiers modifiers, const Entry &e) const;
    double snapAxis(const Axis &axis, double at) const;
    void finishDrag(bool commit);
    void ensureBoxes(const GridLayout &layout) const;
    // The objects (their places in the list, in order) that may be in `area` of the pane (device pixels): those the index finds by where the area is on the sheet, and the few that are
    // too large to be indexed. Each is then asked for its own rectangle.
    void candidates(const GridLayout &layout, Pane pane, const QRect &area, std::vector<size_t> *out) const;
    // The same for the four panes of the window, each in the area given by `widen` around what it shows, with none twice.
    void candidatesAll(const GridLayout &layout, double widen, std::vector<size_t> *out) const;
    void buildIndex() const;
    int firstLive() const;
    size_t entryOf(int id) const;   // the place of the object in the list, or the list's size if there is none
    QRect rectIn(const GridLayout &layout, Pane pane, const Entry &e) const;
    // The rectangle drawn for the object (its box and the margin), and what it asks of the object's draw function when the picture is `wanted` pixels.
    QRect drawRect(const QRect &box, const Entry &e, double zoom) const;
    RenderRequest requestFor(const Entry &e, const QRect &box, const QSize &wanted, double zoom) const;
    QSize sizeWanted(const QRect &rect, double dpr) const;
    bool missing(const Entry &e, const QSize &wanted, double zoom) const;
    void settle(Entry &e);
    void store(Entry &e, const QImage &image, const QSize &wanted, double zoom);
    void startJob(Entry &e, size_t index, const QSize &wanted, double zoom, const RenderRequest &request);
    void updateShrink(const GridLayout &layout);
    void evict();
    void schedule(int millis);
    // Makes the pictures the screen needs (those with none first), and starts the jobs; `all` leaves out the limits of a step; `near` goes on to the objects a screenful away on each side
    // (what a scroll will show next). True if some are left.
    bool makePictures(bool all, bool near);
    void step();
    QRect visibleRect(const Entry &e) const;
    bool inkAt(const Entry &e, const GridLayout &layout, Pane pane, const QPoint &point) const;

    std::vector<Entry> m_entries;
    std::unordered_map<int, size_t> m_byId;   // an object's place in the list, by its id (the first, if the ids repeat)
    // Where the objects are on the sheet: a grid of squares of kCell units, each holding the objects that touch it, made when the boxes are worked out.
    struct Index {
        std::unordered_map<quint64, std::vector<quint32>> squares;
        std::vector<quint32> large;           // objects that touch more squares than are worth registering
    };
    mutable Index m_index;
    mutable bool m_indexValid = false;
    mutable std::vector<quint32> m_seen;      // for each object, the last query that found it (a query finds each once)
    mutable quint32 m_query = 0;
    double m_maxMargin = 0;                   // the largest margin of any object: what an area is widened by
    mutable int m_firstLive = -2;             // the first object that is not deleted (what fills the view), -1 if there is none, -2 if it must be looked for
    const GridLayout *m_layout = nullptr;
    bool m_painting = false;
    mutable std::vector<qint64> m_signature;   // of the rows and columns the boxes were worked out for
    mutable bool m_boxesValid = false;
    bool m_fills = false;
    int m_selected = -1;
    quint64 m_frame = 0;
    quint64 m_generation = 1;
    qint64 m_bytes = 0, m_reserved = 0;
    int m_inflight = 0;
    double m_dpr = 1.0;
    double m_shrink = 1.0;
    std::vector<qint64> m_nearKey;   // where the grid was when the objects near the screen were last looked at
    QTimer m_timer;
    QThreadPool m_pool;
    std::mutex m_resultsMutex;
    std::vector<Result> m_results;
    mutable Stats m_stats;
    std::function<void(int)> m_deleteHook;
    Drag m_drag;
    mutable size_t m_dragEntry = size_t(-1);   // the entry being dragged: always drawn, wherever the index puts it
};

} // namespace jp::grid

Q_DECLARE_METATYPE(jp::office::sheet::DrawingAnchor)
