// Writing DrawingML (drawingml_write.h): placed objects as shape tree
// elements. The text (drawingml_write_text.cpp) and what the files share
// (drawingml_write_util.h) are beside this.

#include "office/drawingml_write.h"

#include "core/shapetypes.h"
#include "office/drawingml_geometry.h"
#include "office/drawingml_write_util.h"
#include "office/xml.h"
#include "render/shapes.h"

#include <QCoreApplication>
#include <QTextDocument>
#include <algorithm>
#include <climits>
#include <cmath>

namespace jp::office {

using namespace dmlwrite;

namespace {

QString say(const char *text) { return QCoreApplication::translate("DrawingWriter", text); }

void begin(QXmlStreamWriter &w, const char *name) { w.writeStartElement(QLatin1String(name)); }
void attribute(QXmlStreamWriter &w, const char *name, const QString &value) { w.writeAttribute(QLatin1String(name), value); }
void attribute(QXmlStreamWriter &w, const char *name, qint64 value) { w.writeAttribute(QLatin1String(name), QString::number(value)); }
// An element with nothing in it.
void leaf(QXmlStreamWriter &w, const char *name)
{
    begin(w, name);
    w.writeEndElement();
}

// ---- fills, outlines, and effects

// The standard's pattern for each of the library's, the nearest in density or direction.
const char *patternName(int index)
{
    static const char *const names[] = {"pct90", "pct80", "pct70", "pct50", "pct25", "pct10", "pct5", "horz", "vert", "cross", "upDiag", "dnDiag", "diagCross"};
    return names[std::clamp(index, 0, 12)];
}

void writeSolid(QXmlStreamWriter &w, const ColorRef &color, double transparency, const DrawingWriteContext &ctx)
{
    const QColor c = resolve(color, ctx);
    if (!c.isValid()) {
        leaf(w, "a:noFill");
        return;
    }
    begin(w, "a:solidFill");
    writeColor(w, c, transparency);
    w.writeEndElement();
}

void writeGradient(QXmlStreamWriter &w, const Fill &fill, const DrawingWriteContext &ctx)
{
    QVector<GradientStop> stops = fill.stops;
    if (stops.isEmpty()) {
        stops.append({0, fill.color, fill.transparency});
        stops.append({1, fill.color2.isNone() ? ColorRef::rgb(Qt::white) : fill.color2, fill.transparency});
    }
    if (stops.size() == 1) {
        writeSolid(w, stops[0].color, stops[0].transparency, ctx);
        return;
    }
    std::stable_sort(stops.begin(), stops.end(), [](const GradientStop &a, const GradientStop &b) { return a.pos < b.pos; });
    begin(w, "a:gradFill");
    attribute(w, "rotWithShape", QStringLiteral("1"));
    begin(w, "a:gsLst");
    for (const GradientStop &s : std::as_const(stops)) {
        const QColor c = resolve(s.color, ctx);
        begin(w, "a:gs");
        attribute(w, "pos", std::clamp<qint64>(percent(s.pos), 0, 100000));
        writeColor(w, c.isValid() ? c : QColor(Qt::black), c.isValid() ? s.transparency : 1.0);
        w.writeEndElement();
    }
    w.writeEndElement();
    if (fill.gradType == Fill::Linear) {
        begin(w, "a:lin");
        attribute(w, "ang", angle(fill.angle));
        attribute(w, "scaled", QStringLiteral("0"));
        w.writeEndElement();
    } else {
        begin(w, "a:path");
        attribute(w, "path", fill.gradType == Fill::Radial ? QStringLiteral("circle") : fill.gradType == Fill::PathGrad ? QStringLiteral("shape") : QStringLiteral("rect"));
        begin(w, "a:fillToRect");
        for (const char *side : {"l", "t", "r", "b"}) attribute(w, side, QStringLiteral("50000"));
        w.writeEndElement();
        w.writeEndElement();
    }
    w.writeEndElement();
}

// The color effects of a picture (the children of a:blip), as they were listed.
void writeBlipEffects(QXmlStreamWriter &w, const QVector<BlipEffect> &effects)
{
    auto holding = [&](const char *name, const QColor &color) {
        begin(w, name);
        writeColor(w, color);
        w.writeEndElement();
    };
    for (const BlipEffect &e : effects) {
        switch (e.kind) {
        case BlipEffect::Luminance:
            begin(w, "a:lum");
            if (e.brightness != 0) attribute(w, "bright", percent(std::clamp(e.brightness, -1.0, 1.0)));
            if (e.contrast != 0) attribute(w, "contrast", percent(std::clamp(e.contrast, -1.0, 1.0)));
            w.writeEndElement();
            break;
        case BlipEffect::Grayscale: leaf(w, "a:grayscl"); break;
        case BlipEffect::BiLevel:
            begin(w, "a:biLevel");
            attribute(w, "thresh", std::clamp<qint64>(percent(e.threshold), 0, 100000));
            w.writeEndElement();
            break;
        case BlipEffect::ColorChange:
            begin(w, "a:clrChange");
            if (!e.useAlpha) attribute(w, "useA", QStringLiteral("0"));
            holding("a:clrFrom", e.color);
            holding("a:clrTo", e.color2);
            w.writeEndElement();
            break;
        case BlipEffect::ColorReplace:
            begin(w, "a:clrRepl");
            writeColor(w, e.color);
            w.writeEndElement();
            break;
        case BlipEffect::Duotone:
            begin(w, "a:duotone");
            writeColor(w, e.color);
            writeColor(w, e.color2);
            w.writeEndElement();
            break;
        case BlipEffect::Hsl:
            begin(w, "a:hsl");
            attribute(w, "hue", angle(e.hue));
            attribute(w, "sat", percent(std::clamp(e.saturation, -1.0, 1.0)));
            attribute(w, "lum", percent(std::clamp(e.lightness, -1.0, 1.0)));
            w.writeEndElement();
            break;
        case BlipEffect::Tint:
            begin(w, "a:tint");
            attribute(w, "hue", angle(e.hue));
            attribute(w, "amt", percent(std::clamp(e.amount, -1.0, 1.0)));
            w.writeEndElement();
            break;
        }
    }
}

// A picture fill (a:blipFill); false if the picture has no bytes to store.
bool writePictureFill(QXmlStreamWriter &w, const Fill &fill, DrawingWriteContext &ctx, bool tiled)
{
    const QString rId = relatePicture(ctx, fill.imageId);
    if (rId.isEmpty()) return false;
    begin(w, "a:blipFill");
    attribute(w, "rotWithShape", QStringLiteral("1"));
    begin(w, "a:blip");
    attribute(w, "r:embed", rId);
    if (fill.transparency > 0) {
        begin(w, "a:alphaModFix");
        attribute(w, "amt", std::clamp<qint64>(percent(1.0 - fill.transparency), 0, 100000));
        w.writeEndElement();
    }
    writeBlipEffects(w, fill.blipEffects);
    w.writeEndElement();
    if (tiled) {
        const qint64 scale = std::clamp<qint64>(percent(fill.tileScale > 0 ? fill.tileScale : 1.0), 1000, 100000000);
        begin(w, "a:tile");
        attribute(w, "tx", QStringLiteral("0"));
        attribute(w, "ty", QStringLiteral("0"));
        attribute(w, "sx", scale);
        attribute(w, "sy", scale);
        attribute(w, "flip", QStringLiteral("none"));
        attribute(w, "algn", QStringLiteral("tl"));
        w.writeEndElement();
    } else {
        begin(w, "a:stretch");
        leaf(w, "a:fillRect");
        w.writeEndElement();
    }
    w.writeEndElement();
    return true;
}

const char *arrowName(Arrow a)
{
    switch (a) {
    case Arrow::Triangle: return "triangle";
    case Arrow::Open: return "arrow";
    case Arrow::Stealth: return "stealth";
    case Arrow::Diamond: return "diamond";
    case Arrow::Oval: return "oval";
    case Arrow::None: break;
    }
    return "none";
}

void writeArrowEnd(QXmlStreamWriter &w, const char *tag, Arrow arrow, int size)
{
    if (arrow == Arrow::None) return;
    static const char *const sizes[] = {"sm", "med", "lg"};
    begin(w, tag);
    attribute(w, "type", QLatin1String(arrowName(arrow)));
    attribute(w, "w", QLatin1String(sizes[std::clamp(size, 0, 2)]));
    attribute(w, "len", QLatin1String(sizes[std::clamp(size, 0, 2)]));
    w.writeEndElement();
}

void writeEffects(QXmlStreamWriter &w, const Effects &fx, const DrawingWriteContext &ctx)
{
    const bool shadow = fx.shadow.on, glow = fx.glow.on, soft = fx.softEdge > 0, reflection = fx.reflection.on;
    if (!shadow && !glow && !soft && !reflection) return;
    auto color = [&](const ColorRef &c, double transparency) {
        const QColor rgb = resolve(c, ctx);
        writeColor(w, rgb.isValid() ? rgb : QColor(Qt::black), transparency);
    };
    begin(w, "a:effectLst");
    if (glow) {
        begin(w, "a:glow");
        attribute(w, "rad", extent(std::clamp(fx.glow.size, 0.0, 1584.0)));
        color(fx.glow.color, fx.glow.transparency);
        w.writeEndElement();
    }
    if (shadow) {
        begin(w, fx.shadow.inner ? "a:innerShdw" : "a:outerShdw");
        attribute(w, "blurRad", extent(std::clamp(fx.shadow.blur, 0.0, 1584.0)));
        attribute(w, "dist", extent(std::clamp(fx.shadow.distance, 0.0, 1584.0)));
        attribute(w, "dir", angle(fx.shadow.angle));
        if (!fx.shadow.inner) {
            if (fx.shadow.size != 100) {
                attribute(w, "sx", std::clamp<qint64>(percent(fx.shadow.size / 100), 1000, 1000000));
                attribute(w, "sy", std::clamp<qint64>(percent(fx.shadow.size / 100), 1000, 1000000));
            }
            attribute(w, "algn", QStringLiteral("ctr"));
            attribute(w, "rotWithShape", QStringLiteral("0"));
        }
        color(fx.shadow.color, fx.shadow.transparency);
        w.writeEndElement();
    }
    if (reflection) {
        // Turned over (sy of -100%) under the object, from its start opacity to nothing at endPos of its height.
        begin(w, "a:reflection");
        attribute(w, "blurRad", extent(std::clamp(fx.reflection.blur, 0.0, 1584.0)));
        attribute(w, "stA", std::clamp<qint64>(percent(1.0 - fx.reflection.transparency), 0, 100000));
        attribute(w, "endA", QStringLiteral("0"));
        attribute(w, "endPos", std::clamp<qint64>(percent(fx.reflection.size), 0, 100000));
        attribute(w, "dist", extent(std::clamp(fx.reflection.distance, 0.0, 1584.0)));
        attribute(w, "dir", QStringLiteral("5400000"));
        attribute(w, "sy", QStringLiteral("-100000"));
        attribute(w, "algn", QStringLiteral("bl"));
        attribute(w, "rotWithShape", QStringLiteral("0"));
        w.writeEndElement();
    }
    if (soft) {
        begin(w, "a:softEdge");
        attribute(w, "rad", extent(std::clamp(fx.softEdge, 0.0, 1584.0)));
        w.writeEndElement();
    }
    w.writeEndElement();
}

// An outline as the element `tag` (a:ln, or a table cell's a:lnL and its kin).
void writeOutline(QXmlStreamWriter &w, const char *tag, const Stroke &stroke, DrawingWriteContext &ctx)
{
    begin(w, tag);
    const QColor color = resolve(stroke.color, ctx);
    if (stroke.isNone() || !color.isValid()) {
        // No line, of the width it had.
        if (stroke.width > 0) attribute(w, "w", std::clamp<qint64>(extent(std::clamp(stroke.width, 0.0, 1584.0)), 0, 20116800));
        leaf(w, "a:noFill");
        w.writeEndElement();
        return;
    }
    attribute(w, "w", std::clamp<qint64>(extent(std::clamp(stroke.width, 0.0, 1584.0)), 0, 20116800));
    attribute(w, "cap", stroke.cap == Qt::RoundCap ? QStringLiteral("rnd") : stroke.cap == Qt::SquareCap ? QStringLiteral("sq") : QStringLiteral("flat"));
    static const char *const compounds[] = {"sng", "dbl", "thickThin", "thinThick", "tri"};
    attribute(w, "cmpd", QLatin1String(compounds[std::clamp(int(stroke.compound), 0, 4)]));
    attribute(w, "algn", stroke.inset ? QStringLiteral("in") : QStringLiteral("ctr"));
    if (stroke.hasGradient()) writeGradient(w, stroke.gradient, ctx);
    else writeSolid(w, stroke.color, stroke.transparency, ctx);
    static const char *const dashes[] = {"solid", "dot", "sysDot", "dash", "dashDot", "lgDash", "lgDashDot", "lgDashDotDot"};
    begin(w, "a:prstDash");
    attribute(w, "val", QLatin1String(dashes[std::clamp(int(stroke.dash), 0, 7)]));
    w.writeEndElement();
    switch (stroke.join) {
    case Qt::RoundJoin: leaf(w, "a:round"); break;
    case Qt::BevelJoin: leaf(w, "a:bevel"); break;
    default:
        begin(w, "a:miter");
        attribute(w, "lim", QStringLiteral("800000"));
        w.writeEndElement();
        break;
    }
    writeArrowEnd(w, "a:headEnd", stroke.startArrow, stroke.startSize);
    writeArrowEnd(w, "a:tailEnd", stroke.endArrow, stroke.endSize);
    w.writeEndElement();
}

// ---- places

// Where an object is: its frame, turned and flipped about its center.
void writeTransform(QXmlStreamWriter &w, const QRectF &rect, double rotation, bool flipH, bool flipV, bool group = false)
{
    begin(w, "a:xfrm");
    if (const qint64 rot = angle(rotation)) attribute(w, "rot", rot);
    if (flipH) attribute(w, "flipH", QStringLiteral("1"));
    if (flipV) attribute(w, "flipV", QStringLiteral("1"));
    begin(w, "a:off");
    attribute(w, "x", position(rect.x()));
    attribute(w, "y", position(rect.y()));
    w.writeEndElement();
    begin(w, "a:ext");
    attribute(w, "cx", extent(rect.width()));
    attribute(w, "cy", extent(rect.height()));
    w.writeEndElement();
    if (group) {
        // What is in a group is placed in the page's own space.
        begin(w, "a:chOff");
        attribute(w, "x", position(rect.x()));
        attribute(w, "y", position(rect.y()));
        w.writeEndElement();
        begin(w, "a:chExt");
        attribute(w, "cx", extent(rect.width()));
        attribute(w, "cy", extent(rect.height()));
        w.writeEndElement();
    }
    w.writeEndElement();
}

// A preset geometry with its adjustments.
void writePreset(QXmlStreamWriter &w, const QString &prst, const QVector<QPair<QString, double>> &adjustments = {})
{
    begin(w, "a:prstGeom");
    attribute(w, "prst", prst);
    begin(w, "a:avLst");
    for (const auto &a : adjustments) {
        begin(w, "a:gd");
        attribute(w, "name", a.first);
        attribute(w, "fmla", QStringLiteral("val %1").arg(rounded(a.second)));
        w.writeEndElement();
    }
    w.writeEndElement();
    w.writeEndElement();
}

// A freeform: the path in the frame's own EMU. `open` shapes (arcs and brackets) are not filled.
void writeFreeform(QXmlStreamWriter &w, const QPainterPath &path, const QSizeF &size, const QRectF &textRect, bool open)
{
    const qint64 pw = std::max<qint64>(1, extent(size.width())), ph = std::max<qint64>(1, extent(size.height()));
    begin(w, "a:custGeom");
    leaf(w, "a:avLst");
    leaf(w, "a:gdLst");
    leaf(w, "a:ahLst");
    leaf(w, "a:cxnLst");
    begin(w, "a:rect");
    if (textRect.isValid()) {
        attribute(w, "l", position(textRect.left()));
        attribute(w, "t", position(textRect.top()));
        attribute(w, "r", position(textRect.right()));
        attribute(w, "b", position(textRect.bottom()));
    } else {
        attribute(w, "l", QStringLiteral("0"));
        attribute(w, "t", QStringLiteral("0"));
        attribute(w, "r", QStringLiteral("r"));
        attribute(w, "b", QStringLiteral("b"));
    }
    w.writeEndElement();
    begin(w, "a:pathLst");
    begin(w, "a:path");
    attribute(w, "w", pw);
    attribute(w, "h", ph);
    if (open) attribute(w, "fill", QStringLiteral("none"));
    auto point = [&](double x, double y) {
        begin(w, "a:pt");
        attribute(w, "x", position(x));
        attribute(w, "y", position(y));
        w.writeEndElement();
    };
    const int count = path.elementCount();
    int i = 0;
    while (i < count) {
        // A piece runs from a move to the next one.
        int end = i + 1;
        while (end < count && path.elementAt(end).type != QPainterPath::MoveToElement) ++end;
        const QPainterPath::Element first = path.elementAt(i);
        // One that goes back to where it began is closed (Qt closes a piece with a line, or with nothing if it is there).
        int last = end;
        bool closed = false;
        if (end - i > 2) {
            const QPainterPath::Element e = path.elementAt(end - 1);
            if (std::fabs(e.x - first.x) < 1e-9 && std::fabs(e.y - first.y) < 1e-9) {
                closed = true;
                if (e.type == QPainterPath::LineToElement) last = end - 1;
            }
        }
        if (end - i > 1) {
            begin(w, "a:moveTo");
            point(first.x, first.y);
            w.writeEndElement();
            for (int k = i + 1; k < last; ++k) {
                const QPainterPath::Element e = path.elementAt(k);
                if (e.type == QPainterPath::LineToElement) {
                    begin(w, "a:lnTo");
                    point(e.x, e.y);
                    w.writeEndElement();
                } else if (e.type == QPainterPath::CurveToElement && k + 2 < end) {
                    begin(w, "a:cubicBezTo");
                    point(e.x, e.y);
                    point(path.elementAt(k + 1).x, path.elementAt(k + 1).y);
                    point(path.elementAt(k + 2).x, path.elementAt(k + 2).y);
                    w.writeEndElement();
                    k += 2;
                }
            }
            if (closed) leaf(w, "a:close");
        }
        i = end;
    }
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndElement();
}

// ---- the placeholder an object stands for

struct Placeholder {
    bool is = false;
    QString type, idx;   // type empty for content ("obj"); idx empty for 0
};

Placeholder placeholderOf(const Item &it)
{
    Placeholder p;
    if (!it.role.startsWith(QLatin1String("ph:"))) return p;
    static const QStringList types = {QStringLiteral("title"), QStringLiteral("body"), QStringLiteral("ctrTitle"), QStringLiteral("subTitle"), QStringLiteral("dt"),
        QStringLiteral("sldNum"), QStringLiteral("ftr"), QStringLiteral("hdr"), QStringLiteral("chart"), QStringLiteral("tbl"), QStringLiteral("clipArt"),
        QStringLiteral("dgm"), QStringLiteral("media"), QStringLiteral("sldImg"), QStringLiteral("pic")};
    p.is = true;
    const QStringList parts = it.role.mid(3).split(QLatin1Char(':'));
    if (types.contains(parts.value(0))) p.type = parts.value(0);
    bool ok = false;
    const qint64 idx = parts.value(1).toLongLong(&ok);
    if (ok && idx > 0 && idx <= 4294967295LL) p.idx = QString::number(idx);
    return p;
}

void writePlaceholder(QXmlStreamWriter &w, const Placeholder &ph)
{
    begin(w, "p:ph");
    if (!ph.type.isEmpty()) attribute(w, "type", ph.type);
    if (!ph.idx.isEmpty()) attribute(w, "idx", ph.idx);
    w.writeEndElement();
}

// ---- the parts of an object that all have

// The number the object is written with: the one it was given before (assignShapeIds),
// else the one a file gave it, if free, else the next.
int takeId(DrawingWriteContext &ctx, const Item &it)
{
    const auto given = ctx.shapeIds.constFind(it.id);
    if (given != ctx.shapeIds.constEnd()) return *given;
    int id = 0;
    if (it.sourceId >= 2 && !ctx.usedIds.contains(it.sourceId)) {
        id = it.sourceId;
    } else {
        id = std::max(ctx.nextShapeId, 2);
        while (id < INT_MAX && (ctx.usedIds.contains(id) || ctx.reservedIds.contains(id))) ++id;
        ctx.nextShapeId = id + 1;
    }
    ctx.usedIds.insert(id);
    ctx.shapeIds.insert(it.id, id);
    return id;
}

// p:cNvPr: the number, the name, the alternative text, and the link of the whole object.
// `playsMedia`: a click on it plays the movie or sound it has, unless it is a link.
void writeIdentity(QXmlStreamWriter &w, const Item &it, int id, const QString &fallbackName, DrawingWriteContext &ctx, bool playsMedia = false)
{
    begin(w, "p:cNvPr");
    attribute(w, "id", id);
    attribute(w, "name", it.name.isEmpty() ? QStringLiteral("%1 %2").arg(fallbackName).arg(id - 1) : clean(it.name));
    if (!it.altText.isEmpty()) attribute(w, "descr", clean(it.altText));
    if (!it.altTitle.isEmpty()) attribute(w, "title", clean(it.altTitle));
    const QString rId = it.hyperlink.isEmpty() ? QString() : relateAddress(ctx, it.hyperlink);
    if (!rId.isEmpty()) {
        begin(w, "a:hlinkClick");
        attribute(w, "r:id", rId);
        w.writeEndElement();
    } else if (playsMedia) {
        begin(w, "a:hlinkClick");
        attribute(w, "r:id", QString());
        attribute(w, "action", QStringLiteral("ppaction://media"));
        w.writeEndElement();
    }
    if (it.decorative) writeDecorative(w);
    w.writeEndElement();
}

// The locks of an object's non-visual properties (the element named by `locks`, in p:cNvSpPr or p:cNvPicPr): that it takes no place in a group
// (a placeholder), and whether its proportions are kept. A shape says it only when it is on (that is what a missing lock means).
void writeLocks(QXmlStreamWriter &w, const char *locks, const Item &it, bool noGroup, bool saysOff)
{
    if (!noGroup && !it.lockAspect && !saysOff) return;
    begin(w, locks);
    if (noGroup) attribute(w, "noGrp", QStringLiteral("1"));
    if (it.lockAspect || saysOff) attribute(w, "noChangeAspect", it.lockAspect ? QStringLiteral("1") : QStringLiteral("0"));
    w.writeEndElement();
}

// A picture's movie or sound, in its p:nvPr after any p:ph.
void writeMedia(QXmlStreamWriter &w, const PictureItem &p, const MediaRelations &media)
{
    if (media.link.isEmpty()) return;
    begin(w, p.mediaKind == QLatin1String("video") ? "a:videoFile" : "a:audioFile");
    attribute(w, "r:link", media.link);
    w.writeEndElement();
    if (media.copy.isEmpty()) return;
    begin(w, "p:extLst");
    begin(w, "p:ext");
    attribute(w, "uri", QStringLiteral("{DAA4B4D4-6D71-4841-9C94-3DE7FCFB9230}"));
    begin(w, "p14:media");
    w.writeAttribute(QStringLiteral("xmlns:p14"), QStringLiteral("http://schemas.microsoft.com/office/powerpoint/2010/main"));
    attribute(w, "r:embed", media.copy);
    if (p.mediaTrimStart > 0 || p.mediaTrimEnd > 0) {
        begin(w, "p14:trim");
        if (p.mediaTrimStart > 0) attribute(w, "st", p.mediaTrimStart);
        if (p.mediaTrimEnd > 0) attribute(w, "end", p.mediaTrimEnd);
        w.writeEndElement();
    }
    if (p.mediaFadeIn > 0 || p.mediaFadeOut > 0) {
        begin(w, "p14:fade");
        if (p.mediaFadeIn > 0) attribute(w, "in", p.mediaFadeIn);
        if (p.mediaFadeOut > 0) attribute(w, "out", p.mediaFadeOut);
        w.writeEndElement();
    }
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndElement();
}

// The properties a shape (and a picture) has beyond its geometry.
void writeLook(QXmlStreamWriter &w, const Item &it, DrawingWriteContext &ctx)
{
    writeFill(w, it.fill, ctx);
    writeLine(w, it.stroke, ctx);
    writeEffects(w, it.fx, ctx);
}

// A margin round text in EMU, held to what a 32-bit number counts.
qint64 inset(double points)
{
    return position(std::clamp(points, -4000.0, 4000.0));
}

// What a text body says of its frame.
struct Body {
    QMarginsF insets;
    VAlign valign = VAlign::Top;
    int columns = 1;
    double columnGap = 0;
    bool vertical = false;
    bool noWrap = false;
    bool centerBlock = false;
    TextItem::Autofit autofit = TextItem::NoAutofit;
    double fitScale = 1.0;
    double fitLineCut = 0;
};

void writeBody(QXmlStreamWriter &w, const Body &b, const QTextDocument *doc, bool placeholder, DrawingWriteContext &ctx)
{
    begin(w, "p:txBody");
    begin(w, "a:bodyPr");
    if (b.vertical) attribute(w, "vert", QStringLiteral("vert"));
    attribute(w, "wrap", b.noWrap ? QStringLiteral("none") : QStringLiteral("square"));
    if (b.centerBlock) attribute(w, "anchorCtr", QStringLiteral("1"));
    attribute(w, "lIns", inset(b.insets.left()));
    attribute(w, "tIns", inset(b.insets.top()));
    attribute(w, "rIns", inset(b.insets.right()));
    attribute(w, "bIns", inset(b.insets.bottom()));
    if (b.columns > 1) {
        attribute(w, "numCol", std::clamp(b.columns, 1, 16));
        attribute(w, "spcCol", extent(std::clamp(b.columnGap, 0.0, 1000.0)));
    }
    attribute(w, "rtlCol", QStringLiteral("0"));
    attribute(w, "anchor", b.valign == VAlign::Middle ? QStringLiteral("ctr") : b.valign == VAlign::Bottom ? QStringLiteral("b") : QStringLiteral("t"));
    switch (b.autofit) {
    case TextItem::NoAutofit: leaf(w, "a:noAutofit"); break;
    case TextItem::GrowBox: leaf(w, "a:spAutoFit"); break;
    case TextItem::BestFit:
    case TextItem::ShrinkOnOverflow:
        begin(w, "a:normAutofit");
        if (b.fitScale < 1.0) attribute(w, "fontScale", std::clamp<qint64>(percent(b.fitScale), 1000, 100000));
        if (b.fitLineCut > 0) attribute(w, "lnSpcReduction", std::clamp<qint64>(percent(b.fitLineCut / 100), 0, 99000));
        w.writeEndElement();
        break;
    }
    w.writeEndElement();
    leaf(w, "a:lstStyle");
    const bool was = ctx.placeholderText;
    ctx.placeholderText = placeholder;
    writeParagraphs(w, doc, ctx);
    ctx.placeholderText = was;
    if (!doc) {
        begin(w, "a:p");
        leaf(w, "a:endParaRPr");
        w.writeEndElement();
    }
    w.writeEndElement();
}

const QTextDocument *storyOf(const QString &storyId, const DrawingWriteContext &ctx)
{
    return storyId.isEmpty() || !ctx.storyDoc ? nullptr : ctx.storyDoc(storyId);
}

// ---- the objects

void writeTextBox(QXmlStreamWriter &w, const TextItem &t, DrawingWriteContext &ctx)
{
    const Placeholder ph = placeholderOf(t);
    const int id = takeId(ctx, t);
    begin(w, "p:sp");
    begin(w, "p:nvSpPr");
    writeIdentity(w, t, id, ph.is ? say("Placeholder") : say("Text Box"), ctx);
    begin(w, "p:cNvSpPr");
    if (!ph.is) attribute(w, "txBox", QStringLiteral("1"));
    writeLocks(w, "a:spLocks", t, ph.is, Item::lockAspectByDefault(t.type()));
    w.writeEndElement();
    begin(w, "p:nvPr");
    if (ph.is) writePlaceholder(w, ph);
    w.writeEndElement();
    w.writeEndElement();
    begin(w, "p:spPr");
    writeTransform(w, t.rect, t.rotation, t.flipH, t.flipV);
    writePreset(w, QStringLiteral("rect"));
    writeLook(w, t, ctx);
    w.writeEndElement();
    Body body;
    body.insets = t.insets;
    body.valign = t.valign;
    body.columns = t.columns;
    body.columnGap = t.columnGap;
    body.vertical = t.vertical;
    body.noWrap = t.noWrap;
    body.centerBlock = t.centerBlock;
    body.autofit = t.autofit;
    body.fitScale = t.fitScale;
    body.fitLineCut = t.fitLineCut;
    writeBody(w, body, storyOf(t.storyId, ctx), ph.is, ctx);
    w.writeEndElement();
}

// The standard's warps of text art, in the order of the drawing format's text shapes they stand for (136 and on).
const char *const kTextWarps[] = {
    "textPlain", "textStop", "textTriangle", "textTriangleInverted", "textChevron", "textChevronInverted", "textRingInside",
    "textRingOutside", "textArchUp", "textArchDown", "textCircle", "textButton", "textArchUpPour", "textArchDownPour",
    "textCirclePour", "textButtonPour", "textCurveUp", "textCurveDown", "textCascadeUp", "textCascadeDown", "textWave1",
    "textWave2", "textDoubleWave1", "textWave4", "textInflate", "textDeflate", "textInflateBottom", "textDeflateBottom",
    "textInflateTop", "textDeflateTop", "textDeflateInflate", "textDeflateInflateDeflate", "textFadeRight", "textFadeLeft",
    "textFadeUp", "textFadeDown", "textSlantUp", "textSlantDown", "textCanUp", "textCanDown",
};

// Text art: a box whose text is warped into its shape (a:prstTxWarp) and
// stretched to fill it as the older text art was (fromWordArt), each line a
// paragraph, the art's fill and outline those of its letters.
void writeTextArt(QXmlStreamWriter &w, const TextArtItem &a, DrawingWriteContext &ctx)
{
    const int id = takeId(ctx, a);
    begin(w, "p:sp");
    begin(w, "p:nvSpPr");
    writeIdentity(w, a, id, say("Text Art"), ctx);
    leaf(w, "p:cNvSpPr");
    leaf(w, "p:nvPr");
    w.writeEndElement();
    begin(w, "p:spPr");
    writeTransform(w, a.rect, a.rotation, a.flipH, a.flipV);
    writePreset(w, QStringLiteral("rect"));
    leaf(w, "a:noFill");
    w.writeEndElement();
    begin(w, "p:txBody");
    begin(w, "a:bodyPr");
    attribute(w, "wrap", QStringLiteral("none"));
    for (const char *side : {"lIns", "tIns", "rIns", "bIns"}) attribute(w, side, QStringLiteral("0"));
    if (a.vertical) attribute(w, "vert", QStringLiteral("wordArtVert"));
    attribute(w, "fromWordArt", QStringLiteral("1"));
    attribute(w, "anchor", QStringLiteral("ctr"));
    const int type = textArtShapeType(a.transform_);
    begin(w, "a:prstTxWarp");
    attribute(w, "prst", QLatin1String(type >= 136 && type < 136 + int(std::size(kTextWarps)) ? kTextWarps[type - 136] : "textPlain"));
    leaf(w, "a:avLst");
    w.writeEndElement();
    leaf(w, "a:normAutofit");
    w.writeEndElement();
    leaf(w, "a:lstStyle");
    static const char *const kAlign[] = {"l", "ctr", "r", "just", "dist", "ctr"};
    const qint64 size = std::clamp<qint64>(std::llround(a.size * 100), 100, 400000);
    for (const QString &line : a.text.split(QLatin1Char('\n'))) {
        begin(w, "a:p");
        begin(w, "a:pPr");
        attribute(w, "algn", QLatin1String(kAlign[std::clamp(a.align, 0, 5)]));
        w.writeEndElement();
        auto runProperties = [&](const char *name) {
            begin(w, name);
            attribute(w, "lang", QStringLiteral("en-US"));
            attribute(w, "sz", size);
            attribute(w, "b", QLatin1String(a.bold ? "1" : "0"));
            attribute(w, "i", QLatin1String(a.italic ? "1" : "0"));
            if (std::abs(a.spacing - 1.0) > 0.001) attribute(w, "spc", std::clamp<qint64>(std::llround((a.spacing - 1.0) * a.size * 100), -400000, 400000));
            writeLine(w, a.stroke, ctx);
            writeFill(w, a.fill, ctx);
            writeEffects(w, a.fx, ctx);
            begin(w, "a:latin");
            attribute(w, "typeface", clean(a.font));
            w.writeEndElement();
            w.writeEndElement();
        };
        if (!line.isEmpty()) {
            begin(w, "a:r");
            runProperties("a:rPr");
            begin(w, "a:t");
            w.writeCharacters(clean(line));
            w.writeEndElement();
            w.writeEndElement();
        }
        runProperties("a:endParaRPr");
        w.writeEndElement();
    }
    w.writeEndElement();
    w.writeEndElement();
}

// The arc-based presets, as the shape's own preset with its adjustments, and the geometry that gives. Empty for any other shape.
Geometry presetOf(const ShapeItem &s, QVector<QPair<QString, double>> *adjustments)
{
    static const QStringList arcBased = {QStringLiteral("arc"), QStringLiteral("chord"), QStringLiteral("pie"), QStringLiteral("blockArc")};
    if (!arcBased.contains(s.shape)) return Geometry();
    *adjustments = presetAdjustments(s.shape, s.adj, s.rect.size());
    QHash<QString, double> given;
    for (const auto &a : *adjustments) given.insert(a.first, a.second);
    return presetGeometry(s.shape, given, s.rect.size());
}

// Whether two paths are the same, point for point, to a hundredth of a point.
bool samePath(const QPainterPath &a, const QPainterPath &b)
{
    if (a.elementCount() != b.elementCount()) return false;
    for (int i = 0; i < a.elementCount(); ++i) {
        const QPainterPath::Element x = a.elementAt(i), y = b.elementAt(i);
        if (x.type != y.type || qAbs(x.x - y.x) > 0.01 || qAbs(x.y - y.y) > 0.01) return false;
    }
    return true;
}

// Whether a path is one of the preset's own paths (a shape that was edited or resized is not).
bool isPresetPath(const Geometry &preset, const QPainterPath &path)
{
    for (const GeometryPath &p : preset.paths)
        if (samePath(p.path, path)) return true;
    return false;
}

// The curve of an arc that was read as its sector (first) and its curve: the two make the one preset, and the curve is the one
// part that says what it is. Null for any other group.
const ShapeItem *arcCurve(const GroupItem &g)
{
    if (g.children.size() != 2 || g.children[0]->type() != ItemType::Shape || g.children[1]->type() != ItemType::Shape) return nullptr;
    const auto &sector = static_cast<const ShapeItem &>(*g.children[0]);
    const auto &curve = static_cast<const ShapeItem &>(*g.children[1]);
    if (curve.shape != QLatin1String("arc") || sector.customPath.isEmpty() || curve.customPath.isEmpty() || sector.rect != curve.rect) return nullptr;
    QVector<QPair<QString, double>> adjustments;
    const Geometry preset = presetOf(curve, &adjustments);
    if (preset.paths.size() != 2 || !samePath(preset.paths[0].path, sector.customPath) || !samePath(preset.paths[1].path, curve.customPath)) return nullptr;
    return &curve;
}

void writeForm(QXmlStreamWriter &w, const ShapeItem &s, DrawingWriteContext &ctx)
{
    const Placeholder ph = placeholderOf(s);
    const int id = takeId(ctx, s);
    const QSizeF size = s.rect.size();
    begin(w, "p:sp");
    begin(w, "p:nvSpPr");
    writeIdentity(w, s, id, ph.is ? say("Placeholder") : say("Shape"), ctx);
    begin(w, "p:cNvSpPr");
    writeLocks(w, "a:spLocks", s, ph.is, Item::lockAspectByDefault(s.type()));
    w.writeEndElement();
    begin(w, "p:nvPr");
    if (ph.is) writePlaceholder(w, ph);
    w.writeEndElement();
    w.writeEndElement();
    begin(w, "p:spPr");
    writeTransform(w, s.rect, s.rotation, s.flipH, s.flipV);
    QVector<QPair<QString, double>> adjustments;
    const Geometry preset = presetOf(s, &adjustments);
    if (!s.customPath.isEmpty() && !isPresetPath(preset, s.customPath)) {
        writeFreeform(w, s.customPath, size, QRectF(), false);
    } else if (!s.customPath.isEmpty()) {
        // A preset read from a file, whose path is still the preset's own, is written as that preset.
        writePreset(w, s.shape, adjustments);
    } else {
        const QString prst = presetForShape(s.shape);
        if (!prst.isEmpty()) {
            writePreset(w, prst, presetAdjustments(prst, s.adj, size));
        } else {
            // A shape the standard has no name for is drawn as it is here.
            const ShapeDef *def = shapeDef(s.shape);
            writeFreeform(w, shapePath(s.shape, size, s.adj), size, shapeTextRect(s.shape, size, s.adj), def && def->open);
        }
    }
    writeLook(w, s, ctx);
    w.writeEndElement();
    // The text body is there with no text in it, to keep where text would sit.
    Body body;
    body.insets = s.insets;
    body.valign = s.valign;
    body.noWrap = s.noWrap;
    body.centerBlock = s.centerBlock;
    body.columns = s.columns;
    body.columnGap = s.columnGap;
    if (s.shrinks) {
        body.autofit = TextItem::ShrinkOnOverflow;
        body.fitScale = s.fitScale;
        body.fitLineCut = s.fitLineCut;
    }
    writeBody(w, body, storyOf(s.storyId, ctx), ph.is, ctx);
    w.writeEndElement();
}

void writePicture(QXmlStreamWriter &w, const PictureItem &p, DrawingWriteContext &ctx, QStringList *skipped)
{
    const Placeholder ph = placeholderOf(p);
    if (p.imageId.isEmpty()) {
        // The place for a picture, with none in it yet.
        const int id = takeId(ctx, p);
        begin(w, "p:sp");
        begin(w, "p:nvSpPr");
        writeIdentity(w, p, id, say("Picture Placeholder"), ctx);
        begin(w, "p:cNvSpPr");
        begin(w, "a:spLocks");
        attribute(w, "noGrp", QStringLiteral("1"));
        w.writeEndElement();
        w.writeEndElement();
        begin(w, "p:nvPr");
        Placeholder slot = ph;
        slot.is = true;
        if (slot.type.isEmpty()) slot.type = QStringLiteral("pic");
        writePlaceholder(w, slot);
        w.writeEndElement();
        w.writeEndElement();
        begin(w, "p:spPr");
        writeTransform(w, p.rect, p.rotation, p.flipH, p.flipV);
        writePreset(w, QStringLiteral("rect"));
        writeLook(w, p, ctx);
        w.writeEndElement();
        w.writeEndElement();
        return;
    }
    const QString rId = relatePicture(ctx, p.imageId);
    if (rId.isEmpty()) {
        if (skipped) skipped->append(say("\"%1\": a picture that can't be read was left out.").arg(p.name.isEmpty() ? p.id : p.name));
        return;
    }
    const int id = takeId(ctx, p);
    const MediaRelations media = relateMedia(ctx, p);
    begin(w, "p:pic");
    begin(w, "p:nvPicPr");
    writeIdentity(w, p, id, say("Picture"), ctx, !media.link.isEmpty());
    begin(w, "p:cNvPicPr");
    writeLocks(w, "a:picLocks", p, false, Item::lockAspectByDefault(p.type()));
    w.writeEndElement();
    begin(w, "p:nvPr");
    if (ph.is) writePlaceholder(w, ph);
    writeMedia(w, p, media);
    w.writeEndElement();
    w.writeEndElement();

    begin(w, "p:blipFill");
    begin(w, "a:blip");
    attribute(w, "r:embed", rId);
    if (p.transparency > 0) {
        begin(w, "a:alphaModFix");
        attribute(w, "amt", std::clamp<qint64>(percent(1.0 - p.transparency), 0, 100000));
        w.writeEndElement();
    }
    // The settings in the order they are drawn, then what is beside them (colorEffects), so that the file lists them as they are drawn.
    writeBlipEffects(w, p.colorEffects());
    // An SVG picture: the PNG above is for programs that don't draw SVG, and the SVG itself is linked here, as the presentation
    // program links it, last in the blip.
    if (const QString svgId = relateSvg(ctx, p.imageId); !svgId.isEmpty()) {
        begin(w, "a:extLst");
        begin(w, "a:ext");
        attribute(w, "uri", QStringLiteral("{96DAC541-7B7A-43D3-8B79-37D633B846F1}"));
        begin(w, "asvg:svgBlip");
        w.writeAttribute(QStringLiteral("xmlns:asvg"), QStringLiteral("http://schemas.microsoft.com/office/drawing/2016/SVG/main"));
        attribute(w, "r:embed", svgId);
        w.writeEndElement();
        w.writeEndElement();
        w.writeEndElement();
    }
    w.writeEndElement();
    // The part of the picture that shows: what the frame cuts off (or, when it is
    // negative, the room left around it).
    const QRectF whole = p.imgRect, frame(0, 0, p.rect.width(), p.rect.height());
    if (whole.width() > 0 && whole.height() > 0 && !frame.isEmpty() && whole != frame) {
        const qint64 l = percent((frame.left() - whole.left()) / whole.width()), t = percent((frame.top() - whole.top()) / whole.height());
        const qint64 r = percent((whole.right() - frame.right()) / whole.width()), b = percent((whole.bottom() - frame.bottom()) / whole.height());
        if (l || t || r || b) {
            begin(w, "a:srcRect");
            if (l) attribute(w, "l", l);
            if (t) attribute(w, "t", t);
            if (r) attribute(w, "r", r);
            if (b) attribute(w, "b", b);
            w.writeEndElement();
        }
    }
    begin(w, "a:stretch");
    leaf(w, "a:fillRect");
    w.writeEndElement();
    w.writeEndElement();

    begin(w, "p:spPr");
    writeTransform(w, p.rect, p.rotation, p.flipH, p.flipV);
    const QString mask = presetForShape(p.maskShape);
    writePreset(w, mask.isEmpty() ? QStringLiteral("rect") : mask);
    writeLook(w, p, ctx);
    w.writeEndElement();
    w.writeEndElement();
}

// How many connection points the standard gives an object's shape (the
// counterpart of how a connector is read): four on the sides of a rectangle,
// eight round an ellipse, and none for a shape whose points aren't known here.
int connectionPoints(const Item &target)
{
    static const QStringList fourPoints = {QStringLiteral("rect"), QStringLiteral("roundRect"), QStringLiteral("snip1"), QStringLiteral("snip2same"),
        QStringLiteral("snip2diag"), QStringLiteral("snipRound"), QStringLiteral("round1"), QStringLiteral("round2same"), QStringLiteral("round2diag"),
        QStringLiteral("diamond"), QStringLiteral("fcProcess"), QStringLiteral("fcAltProcess"), QStringLiteral("fcDecision"), QStringLiteral("fcPredefined"),
        QStringLiteral("fcInternalStorage")};
    static const QStringList eightPoints = {QStringLiteral("ellipse"), QStringLiteral("fcConnector"), QStringLiteral("fcSummingJunction"), QStringLiteral("fcOr")};
    if (target.type() == ItemType::Text || target.type() == ItemType::Picture) return 4;
    if (target.type() != ItemType::Shape) return 0;
    const auto &s = static_cast<const ShapeItem &>(target);
    if (!s.customPath.isEmpty()) return 0;
    return fourPoints.contains(s.shape) ? 4 : eightPoints.contains(s.shape) ? 8 : 0;
}

// The frame that puts a connector's ends where they are (the counterpart of how
// it is read): turned a quarter when its first leg is upright.
struct Placed {
    QRectF rect;
    double rotation = 0;
    bool flipH = false, flipV = false;
};

Placed connectorFrame(const LineItem &l, bool bent, bool corner)
{
    Placed p;
    const QRectF box = QRectF(l.p1, l.p2).normalized();
    if (!(bent && l.startVertical)) {
        p.rect = box;
        p.flipH = l.p2.x() < l.p1.x();
        p.flipV = l.p2.y() < l.p1.y();
        return p;
    }
    // A quarter turn: the frame is the box on its side, and one of the turns and
    // flips puts the ends (and, with one corner, the corner) in their places.
    p.rect = QRectF(box.center().x() - box.height() / 2, box.center().y() - box.width() / 2, box.height(), box.width());
    p.rotation = 90;
    const double tolerance = 1e-6 * std::max(1.0, std::max(box.width(), box.height()));
    for (double rotation : {90.0, 270.0})
        for (int flips = 0; flips < 4; ++flips) {
            ShapeItem trial;
            trial.rect = p.rect;
            trial.rotation = rotation;
            trial.flipH = flips & 1;
            trial.flipV = flips & 2;
            const QTransform t = trial.transform();
            const QPointF a = t.map(QPointF(0, 0)), b = t.map(QPointF(p.rect.width(), p.rect.height()));
            const QPointF c = t.map(QPointF(p.rect.width(), 0));
            auto same = [&](const QPointF &x, const QPointF &y) { return std::fabs(x.x() - y.x()) <= tolerance && std::fabs(x.y() - y.y()) <= tolerance; };
            if (same(a, l.p1) && same(b, l.p2) && (!corner || same(c, QPointF(l.p1.x(), l.p2.y())))) {
                p.rotation = rotation;
                p.flipH = trial.flipH;
                p.flipV = trial.flipV;
                return p;
            }
        }
    return p;
}

void writeConnector(QXmlStreamWriter &w, const LineItem &l, DrawingWriteContext &ctx)
{
    const int id = takeId(ctx, l);
    const bool bent = l.route != LineItem::Straight;
    // Ends that leave the same way make an S; a corner has one level and one upright.
    const bool corner = bent && l.startVertical != l.endVertical;
    QString prst = QStringLiteral("line");
    if (bent) prst = QStringLiteral("%1Connector%2").arg(l.route == LineItem::Elbow ? QStringLiteral("bent") : QStringLiteral("curved")).arg(corner ? 2 : 3);
    else if (!l.start.id.isEmpty() || !l.end.id.isEmpty()) prst = QStringLiteral("straightConnector1");
    const Placed frame = connectorFrame(l, bent, corner);

    begin(w, "p:cxnSp");
    begin(w, "p:nvCxnSpPr");
    writeIdentity(w, l, id, say("Line"), ctx);
    begin(w, "p:cNvCxnSpPr");
    for (const auto &end : {qMakePair("a:stCxn", l.start), qMakePair("a:endCxn", l.end)}) {
        const auto target = ctx.shapeIds.constFind(end.second.id);
        const auto kind = ctx.connectionSites.constFind(end.second.id);
        if (end.second.id.isEmpty() || target == ctx.shapeIds.constEnd() || kind == ctx.connectionSites.constEnd()) continue;
        // The index of the site among the standard's, for a shape of four or eight points.
        const int idx = *kind == 4 ? end.second.site : *kind == 8 ? end.second.site * 2 : -1;
        if (idx < 0 || end.second.site < 0 || end.second.site >= kConnectionSites) continue;
        begin(w, end.first);
        attribute(w, "id", *target);
        attribute(w, "idx", idx);
        w.writeEndElement();
    }
    w.writeEndElement();
    leaf(w, "p:nvPr");
    w.writeEndElement();
    begin(w, "p:spPr");
    writeTransform(w, frame.rect, frame.rotation, frame.flipH, frame.flipV);
    QVector<QPair<QString, double>> adjustments;
    if (bent && !corner && std::fabs(l.bend - 0.5) > 1e-6) adjustments.append({QStringLiteral("adj1"), l.bend * 100000});
    writePreset(w, prst, adjustments);
    writeLine(w, l.stroke, ctx);
    writeEffects(w, l.fx, ctx);
    w.writeEndElement();
    w.writeEndElement();
}

void writeGroup(QXmlStreamWriter &w, const GroupItem &g, DrawingWriteContext &ctx, QStringList *skipped)
{
    if (g.children.empty()) {
        if (skipped) skipped->append(say("\"%1\": a group with nothing in it was left out.").arg(g.name.isEmpty() ? g.id : g.name));
        return;
    }
    // An arc read from a file is written as the one preset it is: its sector's fill and effects are the arc's.
    if (const ShapeItem *curve = arcCurve(g)) {
        const auto &sector = static_cast<const ShapeItem &>(*g.children[0]);
        ShapeItem arc = *curve;
        arc.customPath = QPainterPath();
        arc.fill = sector.fill;
        arc.fx = sector.fx;
        arc.storyId = sector.storyId;
        writeForm(w, arc, ctx);
        return;
    }
    const int id = takeId(ctx, g);
    begin(w, "p:grpSp");
    begin(w, "p:nvGrpSpPr");
    writeIdentity(w, g, id, say("Group"), ctx);
    leaf(w, "p:cNvGrpSpPr");
    leaf(w, "p:nvPr");
    w.writeEndElement();
    begin(w, "p:grpSpPr");
    writeTransform(w, g.bounds(), 0, false, false, true);
    w.writeEndElement();
    for (const ItemPtr &c : g.children) writeShape(w, *c, ctx, skipped);
    w.writeEndElement();
}

// ---- tables

// The cell that a hidden cell of a merged one is under, for the way it is hidden.
struct Hidden {
    bool across = false, down = false;
};

Hidden hiddenUnder(const TableItem &t, int row, int col)
{
    Hidden h;
    for (int r0 = row; r0 >= 0; --r0)
        for (int c0 = col; c0 >= 0; --c0) {
            const TableCell &c = t.cell(r0, c0);
            if (c.covered || r0 + c.rowSpan <= row || c0 + c.colSpan <= col) continue;
            h.across = col > c0;
            h.down = row > r0;
            return h;
        }
    h.across = true;
    return h;
}

// The line on one side of a cell: the cell's own, else the neighbor's on that same edge (what a person sees is one line, whichever cell holds it).
Stroke edgeOf(const TableItem &t, int row, int col, int side)
{
    const TableCell &c = t.cell(row, col);
    const Stroke own = side == 0 ? c.border.top : side == 1 ? c.border.bottom : side == 2 ? c.border.left : c.border.right;
    // A line round the whole table is on the cells at its edges.
    const bool outer = (side == 0 && row == 0) || (side == 1 && row + c.rowSpan >= t.rows) || (side == 2 && col == 0) || (side == 3 && col + c.colSpan >= t.cols);
    if (outer && !t.stroke.isNone()) return t.stroke;
    // The cell after (below or to the right) is painted later, and has the last word.
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

void writeCell(QXmlStreamWriter &w, const TableItem &t, int row, int col, DrawingWriteContext &ctx)
{
    const TableCell &c = t.cell(row, col);
    begin(w, "a:tc");
    if (c.covered) {
        const Hidden h = hiddenUnder(t, row, col);
        if (h.across) attribute(w, "hMerge", QStringLiteral("1"));
        if (h.down) attribute(w, "vMerge", QStringLiteral("1"));
    } else {
        if (c.colSpan > 1) attribute(w, "gridSpan", std::min(c.colSpan, t.cols - col));
        if (c.rowSpan > 1) attribute(w, "rowSpan", std::min(c.rowSpan, t.rows - row));
    }
    begin(w, "a:txBody");
    leaf(w, "a:bodyPr");
    leaf(w, "a:lstStyle");
    const QTextDocument *first = c.covered ? nullptr : storyOf(c.storyId, ctx);
    const QTextDocument *second = c.covered || !c.divided() ? nullptr : storyOf(c.storyId2, ctx);
    writeParagraphs(w, first, ctx);
    // A cell divided by a diagonal has the text of its second half after the first.
    if (second && !second->isEmpty()) writeParagraphs(w, second, ctx);
    if (!first) {
        begin(w, "a:p");
        leaf(w, "a:endParaRPr");
        w.writeEndElement();
    }
    w.writeEndElement();

    begin(w, "a:tcPr");
    attribute(w, "marL", inset(c.margins.left()));
    attribute(w, "marR", inset(c.margins.right()));
    attribute(w, "marT", inset(c.margins.top()));
    attribute(w, "marB", inset(c.margins.bottom()));
    if (c.vertical) attribute(w, "vert", QStringLiteral("vert"));
    attribute(w, "anchor", c.valign == VAlign::Middle ? QStringLiteral("ctr") : c.valign == VAlign::Bottom ? QStringLiteral("b") : QStringLiteral("t"));
    if (!c.covered) {
        writeOutline(w, "a:lnL", edgeOf(t, row, col, 2), ctx);
        writeOutline(w, "a:lnR", edgeOf(t, row, col, 3), ctx);
        writeOutline(w, "a:lnT", edgeOf(t, row, col, 0), ctx);
        writeOutline(w, "a:lnB", edgeOf(t, row, col, 1), ctx);
        if (c.diagonal == 1) writeOutline(w, "a:lnTlToBr", c.diagonalLine, ctx);
        if (c.diagonal == 2) writeOutline(w, "a:lnBlToTr", c.diagonalLine, ctx);
        // The table's own fill shows through a cell that has none.
        writeFill(w, c.fill.isNone() ? t.fill : c.fill, ctx);
    }
    w.writeEndElement();
    w.writeEndElement();
}

// Whether a table has the cells, columns, and rows to be written.
bool hasCells(const TableItem &t)
{
    return t.rows > 0 && t.cols > 0 && t.cells.size() == t.rows * t.cols && t.colW.size() >= t.cols && t.rowH.size() >= t.rows;
}

void writeTable(QXmlStreamWriter &w, const TableItem &t, DrawingWriteContext &ctx, QStringList *skipped)
{
    if (!hasCells(t)) {
        if (skipped) skipped->append(say("\"%1\": a table with no cells was left out.").arg(t.name.isEmpty() ? t.id : t.name));
        return;
    }
    const int id = takeId(ctx, t);
    double width = 0, height = 0;
    for (int c = 0; c < t.cols; ++c) width += std::max(0.0, t.colW[c]);
    for (int r = 0; r < t.rows; ++r) height += std::max(0.0, t.rowH[r]);
    begin(w, "p:graphicFrame");
    begin(w, "p:nvGraphicFramePr");
    writeIdentity(w, t, id, say("Table"), ctx);
    begin(w, "p:cNvGraphicFramePr");
    begin(w, "a:graphicFrameLocks");
    attribute(w, "noGrp", QStringLiteral("1"));
    w.writeEndElement();
    w.writeEndElement();
    leaf(w, "p:nvPr");
    w.writeEndElement();
    // A table is as wide and tall as its columns and rows; it is not turned.
    begin(w, "p:xfrm");
    begin(w, "a:off");
    attribute(w, "x", position(t.rect.x()));
    attribute(w, "y", position(t.rect.y()));
    w.writeEndElement();
    begin(w, "a:ext");
    attribute(w, "cx", extent(width));
    attribute(w, "cy", extent(height));
    w.writeEndElement();
    w.writeEndElement();
    begin(w, "a:graphic");
    begin(w, "a:graphicData");
    attribute(w, "uri", QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/table"));
    begin(w, "a:tbl");
    // Every cell says how it looks, so no table style is asked for.
    leaf(w, "a:tblPr");
    begin(w, "a:tblGrid");
    for (int c = 0; c < t.cols; ++c) {
        begin(w, "a:gridCol");
        attribute(w, "w", extent(t.colW[c]));
        w.writeEndElement();
    }
    w.writeEndElement();
    for (int r = 0; r < t.rows; ++r) {
        begin(w, "a:tr");
        attribute(w, "h", extent(t.rowH[r]));
        for (int c = 0; c < t.cols; ++c) writeCell(w, t, r, c, ctx);
        w.writeEndElement();
    }
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndElement();
}

} // namespace

int shapeNumber(DrawingWriteContext &ctx, const Item &item) { return takeId(ctx, item); }

void assignShapeIds(const ItemList &items, DrawingWriteContext &ctx)
{
    walkItems(items, [&](const ItemPtr &it) {
        if (it->type() == ItemType::Table && !hasCells(static_cast<const TableItem &>(*it))) return;
        if (it->type() == ItemType::Group && static_cast<const GroupItem &>(*it).children.empty()) return;
        if (it->type() == ItemType::Picture) {
            const QString image = static_cast<const PictureItem &>(*it).imageId;
            if (!image.isEmpty() && relatePicture(ctx, image).isEmpty()) return;
        }
        takeId(ctx, *it);
        if (const int points = connectionPoints(*it)) ctx.connectionSites.insert(it->id, points);
    });
}

void writeShape(QXmlStreamWriter &w, const Item &item, DrawingWriteContext &ctx, QStringList *skipped)
{
    // What a connector can be attached to: the points the standard gives the shape.
    auto remember = [&](const Item &it) {
        if (const int points = connectionPoints(it)) ctx.connectionSites.insert(it.id, points);
    };
    switch (item.type()) {
    case ItemType::Text:
        writeTextBox(w, static_cast<const TextItem &>(item), ctx);
        remember(item);
        break;
    case ItemType::Shape:
        writeForm(w, static_cast<const ShapeItem &>(item), ctx);
        remember(item);
        break;
    case ItemType::Picture:
        writePicture(w, static_cast<const PictureItem &>(item), ctx, skipped);
        remember(item);
        break;
    case ItemType::Line:
        writeConnector(w, static_cast<const LineItem &>(item), ctx);
        break;
    case ItemType::Group:
        writeGroup(w, static_cast<const GroupItem &>(item), ctx, skipped);
        break;
    case ItemType::Table:
        writeTable(w, static_cast<const TableItem &>(item), ctx, skipped);
        break;
    case ItemType::TextArt:
        writeTextArt(w, static_cast<const TextArtItem &>(item), ctx);
        break;
    }
}

void writeFill(QXmlStreamWriter &w, const Fill &fill, DrawingWriteContext &ctx)
{
    switch (fill.type) {
    case Fill::NoFill:
        leaf(w, "a:noFill");
        break;
    case Fill::Solid:
        writeSolid(w, fill.color, fill.transparency, ctx);
        break;
    case Fill::Gradient:
        writeGradient(w, fill, ctx);
        break;
    case Fill::Picture:
    case Fill::Texture:
        // A picture that can't be had is no fill.
        if (!writePictureFill(w, fill, ctx, fill.tile || fill.type == Fill::Texture)) leaf(w, "a:noFill");
        break;
    case Fill::Pattern: {
        const QColor front = resolve(fill.color, ctx), back = resolve(fill.color2, ctx);
        begin(w, "a:pattFill");
        attribute(w, "prst", QLatin1String(patternName(fill.pattern)));
        begin(w, "a:fgClr");
        writeColor(w, front.isValid() ? front : QColor(Qt::black), fill.transparency);
        w.writeEndElement();
        begin(w, "a:bgClr");
        writeColor(w, back.isValid() ? back : QColor(Qt::white));
        w.writeEndElement();
        w.writeEndElement();
        break;
    }
    }
}

void writeLine(QXmlStreamWriter &w, const Stroke &stroke, DrawingWriteContext &ctx)
{
    writeOutline(w, "a:ln", stroke, ctx);
}

} // namespace jp::office
