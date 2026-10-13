// The objects of an open document presentation (odpexport_p.h): text boxes
// and placeholders, shapes by their outlines, pictures with their crop, lines
// and connectors, groups, and tables.

#include "io/odpexport_p.h"

#include "core/media.h"
#include "core/shapetypes.h"
#include "office/drawingml_geometry.h"
#include "office/drawingml_write.h"
#include "render/renderer.h"
#include "render/shapes.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QImage>
#include <QTextDocument>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp::deck::odpw {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("OdpExport", text); }

QString attrOf(const char *name, const QString &value) { return QLatin1Char(' ') + QLatin1String(name) + QStringLiteral("=\"") + esc(value) + QLatin1Char('"'); }

// Units of a path: hundredths of a millimeter, the unit of a view box.
constexpr double kUnit = 2540.0 / 72.0;

QString unit(double points) { return QString::number(std::llround(points * kUnit)); }

// A path as the commands of an enhanced geometry, in the units of its view box; `open` shapes are not filled. A piece that ends where it
// began is closed; one that doesn't is left open (it is filled as if closed, but its outline doesn't run across).
QString enhancedPath(const QPainterPath &path, const QTransform &flip, bool open)
{
    QString out;
    const int count = path.elementCount();
    auto pt = [&](const QPainterPath::Element &e) {
        const QPointF p = flip.map(QPointF(e.x, e.y));
        return unit(p.x()) + QLatin1Char(' ') + unit(p.y());
    };
    int i = 0;
    while (i < count) {
        int end = i + 1;
        while (end < count && path.elementAt(end).type != QPainterPath::MoveToElement) ++end;
        const QPainterPath::Element first = path.elementAt(i), last = path.elementAt(end - 1);
        out += (out.isEmpty() ? QString() : QStringLiteral(" ")) + QStringLiteral("M ") + pt(first);
        for (int k = i + 1; k < end; ++k) {
            const QPainterPath::Element e = path.elementAt(k);
            if (e.type == QPainterPath::LineToElement) {
                out += QStringLiteral(" L ") + pt(e);
            } else if (e.type == QPainterPath::CurveToElement && k + 2 < end) {
                out += QStringLiteral(" C ") + pt(e) + QLatin1Char(' ') + pt(path.elementAt(k + 1)) + QLatin1Char(' ') + pt(path.elementAt(k + 2));
                k += 2;
            }
        }
        if (!open && end - i > 2 && std::fabs(last.x - first.x) < 1e-6 && std::fabs(last.y - first.y) < 1e-6) out += QStringLiteral(" Z");
        i = end;
    }
    return out + (open ? QStringLiteral(" F N") : QStringLiteral(" N"));
}

// The length-and-color of a border: "0.05cm solid #000000".
QString borderOf(const Stroke &s, const QColor &color)
{
    if (s.isNone() || !color.isValid()) return QStringLiteral("none");
    const bool dotted = s.dash == Stroke::RoundDot || s.dash == Stroke::SquareDot;
    const QString style = s.dash == Stroke::SolidLine ? QStringLiteral("solid") : dotted ? QStringLiteral("dotted") : QStringLiteral("dashed");
    return cm(std::max(s.width, 0.25)) + QLatin1Char(' ') + style + QLatin1Char(' ') + hexOf(color);
}

// The line on one side of a cell: the cell's own, else the neighbor's on that same edge (what a person sees is one line, whichever cell holds it).
Stroke edgeOf(const TableItem &t, int row, int col, int side)
{
    const TableCell &c = t.cell(row, col);
    const Stroke own = side == 0 ? c.border.top : side == 1 ? c.border.bottom : side == 2 ? c.border.left : c.border.right;
    const bool outer = (side == 0 && row == 0) || (side == 1 && row + c.rowSpan >= t.rows) || (side == 2 && col == 0) || (side == 3 && col + c.colSpan >= t.cols);
    if (outer && !t.stroke.isNone()) return t.stroke;
    if (side == 0 && row > 0) {
        const Stroke other = t.cell(row - 1, col).border.bottom;
        return own.isNone() ? other : own;
    }
    if (side == 2 && col > 0) {
        const Stroke other = t.cell(row, col - 1).border.right;
        return own.isNone() ? other : own;
    }
    if (side == 1 && row + c.rowSpan < t.rows) {
        const Stroke later = t.cell(row + c.rowSpan, col).border.top;
        return later.isNone() ? own : later;
    }
    if (side == 3 && col + c.colSpan < t.cols) {
        const Stroke later = t.cell(row, col + c.colSpan).border.left;
        return later.isNone() ? own : later;
    }
    return own;
}

bool hasCells(const TableItem &t) { return t.rows > 0 && t.cols > 0 && t.cells.size() == t.rows * t.cols && t.colW.size() >= t.cols && t.rowH.size() >= t.rows; }

} // namespace

// ---- what every object has

void Exporter::writePlace(X &x, const QRectF &rect, double rotation)
{
    const double w = std::max(0.0, rect.width()), h = std::max(0.0, rect.height());
    const double turn = std::fmod(std::fmod(rotation, 360.0) + 360.0, 360.0);
    x.len("svg:width", w).len("svg:height", h);
    if (turn < 0.005 || turn > 359.995) {
        x.len("svg:x", rect.x()).len("svg:y", rect.y());
        return;
    }
    // The shape is made at the origin, turned there (the file's angle runs the other way), and moved to where its middle stands.
    QTransform r;
    r.rotate(turn);
    const QPointF moved = rect.center() - r.map(QPointF(w / 2, h / 2));
    x.attr("draw:transform", QStringLiteral("rotate (%1) translate (%2 %3)").arg(number(-qDegreesToRadians(turn), 8), cm(moved.x()), cm(moved.y())));
}

QString Exporter::idAttr(const Item &it) const
{
    const auto at = m_ids.constFind(it.id);
    return at == m_ids.cend() ? QString() : at.value();
}

void Exporter::writeCommon(X &x, const Item &it)
{
    if (!it.name.isEmpty()) x.attr("draw:name", it.name);
    const QString id = idAttr(it);
    if (!id.isEmpty()) x.attr("xml:id", id).attr("draw:id", id);
}

// The description for people who can't see the object, and what a click on it does; inside its element.
void Exporter::writeInside(X &x, const Item &it)
{
    if (!it.altText.isEmpty()) x.open("svg:desc").text(it.altText).close();
    if (!it.hyperlink.isEmpty()) {
        x.open("office:event-listeners");
        x.open("presentation:event-listener").attr("script:event-name", "dom:click").attr("presentation:action", "show");
        x.attr("xlink:href", it.hyperlink).attr("xlink:type", "simple").attr("xlink:show", "embed").attr("xlink:actuate", "onRequest").close();
        x.close();
    }
}

QString Exporter::presentationClass(const Item &it, bool master) const
{
    if (!it.role.startsWith(QLatin1String("ph:"))) return QString();
    const QString cls = holderClassOf(placeholderType(it));
    // On a slide the footers are the page's; a placeholder of another kind than text has no place on a master page.
    if (master && (cls == QLatin1String("graphic") || cls == QLatin1String("chart") || cls == QLatin1String("table") || cls == QLatin1String("object"))) return QString();
    return cls;
}

QString Exporter::frameStyle(Scope &sc, const Item &it, const QString &cls, const QString &extra, const QString &children, const QString &paragraph, bool filled)
{
    QString attrs = (filled ? fillAttrs(it.fill, it.rect.size()) : QStringLiteral(" draw:fill=\"none\"")) + strokeAttrs(it.stroke) + effectAttrs(it.fx) + extra;
    QString props = children.isEmpty() ? QStringLiteral("<style:graphic-properties%1/>").arg(attrs) : QStringLiteral("<style:graphic-properties%1>%2</style:graphic-properties>").arg(attrs, children);
    if (!paragraph.isEmpty()) props += QStringLiteral("<style:paragraph-properties%1/>").arg(paragraph);
    if (cls.isEmpty()) return sc.styles->style(QStringLiteral("graphic"), sc.master ? QStringLiteral("Mgr") : QStringLiteral("gr"), QString(), props);
    QString parent;
    if (cls == QLatin1String("title")) parent = sc.prefix + QStringLiteral("title");
    else if (cls == QLatin1String("subtitle")) parent = sc.prefix + QStringLiteral("subtitle");
    else if (cls == QLatin1String("outline")) parent = sc.prefix + QStringLiteral("outline1");
    else if (cls == QLatin1String("notes")) parent = sc.prefix + QStringLiteral("notes");
    else if (sc.master) parent = sc.prefix + QStringLiteral("backgroundobjects");
    return sc.styles->style(QStringLiteral("presentation"), sc.master ? QStringLiteral("Mpr") : QStringLiteral("pr"), parent, props);
}

// ---- the list of objects

void Exporter::writeItems(X &x, Scope &sc, const ItemList &items)
{
    for (const ItemPtr &it : items) writeItem(x, sc, *it);
}

void Exporter::writeItem(X &x, Scope &sc, const Item &it)
{
    if (m_flat.contains(it.id)) {
        writeAsPicture(x, sc, it);
        return;
    }
    switch (it.type()) {
    case ItemType::Text: writeTextBox(x, sc, static_cast<const TextItem &>(it)); break;
    case ItemType::Shape: writeShape(x, sc, static_cast<const ShapeItem &>(it)); break;
    case ItemType::Picture: writePicture(x, sc, static_cast<const PictureItem &>(it)); break;
    case ItemType::Line: writeLine(x, sc, static_cast<const LineItem &>(it)); break;
    case ItemType::Group: writeGroup(x, sc, static_cast<const GroupItem &>(it)); break;
    case ItemType::Table: writeTable(x, sc, static_cast<const TableItem &>(it)); break;
    case ItemType::TextArt:
        sayOnce(QStringLiteral("textart"), tr("Text Art is saved as a picture."));
        writeAsPicture(x, sc, it);
        break;
    }
}

// An object that this kind of file can't draw as itself, as a picture of what it looks like.
void Exporter::writeAsPicture(X &x, Scope &sc, const Item &it)
{
    const ItemPtr ptr = m_doc.itemPtr(it.id);
    if (!ptr) return;
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = &m_doc;
    ctx.cache = &cache;
    ctx.opt.output = true;
    const ItemList one{ptr};
    const QRectF bounds = unionBounds(one).adjusted(-4, -4, 4, 4);
    if (!(bounds.width() > 0 && bounds.height() > 0)) return;
    const double scale = std::clamp(2.0, 0.1, 4000.0 / std::max(bounds.width(), bounds.height()));
    const QImage image = Renderer::renderItemsToImage(ctx, one, scale);
    if (image.isNull()) return;
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG")) return;
    const QString part = QStringLiteral("Pictures/object%1.png").arg(m_pictureNames.size() + 1);
    m_pictureNames.insert(QStringLiteral("flat:") + it.id, part);
    addFile(part, bytes, QStringLiteral("image/png"), false);
    const QString style = sc.styles->style(QStringLiteral("graphic"), sc.master ? QStringLiteral("Mgr") : QStringLiteral("gr"), QString(),
                                           QStringLiteral("<style:graphic-properties draw:fill=\"none\" draw:stroke=\"none\" draw:shadow=\"hidden\"/>"));
    x.open("draw:frame").attr("draw:style-name", style);
    writeCommon(x, it);
    x.attr("draw:layer", sc.layer);
    writePlace(x, bounds, 0);
    x.open("draw:image").attr("xlink:href", part).attr("xlink:type", "simple").attr("xlink:show", "embed").attr("xlink:actuate", "onLoad").attr("loext:mime-type", "image/png").leaf("text:p").close();
    writeInside(x, it);
    x.close();
}

// ---- text boxes

// Whether text left at the size its file says (the presentation program shrinks it to fit the box when it is edited) runs out of its box at
// that size, as the layout here counts it.
static bool overflowsAsStored(const Document &doc, const TextItem &t, int pageNumber)
{
    if (t.vertical || t.columns > 1) return false;
    LayoutCache cache;
    const LayoutCache::FrameLayout fl = cache.textFrame(doc, t, pageNumber, RenderOptions());
    if (!fl.layout) return false;
    if (fl.layout->overflow()) return true;
    const auto lines = fl.layout->lineInfo(fl.frame);
    if (lines.isEmpty()) return false;
    double top = lines.first().rect.top(), bottom = lines.first().rect.bottom() - lines.first().below;
    for (const auto &li : lines) {
        top = std::min(top, li.rect.top());
        bottom = std::max(bottom, li.rect.bottom() - li.below);
    }
    return bottom - top > t.rect.height() - t.insets.top() - t.insets.bottom() + 0.5;
}

void Exporter::writeTextBox(X &x, Scope &sc, const TextItem &t)
{
    const QString cls = presentationClass(t, sc.master);
    const QTextDocument *story = m_doc.storyDoc(t.storyId);
    const bool empty = !story || story->characterCount() <= 1;
    // The pieces of the style that say how the text sits in its frame.
    QString extra;
    extra += attrOf("fo:padding-top", cm(t.insets.top())) + attrOf("fo:padding-bottom", cm(t.insets.bottom())) + attrOf("fo:padding-left", cm(t.insets.left())) + attrOf("fo:padding-right", cm(t.insets.right()));
    extra += attrOf("draw:textarea-vertical-align", t.valign == VAlign::Middle ? QStringLiteral("middle") : t.valign == VAlign::Bottom ? QStringLiteral("bottom") : QStringLiteral("top"));
    extra += attrOf("draw:textarea-horizontal-align", QStringLiteral("justify"));
    extra += attrOf("draw:auto-grow-height", t.autofit == TextItem::GrowBox ? QStringLiteral("true") : QStringLiteral("false"));
    extra += attrOf("draw:auto-grow-width", QStringLiteral("false")) + attrOf("draw:fit-to-size", QStringLiteral("false"));
    // Text that shrinks to fit keeps doing so, unless the file left it at a size that doesn't fit (as a design's sample text does): the other
    // suite would shrink that when it opens it, and it is shown as it is.
    const bool stored = m_doc.slideText && t.autofit == TextItem::ShrinkOnOverflow && t.fitAsStored && !sc.master;
    bool shrinks = t.autofit == TextItem::BestFit || t.autofit == TextItem::ShrinkOnOverflow;
    if (shrinks && stored && !empty && m_page) {
        int number = 1;
        for (int i = 0; i < m_doc.pages.size(); ++i)
            if (m_doc.pages[i].get() == m_page) number = i + 1;
        if (overflowsAsStored(m_doc, t, number)) shrinks = false;
    }
    extra += attrOf("style:shrink-to-fit", shrinks ? QStringLiteral("true") : QStringLiteral("false"));
    extra += attrOf("fo:min-height", cm(t.autofit == TextItem::GrowBox ? t.rect.height() : 0.0)) + attrOf("fo:wrap-option", QStringLiteral("wrap"));
    QString children;
    if (t.columns > 1) children = QStringLiteral("<style:columns fo:column-count=\"%1\" fo:column-gap=\"%2\"/>").arg(std::clamp(t.columns, 1, 16)).arg(cm(t.columnGap));
    const QString paragraph = attrOf("style:writing-mode", t.vertical ? QStringLiteral("tb-rl") : QStringLiteral("lr-tb"));
    const QString style = frameStyle(sc, t, cls, extra, children, paragraph);

    x.open("draw:frame");
    x.attr(cls.isEmpty() ? "draw:style-name" : "presentation:style-name", style);
    x.attr("draw:text-style-name", slideLinesStyle(sc));
    writeCommon(x, t);
    x.attr("draw:layer", sc.layer);
    writePlace(x, t.rect, t.rotation);
    if (!cls.isEmpty()) {
        x.attr("presentation:class", cls);
        x.attr("presentation:user-transformed", "true");
        if (empty && !sc.master) x.attr("presentation:placeholder", "true");
    }
    x.open("draw:text-box");
    if (t.autofit == TextItem::GrowBox) x.len("fo:min-height", t.rect.height());
    // Text the presentation program shrank is written at the size it was shrunk to, as it is laid out here.
    struct FitGuard {
        double &scale, &cut, oldScale, oldCut;
        FitGuard(double &s, double &c, double ns, double nc) : scale(s), cut(c), oldScale(s), oldCut(c) { scale = ns; cut = nc; }
        ~FitGuard() { scale = oldScale; cut = oldCut; }
    };
    FitGuard fit(m_fitScale, m_fitCut, stored ? std::clamp(t.fitScale, 0.01, 1.0) : 1.0, stored ? std::clamp(t.fitLineCut, 0.0, 99.0) : 0.0);
    if (!empty || sc.master) {
        TextOpts opts;
        opts.placeholder = !cls.isEmpty();
        opts.behind = sc.background;
        opts.shape = t.sourceId;
        // A master's footers say what they stand for; the slides fill them in.
        if (sc.master && cls == QLatin1String("date-time")) {
            QTextBlock b = story ? story->begin() : QTextBlock();
            const QString style = b.isValid() ? paraStyleOf(sc, b, false, opts) : QString();
            x.open("text:p").attr("text:style-name", style).leaf("presentation:date-time").close();
        } else if (sc.master && cls == QLatin1String("footer")) {
            QTextBlock b = story ? story->begin() : QTextBlock();
            const QString style = b.isValid() ? paraStyleOf(sc, b, false, opts) : QString();
            x.open("text:p").attr("text:style-name", style).leaf("presentation:footer").close();
        } else if (sc.master && cls == QLatin1String("page-number")) {
            QTextBlock b = story ? story->begin() : QTextBlock();
            const QString style = b.isValid() ? paraStyleOf(sc, b, false, opts) : QString();
            QTextCharFormat cf = b.isValid() ? b.charFormat() : QTextCharFormat();
            x.open("text:p").attr("text:style-name", style).open("text:span").attr("text:style-name", textStyleOf(sc, cf)).open("text:page-number").text(QStringLiteral("<number>")).close().close().close();
        } else if (story) {
            writeStory(x, sc, story, opts);
        }
    }
    x.close();
    writeInside(x, t);
    x.close();
}

// ---- shapes

void Exporter::writeShape(X &x, Scope &sc, const ShapeItem &s)
{
    const QSizeF size(std::max(0.0, s.rect.width()), std::max(0.0, s.rect.height()));
    QPainterPath path;
    QRectF textRect;
    bool open = false;
    QString type = QStringLiteral("non-primitive"), modifiers;
    if (!s.customPath.isEmpty()) {
        path = s.customPath;
    } else {
        path = shapePath(s.shape, size, s.adj);
        textRect = shapeTextRect(s.shape, size, s.adj);
        const ShapeDef *def = shapeDef(s.shape);
        open = def && def->open;
        const QString prst = office::presetForShape(s.shape);
        if (!prst.isEmpty()) {
            type = QStringLiteral("ooxml-") + prst;
            // The values of its handles, in the order the standard names them.
            QVector<QPair<QString, double>> adjusted = office::presetAdjustments(prst, s.adj, size);
            std::sort(adjusted.begin(), adjusted.end(), [](const auto &a, const auto &b) { return a.first.mid(3).toInt() < b.first.mid(3).toInt(); });
            QStringList values;
            for (const auto &a : std::as_const(adjusted)) values << number(a.second, 0);
            modifiers = values.join(QLatin1Char(' '));
        }
    }
    // A flipped shape is drawn flipped: the path says so.
    QTransform flip;
    if (s.flipH) flip.translate(size.width(), 0).scale(-1, 1);
    if (s.flipV) flip.translate(0, size.height()).scale(1, -1);
    const QString d = enhancedPath(path, flip, open);

    const QTextDocument *story = m_doc.storyDoc(s.storyId);
    const bool hasText = story && story->characterCount() > 1;
    QString extra;
    extra += attrOf("fo:padding-top", cm(s.insets.top())) + attrOf("fo:padding-bottom", cm(s.insets.bottom())) + attrOf("fo:padding-left", cm(s.insets.left())) + attrOf("fo:padding-right", cm(s.insets.right()));
    extra += attrOf("draw:textarea-vertical-align", s.valign == VAlign::Middle ? QStringLiteral("middle") : s.valign == VAlign::Bottom ? QStringLiteral("bottom") : QStringLiteral("top"));
    extra += attrOf("draw:textarea-horizontal-align", QStringLiteral("justify")) + attrOf("draw:auto-grow-height", QStringLiteral("false")) + attrOf("draw:auto-grow-width", QStringLiteral("false"));
    extra += attrOf("fo:wrap-option", QStringLiteral("wrap"));
    const QString style = frameStyle(sc, s, QString(), extra, QString(), attrOf("style:writing-mode", QStringLiteral("lr-tb")), true);

    x.open("draw:custom-shape").attr("draw:style-name", style);
    if (hasText) x.attr("draw:text-style-name", slideLinesStyle(sc));
    writeCommon(x, s);
    x.attr("draw:layer", sc.layer);
    writePlace(x, s.rect, s.rotation);
    writeInside(x, s);
    if (hasText) {
        TextOpts opts;
        opts.behind = sc.background;
        opts.shape = s.sourceId;
        writeStory(x, sc, story, opts);
    }
    x.open("draw:enhanced-geometry");
    x.attr("svg:viewBox", QStringLiteral("0 0 %1 %2").arg(std::max<qint64>(1, std::llround(size.width() * kUnit))).arg(std::max<qint64>(1, std::llround(size.height() * kUnit))));
    x.attr("draw:type", type);
    if (!modifiers.isEmpty()) x.attr("draw:modifiers", modifiers);
    if (textRect.isValid() && textRect != QRectF(QPointF(0, 0), size))
        x.attr("draw:text-areas", QStringLiteral("%1 %2 %3 %4").arg(unit(textRect.left()), unit(textRect.top()), unit(textRect.right()), unit(textRect.bottom())));
    x.attr("draw:enhanced-path", d);
    x.close();
    x.close();
}

// ---- pictures

void Exporter::writePicture(X &x, Scope &sc, const PictureItem &p)
{
    const QString cls = presentationClass(p, sc.master);
    if (p.imageId.isEmpty()) {
        // The place for a picture, with none in it yet; one with a fill or an outline of its own shows that, as the shape it is.
        if (!p.fill.isNone() || !p.stroke.isNone()) {
            ShapeItem box;
            box.id = p.id;
            box.sourceId = p.sourceId;
            box.name = p.name;
            box.altText = p.altText;
            box.hyperlink = p.hyperlink;
            box.rect = p.rect;
            box.rotation = p.rotation;
            box.flipH = p.flipH;
            box.flipV = p.flipV;
            box.fill = p.fill;
            box.stroke = p.stroke;
            box.fx = p.fx;
            writeShape(x, sc, box);
            return;
        }
        if (cls.isEmpty() || sc.master) return;
        const QString style = frameStyle(sc, p, cls, QString(), QString(), QString(), false);
        x.open("draw:frame").attr("presentation:style-name", style);
        writeCommon(x, p);
        x.attr("draw:layer", sc.layer);
        writePlace(x, p.rect, p.rotation);
        x.attr("presentation:class", cls).attr("presentation:user-transformed", "true").attr("presentation:placeholder", "true");
        x.leaf("draw:image");
        x.close();
        return;
    }
    // Looks this kind of file can't give a picture are in the picture itself.
    const bool masked = !p.maskShape.isEmpty() && p.maskShape != QLatin1String("rect");
    const bool baked = masked || p.recolor == PictureItem::Sepia || p.recolor == PictureItem::ColorTint || (p.hasTransparentColor && p.transparentColor.isValid());
    if (baked) {
        writeAsPicture(x, sc, p);
        return;
    }
    const QString part = addPicture(p.imageId);
    if (part.isEmpty()) {
        sayOnce(QStringLiteral("nopicture"), tr("A picture that can't be read was left out."));
        return;
    }
    // A movie or sound the picture plays is kept in the package and played from the frame, with the picture as its poster.
    // One that is linked, not kept in the presentation, keeps its address (the other suite plays it if it can).
    QString mediaPart, mediaType;
    if (!p.mediaKind.isEmpty()) {
        mediaPart = addMedia(p.mediaId, &mediaType, p.mediaKind);
        if (mediaPart.isEmpty() && !p.mediaLink.isEmpty()) {
            mediaPart = p.mediaLink;
            mediaType = QStringLiteral("application/vnd.sun.star.media");
        } else if (mediaPart.isEmpty()) {
            sayOnce(QStringLiteral("media"), tr("A movie or sound that can't be read is saved as its picture."));
        }
    }

    QString extra;
    // The crop: lengths cut from the picture at the size its own resolution gives it.
    const QRectF whole = p.imgRect, frame(0, 0, p.rect.width(), p.rect.height());
    if (whole.width() > 0 && whole.height() > 0 && !frame.isEmpty() && whole != frame) {
        const QSize px = m_doc.imageSize(p.imageId);
        if (px.width() > 0 && px.height() > 0) {
            const QImage image = m_doc.image(p.imageId);
            const double dpiX = image.dotsPerMeterX() > 0 ? image.dotsPerMeterX() * 0.0254 : 96, dpiY = image.dotsPerMeterY() > 0 ? image.dotsPerMeterY() * 0.0254 : 96;
            const double naturalW = px.width() / dpiX * 72, naturalH = px.height() / dpiY * 72;
            const double l = (frame.left() - whole.left()) / whole.width(), t = (frame.top() - whole.top()) / whole.height();
            const double r = (whole.right() - frame.right()) / whole.width(), b = (whole.bottom() - frame.bottom()) / whole.height();
            extra += attrOf("fo:clip", QStringLiteral("rect(%1, %2, %3, %4)").arg(cm(t * naturalH), cm(r * naturalW), cm(b * naturalH), cm(l * naturalW)));
        }
    }
    // The other suite mirrors a picture side to side and ignores a mirror from top to bottom: turned over is half a turn and a mirror side to side.
    bool mirror = p.flipH;
    double turn = p.rotation;
    if (p.flipV) {
        mirror = !mirror;
        turn += 180;
    }
    if (mirror) extra += attrOf("style:mirror", QStringLiteral("horizontal"));
    extra += attrOf("draw:color-mode", p.recolor == PictureItem::Grayscale ? QStringLiteral("greyscale") : p.recolor == PictureItem::BlackWhite ? QStringLiteral("mono") : p.recolor == PictureItem::Washout ? QStringLiteral("watermark") : QStringLiteral("standard"));
    if (p.brightness != 0) extra += attrOf("draw:luminance", percent(std::clamp(p.brightness, -100.0, 100.0) / 100));
    if (p.contrast != 0) extra += attrOf("draw:contrast", percent(std::clamp(p.contrast, -100.0, 100.0) / 100));
    if (p.transparency > 0.001) extra += attrOf("draw:image-opacity", percent(std::clamp(1 - p.transparency, 0.0, 1.0)));
    const QString style = frameStyle(sc, p, cls, extra, QString(), QString(), false);

    x.open("draw:frame");
    x.attr(cls.isEmpty() ? "draw:style-name" : "presentation:style-name", style);
    writeCommon(x, p);
    x.attr("draw:layer", sc.layer);
    writePlace(x, p.rect, turn);
    if (!cls.isEmpty()) x.attr("presentation:class", cls).attr("presentation:user-transformed", "true");
    if (!mediaPart.isEmpty()) {
        const deck::MediaPlay play = m_page ? deck::mediaPlayOf(*m_page, p) : deck::MediaPlay();
        x.open("draw:plugin").attr("xlink:href", mediaPart).attr("xlink:type", "simple").attr("xlink:show", "embed").attr("xlink:actuate", "onLoad").attr("draw:mime-type", mediaType);
        x.open("draw:param").attr("draw:name", "Loop").attr("draw:value", play.loop ? "true" : "false").close();
        x.open("draw:param").attr("draw:name", "Mute").attr("draw:value", play.mute ? "true" : "false").close();
        // The volume in decibels, as the other suite keeps it (0 is full).
        const double db = std::clamp(20.0 * std::log10(std::max(1, play.volume) / 100000.0), -40.0, 0.0);
        x.open("draw:param").attr("draw:name", "VolumeDB").attr("draw:value", number(db, 1)).close();
        x.close();
    }
    x.open("draw:image").attr("xlink:href", part).attr("xlink:type", "simple").attr("xlink:show", "embed").attr("xlink:actuate", "onLoad");
    const auto image = m_doc.images.constFind(p.imageId);
    if (image != m_doc.images.cend()) {
        QString format = image->format.toLower();
        if (format == QLatin1String("jpeg")) format = QStringLiteral("jpg");
        static const QHash<QString, QString> types = {{QStringLiteral("png"), QStringLiteral("image/png")}, {QStringLiteral("jpg"), QStringLiteral("image/jpeg")}, {QStringLiteral("gif"), QStringLiteral("image/gif")},
                                                      {QStringLiteral("svg"), QStringLiteral("image/svg+xml")}, {QStringLiteral("bmp"), QStringLiteral("image/bmp")}};
        if (types.contains(format)) x.attr("loext:mime-type", types.value(format));
    }
    x.leaf("text:p").close();
    writeInside(x, p);
    x.close();
}

// ---- lines

void Exporter::writeLine(X &x, Scope &sc, const LineItem &l)
{
    const bool glued = !l.start.id.isEmpty() || !l.end.id.isEmpty();
    const bool bent = l.route != LineItem::Straight;
    const QString style = sc.styles->style(QStringLiteral("graphic"), sc.master ? QStringLiteral("Mgr") : QStringLiteral("gr"), QString(),
                                           QStringLiteral("<style:graphic-properties draw:fill=\"none\"%1%2/>").arg(strokeAttrs(l.stroke), effectAttrs(l.fx)));
    const bool connector = glued || bent;
    x.open(connector ? "draw:connector" : "draw:line").attr("draw:style-name", style);
    writeCommon(x, l);
    x.attr("draw:layer", sc.layer);
    if (connector) {
        x.attr("draw:type", l.route == LineItem::Elbow ? "standard" : l.route == LineItem::Curved ? "curve" : "line");
        // The shapes its ends are attached to, and the glue point of each: the sides in the file's order (top, right, bottom, left).
        static const int fileSite[4] = {0, 3, 2, 1};
        auto glue = [&](const char *shape, const char *point, const LineItem::Glue &g) {
            const auto id = m_ids.constFind(g.id);
            if (g.id.isEmpty() || id == m_ids.cend() || g.site < 0 || g.site > 3) return;
            x.attr(shape, id.value()).attr(point, fileSite[g.site]);
        };
        glue("draw:start-shape", "draw:start-glue-point", l.start);
        glue("draw:end-shape", "draw:end-glue-point", l.end);
    }
    x.len("svg:x1", l.p1.x()).len("svg:y1", l.p1.y()).len("svg:x2", l.p2.x()).len("svg:y2", l.p2.y());
    writeInside(x, l);
    x.close();
}

// ---- groups

void Exporter::writeGroup(X &x, Scope &sc, const GroupItem &g)
{
    if (g.children.empty()) return;
    if (sc.depth >= 64) return;
    x.open("draw:g");
    writeCommon(x, g);
    writeInside(x, g);
    ++sc.depth;
    writeItems(x, sc, g.children);
    --sc.depth;
    x.close();
}

// ---- tables

void Exporter::writeTable(X &x, Scope &sc, const TableItem &t)
{
    if (!hasCells(t)) return;
    double width = 0, height = 0;
    for (int c = 0; c < t.cols; ++c) width += std::max(0.0, t.colW[c]);
    for (int r = 0; r < t.rows; ++r) height += std::max(0.0, t.rowH[r]);
    const QString cls = presentationClass(t, sc.master);
    const QString style = frameStyle(sc, t, cls, QString(), QString(), QString(), false);
    x.open("draw:frame");
    x.attr(cls.isEmpty() ? "draw:style-name" : "presentation:style-name", style);
    writeCommon(x, t);
    x.attr("draw:layer", sc.layer);
    writePlace(x, QRectF(t.rect.x(), t.rect.y(), width, height), 0);
    if (!cls.isEmpty()) x.attr("presentation:class", cls).attr("presentation:user-transformed", "true");
    x.open("table:table");
    for (int c = 0; c < t.cols; ++c) {
        const QString col = sc.styles->style(QStringLiteral("table-column"), QStringLiteral("co"), QString(), QStringLiteral("<style:table-column-properties style:column-width=\"%1\"/>").arg(cm(t.colW[c])));
        x.open("table:table-column").attr("table:style-name", col).close();
    }
    TextOpts opts;
    opts.behind = sc.background;
    for (int r = 0; r < t.rows; ++r) {
        const QString row = sc.styles->style(QStringLiteral("table-row"), QStringLiteral("ro"), QString(),
                                             QStringLiteral("<style:table-row-properties style:row-height=\"%1\" style:min-row-height=\"%1\" fo:keep-together=\"auto\"/>").arg(cm(t.rowH[r])));
        x.open("table:table-row").attr("table:style-name", row);
        for (int c = 0; c < t.cols; ++c) {
            const TableCell &cell = t.cell(r, c);
            if (cell.covered) {
                x.leaf("table:covered-table-cell");
                continue;
            }
            // The cell's look: its margins, its place for text, its lines on the four sides, and its fill.
            QString props = attrOf("fo:padding-left", cm(cell.margins.left())) + attrOf("fo:padding-right", cm(cell.margins.right())) + attrOf("fo:padding-top", cm(cell.margins.top())) + attrOf("fo:padding-bottom", cm(cell.margins.bottom()));
            props += attrOf("style:vertical-align", cell.valign == VAlign::Middle ? QStringLiteral("middle") : cell.valign == VAlign::Bottom ? QStringLiteral("bottom") : QStringLiteral("top"));
            const int sides[4] = {0, 1, 2, 3};
            static const char *const names[4] = {"fo:border-top", "fo:border-bottom", "fo:border-left", "fo:border-right"};
            for (int k = 0; k < 4; ++k) {
                const Stroke edge = edgeOf(t, r, c, sides[k]);
                props += attrOf(names[k], borderOf(edge, resolve(edge.color)));
            }
            const Fill fill = cell.fill.isNone() ? t.fill : cell.fill;
            props += fillAttrs(fill, QSizeF(t.colW[c], t.rowH[r]));
            if (fill.type == Fill::Solid && resolve(fill.color).isValid() && fill.transparency < 0.5) props += attrOf("fo:background-color", hexOf(resolve(fill.color)));
            const QString cellStyle = sc.styles->style(QStringLiteral("table-cell"), QStringLiteral("ce"), QString(), QStringLiteral("<style:table-cell-properties%1/>").arg(props));
            x.open("table:table-cell").attr("table:style-name", cellStyle).attr("office:value-type", "string");
            if (cell.colSpan > 1) x.attr("table:number-columns-spanned", std::min(cell.colSpan, t.cols - c));
            if (cell.rowSpan > 1) x.attr("table:number-rows-spanned", std::min(cell.rowSpan, t.rows - r));
            const QTextDocument *story = m_doc.storyDoc(cell.storyId);
            if (story && story->characterCount() > 1) writeStory(x, sc, story, opts);
            else x.leaf("text:p");
            x.close();
        }
        x.close();
    }
    x.close();
    writeInside(x, t);
    x.close();
}

} // namespace jp::deck::odpw
