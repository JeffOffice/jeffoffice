#pragma once
// Paints pages and frames with QPainter. The same code renders to the screen,
// thumbnails, PDF, pictures and the printer.

#include "core/document.h"
#include "text/textengine.h"

#include <QDateTime>
#include <QHash>
#include <QPainterPath>
#include <functional>
#include <memory>

class QPainter;

namespace jp {

struct RenderOptions {
    bool output = false;          // printing/exporting: no placeholders or screen-only marks
    TextOutput textOutput = TextOutput::Picture;   // what the text is drawn for (a PDF export, the printer, or a screen or picture)
    int mergeRecord = -1;
    // The time the date and time fields show; unset, the time on the clock. A test, or a caller that needs
    // every page of a long job to say the same minute, sets it.
    QDateTime now;
    bool shadeFields = false;
    bool showSpecial = false;
    bool tableGridlines = true;   // dotted gridlines on borderless tables (screen only)
    bool grayscale = false;
    // Text editing visuals for one story.
    QString editStory;
    int selFrom = -1, selTo = -1;
    QHash<QString, QVector<QPair<int, int>>> misspelled;   // storyId -> document ranges
    QSet<QString> hidden;         // item ids not to paint (e.g. while dragging a copy)
    // PDF/A forbids transparency: draw any item that needs it as an opaque
    // patch already blended with what lies beneath (rendered at flattenDpi).
    bool flattenTransparency = false;
    double flattenDpi = 300;
    // Output only: pictures are downsampled to at most this resolution (0: keep).
    double maxImageDpi = 0;
    bool skipPictures = false;    // spot color plates: pictures print on the process plates
};

class LayoutCache {
public:
    struct FrameLayout {
        const StoryLayout *layout = nullptr;
        int frame = -1;           // index of this text box within its chain
        double fitScale = 1.0;
    };

    // Layout for a text box (and its whole linked chain). pageNumber is used for
    // boxes on master pages, which show the number of the page being drawn.
    FrameLayout textFrame(const Document &doc, const TextItem &frame, int pageNumber, const RenderOptions &opt);
    // Layout for a single story in one box (shape text, table cells).
    // `fitScale` and `lineCut` are what a shape's text that shrinks to fit is drawn at (see ShapeItem::shrinks).
    const StoryLayout *storyBox(const Document &doc, const QString &storyId, const FrameSpec &spec, const QString &key, double fitScale = 1.0, double lineCut = 0);
    void clear() { m_entries.clear(); m_fields.clear(); }

private:
    // What the fields of a story show that its layout's key doesn't say (the date and time, business information, mail merge values), as text,
    // and its notes' text as a number: both in the key, so that the layout is laid out again when they change. The story's fields are found
    // again when it, or a note it refers to, changes.
    QString fieldsKey(const Document &doc, const Story &story, const FieldContext &ctx);
    struct FieldNote {
        int revision = -1;
        quint64 notes = 0;
        StoryFields fields;
    };
    QHash<quint64, FieldNote> m_fields;   // by story serial
    struct Entry {
        QString sig;
        std::shared_ptr<StoryLayout> layout;
        double fitScale = 1.0;
    };
    QHash<QString, Entry> m_entries;
};

struct PaintContext {
    const Document *doc = nullptr;
    LayoutCache *cache = nullptr;
    RenderOptions opt;
    int pageNumber = 1;           // 1-based number of the page being drawn
    int pageCount = 1;
    // For a caller that moves things on a page (a slide show's animations).
    // Unset, a page is painted as it is. `around` is called for each of the
    // page's own objects, not the master pages' (and not when transparency is
    // flattened), in place of painting it: `draw` paints it.
    std::function<void(QPainter *, const Item &, const std::function<void(QPainter *)> &draw)> around;
    // Likewise for the text of a text box or a shape: `paintText` paints the
    // lines of `layout`'s `frame`, with the painter placed as they are placed.
    std::function<void(QPainter *, const Item &, const StoryLayout &layout, int frame, const std::function<void(QPainter *)> &paintText)> aroundText;
};

class Renderer {
public:
    // How a document's text is set: its colors, fonts, tab stops, and (a
    // presentation's) slide lines.
    static LayoutEnv layoutEnv(const Document &doc);
    // Paints one page (background, master items, page items) in page coordinates.
    static void paintPage(QPainter *p, const PaintContext &ctx, int pageIndex);
    static void paintMaster(QPainter *p, const PaintContext &ctx, const MasterPage &m, double xOffset = 0);
    // A master page as the canvas shows it while it is edited, over the page rectangle `r`: its background (else the one of the master
    // page under it), the objects of the master pages under it (unless it hides them), and its own objects, placeholders and all.
    static void paintMasterView(QPainter *p, const PaintContext &ctx, const MasterPage &m, const QRectF &r);
    static void paintBackground(QPainter *p, const PaintContext &ctx, const Fill &bg, const QRectF &r);
    static void paintItems(QPainter *p, const PaintContext &ctx, const ItemList &items);
    // Whether drawing this item produces any transparency (see-through picture
    // pixels, translucent fills or lines, shadows, glows, soft edges...).
    static bool usesTransparency(const Document &doc, const Item &it);
    static void paintItem(QPainter *p, const PaintContext &ctx, const Item &it);

    // Frame-local outline used for selection, wrap and shadows.
    static QPainterPath silhouette(const Document &doc, const Item &it);
    // Starting points for Edit Wrap Points: the object's outline, simplified
    // to a polygon of a few dozen points in frame-local coordinates.
    static QPolygonF defaultWrapPolygon(const Document &doc, const Item &it);
    static QPainterPath textArtOutline(const TextArtItem &w);
    // shown: a picture drawn into pixels smaller than it is comes back at that size, each pixel the average of those it covers.
    static QImage processedImage(const Document &doc, const PictureItem &pic, const QSizeF &deviceSize, const QSizeF &shown = QSizeF());
    static QImage renderToImage(const PaintContext &ctx, int pageIndex, double scale, bool transparent = false);
    static QImage renderItemsToImage(const PaintContext &ctx, const ItemList &items, double scale);

    // Wrap obstacles (frame-local polygons) for a text box on a page.
    static QVector<QPolygonF> wrapObstacles(const Document &doc, const TextItem &frame, bool *covered = nullptr);   // covered: an object in front wraps around the whole box
    static FrameSpec frameSpec(const Document &doc, const TextItem &t, int pageNumber, const RenderOptions &opt);
    // Layout of the text inside a shape or a table cell, with its frame-local origin.
    static const StoryLayout *shapeTextLayout(const PaintContext &ctx, const ShapeItem &s, QPointF *origin);
    // half: 0, or 1 for the second half of a cell divided by a diagonal.
    static const StoryLayout *cellLayout(const PaintContext &ctx, const TableItem &t, int row, int col, QPointF *origin, int half = 0);
    static void strokePath(QPainter *p, const QPainterPath &path, const Stroke &s, const ColorScheme &cs);
};

QColor grayOf(const QColor &c);
void boxBlur(QImage &img, int radius);

} // namespace jp
