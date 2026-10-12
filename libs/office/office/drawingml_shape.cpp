// Shapes, pictures, connectors, and groups (drawingml.h). A first, plain
// reading: position and size, rectangles and text boxes, solid fills and
// lines; the rest follows.

#include "office/drawingml.h"

#include "office/xml.h"

#include <QTextDocument>

namespace jp::office {

namespace {

QDomElement firstChild(const QDomElement &e, const QString &local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == local) return c;
    return QDomElement();
}

// The a:xfrm of the element's properties, or of the first it inherits from that has one.
QDomElement transformOf(const QDomElement &e, const QVector<QDomElement> &inheritsFrom)
{
    for (const QDomElement &from : QVector<QDomElement>{e} + inheritsFrom) {
        QDomElement props = firstChild(from, QStringLiteral("spPr"));
        if (props.isNull()) props = firstChild(from, QStringLiteral("grpSpPr"));
        const QDomElement x = firstChild(props, QStringLiteral("xfrm"));
        if (!x.isNull()) return x;
    }
    return QDomElement();
}

void readTransform(const QDomElement &xfrm, Item &it)
{
    if (xfrm.isNull()) return;
    const QDomElement off = firstChild(xfrm, QStringLiteral("off")), ext = firstChild(xfrm, QStringLiteral("ext"));
    it.rect = QRectF(emuToPoints(parseInt(attr(off, QStringLiteral("x")))), emuToPoints(parseInt(attr(off, QStringLiteral("y")))),
                     emuToPoints(parseInt(attr(ext, QStringLiteral("cx")))), emuToPoints(parseInt(attr(ext, QStringLiteral("cy")))));
    it.rotation = parseAngle(attr(xfrm, QStringLiteral("rot")));
    it.flipH = parseBool(attr(xfrm, QStringLiteral("flipH")));
    it.flipV = parseBool(attr(xfrm, QStringLiteral("flipV")));
}

} // namespace

QString shapeForPreset(const QString &prst)
{
    return prst == QLatin1String("rect") || prst == QLatin1String("ellipse") ? prst : QString();
}

Fill readFill(const QDomElement &spPr, const QDomElement &, const DrawingContext &ctx)
{
    const QDomElement solid = firstChild(spPr, QStringLiteral("solidFill"));
    if (solid.isNull()) return Fill();
    const QColor c = readColor(solid, ctx.theme, ctx.colors);
    return c.isValid() ? Fill::solid(ColorRef::rgb(QColor(c.red(), c.green(), c.blue())), 1.0 - c.alphaF()) : Fill();
}

Stroke readLine(const QDomElement &spPr, const QDomElement &, const DrawingContext &ctx)
{
    const QDomElement ln = firstChild(spPr, QStringLiteral("ln"));
    const QDomElement solid = firstChild(ln, QStringLiteral("solidFill"));
    if (solid.isNull()) return Stroke();
    const QColor c = readColor(solid, ctx.theme, ctx.colors);
    return c.isValid() ? Stroke::line(ColorRef::rgb(c), emuToPoints(parseInt(attr(ln, QStringLiteral("w")), 9525))) : Stroke();
}

Fill readBackground(const QDomElement &bg, const DrawingContext &ctx)
{
    return readFill(firstChild(bg, QStringLiteral("bgPr")), QDomElement(), ctx);
}

ItemPtr readShape(const QDomElement &e, const DrawingContext &ctx, const TextStyleStack &styles, const QVector<QDomElement> &inheritsFrom)
{
    const QString kind = e.localName();
    if (kind == QLatin1String("grpSp")) {
        auto g = std::make_shared<GroupItem>();
        g->children = readShapeTree(e, ctx, styles);
        if (g->children.empty()) return nullptr;
        g->syncRect();
        return g;
    }
    if (kind != QLatin1String("sp") && kind != QLatin1String("cxnSp")) return nullptr;
    const QDomElement spPr = firstChild(e, QStringLiteral("spPr"));
    const QDomElement body = firstChild(e, QStringLiteral("txBody"));
    const QString prst = attr(firstChild(spPr, QStringLiteral("prstGeom")), QStringLiteral("prst"));
    ItemPtr it;
    if (!body.isNull() && (prst.isEmpty() || prst == QLatin1String("rect"))) {
        auto t = std::make_shared<TextItem>();
        if (ctx.newStory) {
            t->storyId = ctx.newStory();
            if (QTextDocument *d = ctx.storyDoc ? ctx.storyDoc(t->storyId) : nullptr) readTextBody(body, d, ctx, styles);
        }
        it = t;
    } else {
        auto s = std::make_shared<ShapeItem>();
        const QString id = shapeForPreset(prst);
        s->shape = id.isEmpty() ? QStringLiteral("rect") : id;
        it = s;
    }
    readTransform(transformOf(e, inheritsFrom), *it);
    it->fill = readFill(spPr, QDomElement(), ctx);
    it->stroke = readLine(spPr, QDomElement(), ctx);
    const QDomElement nv = firstChild(firstChild(e, kind == QLatin1String("sp") ? QStringLiteral("nvSpPr") : QStringLiteral("nvCxnSpPr")), QStringLiteral("cNvPr"));
    it->name = attr(nv, QStringLiteral("name"));
    it->altText = attr(nv, QStringLiteral("descr"));
    return it;
}

ItemList readShapeTree(const QDomElement &tree, const DrawingContext &ctx, const TextStyleStack &styles)
{
    ItemList out;
    for (QDomElement c = tree.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (ItemPtr it = readShape(c, ctx, styles)) out.push_back(it);
    return out;
}

} // namespace jp::office
