// Shapes, pictures, connectors, and groups (drawingml.h): where each is and
// how it is turned, what outline it has and what is in it, and for a group,
// how what is in it is placed in the page.

#include "office/drawingml.h"

#include "core/shapetypes.h"

#include "office/drawingml_geometry.h"
#include "office/drawingml_style.h"
#include "office/drawingml_styleref.h"
#include "office/xml.h"
#include "render/shapes.h"

#include <QTextDocument>
#include <QtMath>
#include <algorithm>
#include <climits>
#include <optional>
#include <cmath>
#include <vector>

namespace jp::office {

namespace {

constexpr int kMaxGroupDepth = 64;
// The drawing a diagram (SmartArt) keeps beside its data: shapes like a slide's, in this namespace.
constexpr QLatin1String kDsp("http://schemas.microsoft.com/office/drawing/2008/diagram");
constexpr int kMaxShapes = 10000;
constexpr double kMaxPoint = 1e6;     // points: the farthest a position or a size is taken to be

double position(double v)
{
    return std::isfinite(v) ? std::clamp(v, -kMaxPoint, kMaxPoint) : 0.0;
}

double extent(double v)
{
    return std::isfinite(v) ? std::clamp(v, 0.0, kMaxPoint) : 0.0;
}

// A turn in degrees, from 0 up to a full one.
double turn(double degrees)
{
    if (!std::isfinite(degrees)) return 0;
    double d = std::fmod(degrees, 360.0);
    if (d < 0) d += 360;
    return d >= 360 ? 0 : d;
}

struct Frame {
    QRectF rect;
    double rotation = 0;
    bool flipH = false, flipV = false;
};

// What an a:xfrm says; nothing is a point at the origin.
Frame readFrame(const QDomElement &xfrm)
{
    Frame f;
    if (xfrm.isNull()) return f;
    const QDomElement off = childNamed(xfrm, "off"), ext = childNamed(xfrm, "ext");
    f.rect = QRectF(position(parseLength(attr(off, QStringLiteral("x")))), position(parseLength(attr(off, QStringLiteral("y")))),
                    extent(parseLength(attr(ext, QStringLiteral("cx")))), extent(parseLength(attr(ext, QStringLiteral("cy")))));
    f.rotation = turn(parseAngle(attr(xfrm, QStringLiteral("rot"))));
    f.flipH = parseBool(attr(xfrm, QStringLiteral("flipH")));
    f.flipV = parseBool(attr(xfrm, QStringLiteral("flipV")));
    return f;
}

// Where a group puts what is in it: from its child space to its frame in the
// page, turned and flipped about the frame's center.
struct GroupMap {
    QPointF origin, childOrigin, center;
    double sx = 1, sy = 1;
    double rotation = 0;
    bool flipH = false, flipV = false;
    QTransform turned;

    QPointF map(const QPointF &p) const
    {
        QPointF q(origin.x() + (p.x() - childOrigin.x()) * sx, origin.y() + (p.y() - childOrigin.y()) * sy);
        if (flipH) q.setX(2 * center.x() - q.x());
        if (flipV) q.setY(2 * center.y() - q.y());
        if (rotation != 0) q = turned.map(q);
        return QPointF(position(q.x()), position(q.y()));
    }
};

GroupMap readGroupMap(const QDomElement &xfrm)
{
    GroupMap m;
    if (xfrm.isNull()) return m;
    const Frame frame = readFrame(xfrm);
    const QDomElement chOff = childNamed(xfrm, "chOff"), chExt = childNamed(xfrm, "chExt");
    const double cw = extent(parseLength(attr(chExt, QStringLiteral("cx")))), ch = extent(parseLength(attr(chExt, QStringLiteral("cy"))));
    m.origin = frame.rect.topLeft();
    m.childOrigin = QPointF(position(parseLength(attr(chOff, QStringLiteral("x")))), position(parseLength(attr(chOff, QStringLiteral("y")))));
    m.sx = cw > 0 ? frame.rect.width() / cw : 1.0;
    m.sy = ch > 0 ? frame.rect.height() / ch : 1.0;
    m.center = frame.rect.center();
    m.rotation = frame.rotation;
    m.flipH = frame.flipH;
    m.flipV = frame.flipV;
    m.turned.translate(m.center.x(), m.center.y());
    m.turned.rotate(m.rotation);
    m.turned.translate(-m.center.x(), -m.center.y());
    return m;
}

// Puts an item (and what is in it) where the group does.
void placeInGroup(Item &it, const GroupMap &m)
{
    if (it.type() == ItemType::Group) {
        auto &g = static_cast<GroupItem &>(it);
        for (const ItemPtr &c : g.children) placeInGroup(*c, m);
        g.syncRect();
        return;
    }
    if (it.type() == ItemType::Line) {
        auto &l = static_cast<LineItem &>(it);
        l.p1 = m.map(l.p1);
        l.p2 = m.map(l.p2);
        l.syncRect();
        return;
    }
    const QSizeF before = it.rect.size();
    // A shape turned a quarter has its width along the page's other side.
    const double a = qDegreesToRadians(it.rotation);
    const QSizeF size(extent(before.width() * std::hypot(m.sx * std::cos(a), m.sy * std::sin(a))),
                      extent(before.height() * std::hypot(m.sx * std::sin(a), m.sy * std::cos(a))));
    const QPointF center = m.map(it.rect.center());
    double rotation = it.rotation;
    if (m.flipH) {
        it.flipH = !it.flipH;
        rotation = -rotation;
    }
    if (m.flipV) {
        it.flipV = !it.flipV;
        rotation = -rotation;
    }
    it.rotation = turn(rotation + m.rotation);
    it.rect = QRectF(position(center.x() - size.width() / 2), position(center.y() - size.height() / 2), size.width(), size.height());
    if (it.type() == ItemType::Shape) {
        static_cast<ShapeItem &>(it).resized(before);
    } else if (it.type() == ItemType::Picture && before.width() > 0 && before.height() > 0) {
        auto &p = static_cast<PictureItem &>(it);
        const double kx = size.width() / before.width(), ky = size.height() / before.height();
        p.imgRect = QRectF(p.imgRect.x() * kx, p.imgRect.y() * ky, p.imgRect.width() * kx, p.imgRect.height() * ky);
    }
}

bool hasText(const QDomElement &body)
{
    for (QDomElement p = body.firstChildElement(); !p.isNull(); p = p.nextSiblingElement()) {
        if (p.localName() != QLatin1String("p")) continue;
        for (QDomElement r = p.firstChildElement(); !r.isNull(); r = r.nextSiblingElement()) {
            if (r.localName() == QLatin1String("fld")) return true;
            if (r.localName() == QLatin1String("r") && !childNamed(r, "t").text().isEmpty()) return true;
        }
    }
    return false;
}

// What a text body's a:bodyPr says, from the shape's own and then those it inherits from.
struct BodyProperties {
    QMarginsF insets{7.2, 3.6, 7.2, 3.6};
    VAlign valign = VAlign::Top;
    TextItem::Autofit autofit = TextItem::NoAutofit;
    double fitScale = 1.0;
    double fitLineCut = 0;      // a:normAutofit's lnSpcReduction, in percent
    int columns = 1;
    double columnGap = 0;
    bool vertical = false;
    bool noWrap = false;        // wrap="none"
    bool centerBlock = false;   // anchorCtr="1"
};

BodyProperties readBodyProperties(const QVector<QDomElement> &bodyPrs)
{
    BodyProperties b;
    auto find = [&](const char *name, QString *value) {
        for (const QDomElement &e : bodyPrs)
            if (hasAttr(e, QString::fromLatin1(name))) {
                *value = attr(e, QString::fromLatin1(name));
                return true;
            }
        return false;
    };
    auto inset = [&](const char *name, double fallback) {
        QString v;
        return find(name, &v) ? std::clamp(parseLength(v, fallback), -1000.0, 10000.0) : fallback;
    };
    b.insets = QMarginsF(inset("lIns", 7.2), inset("tIns", 3.6), inset("rIns", 7.2), inset("bIns", 3.6));
    QString v;
    if (find("anchor", &v)) b.valign = v == QLatin1String("ctr") ? VAlign::Middle : v == QLatin1String("b") ? VAlign::Bottom : VAlign::Top;
    if (find("numCol", &v)) b.columns = int(std::clamp<qint64>(parseInt(v, 1), 1, 16));
    if (find("spcCol", &v)) b.columnGap = std::clamp(parseLength(v, 0), 0.0, 1000.0);
    if (find("vert", &v)) b.vertical = v != QLatin1String("horz");
    if (find("wrap", &v)) b.noWrap = v == QLatin1String("none");
    if (find("anchorCtr", &v)) b.centerBlock = v == QLatin1String("1") || v == QLatin1String("true");
    for (const QDomElement &e : bodyPrs) {
        const QDomElement norm = childNamed(e, "normAutofit"), grow = childNamed(e, "spAutoFit"), none = childNamed(e, "noAutofit");
        if (!norm.isNull()) {
            b.autofit = TextItem::ShrinkOnOverflow;
            b.fitScale = std::clamp(parsePercent(attr(norm, QStringLiteral("fontScale")), 1.0), 0.01, 1.0);
            b.fitLineCut = std::clamp(parsePercent(attr(norm, QStringLiteral("lnSpcReduction")), 0.0) * 100, 0.0, 99.0);
        } else if (!grow.isNull()) {
            b.autofit = TextItem::GrowBox;
        } else if (none.isNull()) {
            continue;
        }
        break;
    }
    return b;
}

// A shaded face of a shape: its fill, moved toward white or black.
Fill shaded(const Fill &fill, GeometryPath::Fill mode)
{
    if (mode == GeometryPath::Normal) return fill;
    if (mode == GeometryPath::None) return Fill();
    if (fill.type != Fill::Solid) return fill;
    const bool lighter = mode == GeometryPath::Lighten || mode == GeometryPath::LightenLess;
    const double amount = mode == GeometryPath::Lighten || mode == GeometryPath::Darken ? 0.4 : 0.2;
    const QColor c = fill.color.rgbValue(), to = lighter ? QColor(Qt::white) : QColor(Qt::black);
    Fill out = fill;
    out.color = ColorRef::rgb(QColor(qRound(c.red() + (to.red() - c.red()) * amount), qRound(c.green() + (to.green() - c.green()) * amount),
                                     qRound(c.blue() + (to.blue() - c.blue()) * amount)));
    return out;
}

// Which of an object's connection points a connector's index (in the order
// the standard lists them for the shape) is, in the object's own order (top,
// left, bottom, right); -1 for a shape whose points aren't known here.
int connectionSiteOf(const Item &target, qint64 index)
{
    static const QStringList fourPoints = {QStringLiteral("rect"), QStringLiteral("roundRect"), QStringLiteral("snip1"), QStringLiteral("snip2same"),
        QStringLiteral("snip2diag"), QStringLiteral("snipRound"), QStringLiteral("round1"), QStringLiteral("round2same"), QStringLiteral("round2diag"),
        QStringLiteral("diamond"), QStringLiteral("fcProcess"), QStringLiteral("fcAltProcess"), QStringLiteral("fcDecision"), QStringLiteral("fcPredefined"),
        QStringLiteral("fcInternalStorage")};
    static const QStringList eightPoints = {QStringLiteral("ellipse"), QStringLiteral("fcConnector"), QStringLiteral("fcSummingJunction"), QStringLiteral("fcOr")};
    bool four = target.type() == ItemType::Text || target.type() == ItemType::Picture;
    bool eight = false;
    if (target.type() == ItemType::Shape) {
        const auto &s = static_cast<const ShapeItem &>(target);
        four = fourPoints.contains(s.shape) && s.customPath.isEmpty();
        eight = eightPoints.contains(s.shape) && s.customPath.isEmpty();
    }
    if (four) return index >= 0 && index < 4 ? int(index) : -1;
    if (eight) return index >= 0 && index < 8 && index % 2 == 0 ? int(index / 2) : -1;
    return -1;
}

// The first child called `name` of the first element of `list` that has one.
QDomElement firstWith(const QVector<QDomElement> &list, const char *name)
{
    for (const QDomElement &e : list) {
        const QDomElement c = childNamed(e, name);
        if (!c.isNull()) return c;
    }
    return QDomElement();
}

// The a:spPr of a shape and of those it inherits from, in that order.
QVector<QDomElement> propertiesOf(const QDomElement &e, const QVector<QDomElement> &inheritsFrom)
{
    QVector<QDomElement> out;
    for (const QDomElement &from : QVector<QDomElement>{e} + inheritsFrom) {
        const QDomElement p = childNamed(from, "spPr");
        if (!p.isNull()) out.append(p);
    }
    return out;
}

// Where the whole picture of a p:blipFill (or a shape's a:blipFill) is in a frame of `size`: it is cut by a:srcRect (a negative cut is room around it),
// and what is left is stretched over the frame, or over part of it (a:fillRect; a negative inset spreads it past the frame).
QRectF pictureRect(const QDomElement &blipFill, const QSizeF &size)
{
    QRectF target(0, 0, size.width(), size.height());
    const QDomElement fillRect = childNamed(childNamed(blipFill, "stretch"), "fillRect");
    if (!fillRect.isNull()) {
        const double l = parsePercent(attr(fillRect, QStringLiteral("l")), 0), t = parsePercent(attr(fillRect, QStringLiteral("t")), 0);
        const double r = parsePercent(attr(fillRect, QStringLiteral("r")), 0), b = parsePercent(attr(fillRect, QStringLiteral("b")), 0);
        if (l + r < 1 && t + b < 1) target = QRectF(size.width() * l, size.height() * t, size.width() * (1 - l - r), size.height() * (1 - t - b));
    }
    const QDomElement src = childNamed(blipFill, "srcRect");
    double cl = parsePercent(attr(src, QStringLiteral("l")), 0), ct = parsePercent(attr(src, QStringLiteral("t")), 0);
    double cr = parsePercent(attr(src, QStringLiteral("r")), 0), cb = parsePercent(attr(src, QStringLiteral("b")), 0);
    if (1 - cl - cr <= 0.001 || 1 - ct - cb <= 0.001) cl = ct = cr = cb = 0;
    const double w = target.width() / (1 - cl - cr), h = target.height() / (1 - ct - cb);
    return QRectF(position(target.x() - cl * w), position(target.y() - ct * h), extent(w), extent(h));
}

class Reader {
public:
    Reader(const DrawingContext &ctx, const TextStyleStack &styles) : m_ctx(ctx), m_styles(styles) {}

    ItemPtr shape(const QDomElement &e, const QVector<QDomElement> &inheritsFrom);
    ItemList tree(const QDomElement &parent);
    // Joins the connectors to what they were drawn between, once all of it is read.
    void join();

private:
    struct Pending {
        std::shared_ptr<LineItem> line;
        qint64 startId = -1, startIndex = -1, endId = -1, endIndex = -1;
    };

    StyleContext styleContext(const QSizeF &frame = QSizeF())
    {
        StyleContext sc;
        sc.ctx = &m_ctx;
        sc.groupFill = m_groupFill;
        sc.frame = frame;
        sc.images = m_ctx.images ? m_ctx.images : &m_images;
        return sc;
    }

    void appendChildren(const QDomElement &parent, ItemList *out, int nesting);
    ItemPtr group(const QDomElement &e);
    ItemPtr pic(const QDomElement &e, const QVector<QDomElement> &inheritsFrom);
    void media(const QDomElement &nvPr, PictureItem &p);
    // A p:sp or p:cxnSp; `nv` is its p:nvSpPr or p:nvCxnSpPr.
    ItemPtr drawn(const QDomElement &e, const QDomElement &nv, const QVector<QDomElement> &inheritsFrom, bool connector);
    // A shape of a diagram's drawing: as a p:sp, but its text goes in the rectangle of dsp:txXfrm.
    ItemPtr diagramShape(const QDomElement &e);
    ItemPtr diagramShapeText(const QDomElement &e);
    ItemPtr withCutPicture(const ItemPtr &item, const QDomElement &spPr);
    // The text of `body` in `frame`, as a box of its own; its look is `decoration`'s, the text's `style`'s.
    ItemPtr makeTextBox(const QDomElement &cNvPr, const Frame &frame, const QVector<QDomElement> &properties, const QDomElement &decoration, const QDomElement &style,
                    const BodyProperties &props, const QDomElement &body);
    ItemPtr line(const Frame &frame, const QString &prst, const QHash<QString, double> &adjust, const QDomElement &cNvPr, const QDomElement &nv,
                 const QVector<QDomElement> &properties, const QDomElement &style);
    void decorate(Item &it, const QDomElement &cNvPr, const Frame &frame, const QVector<QDomElement> &properties, const QDomElement &style);
    void fillText(const QDomElement &body, const QString &storyId, const QDomElement &style);
    QString hyperlinkOf(const QDomElement &cNvPr) const;
    void remember(const QDomElement &cNvPr, const ItemPtr &it);

    const DrawingContext &m_ctx;
    const TextStyleStack &m_styles;
    int m_count = 0;
    int m_depth = 0;
    bool m_noText = false;   // reading a shape whose text is read apart (diagramShape)
    const Fill *m_groupFill = nullptr;
    QHash<QString, QString> m_images;
    QHash<qint64, ItemPtr> m_byId;
    QVector<Pending> m_pending;
};

ItemPtr Reader::shape(const QDomElement &e, const QVector<QDomElement> &inheritsFrom)
{
    if (e.isNull()) return nullptr;
    // A slide's elements, and a document's shapes (wsp) and groups (wgp), which are the same.
    const QString kind = e.localName();
    const bool isShape = kind == QLatin1String("sp") || kind == QLatin1String("wsp"), isGroup = kind == QLatin1String("grpSp") || kind == QLatin1String("wgp");
    const bool isConnector = kind == QLatin1String("cxnSp"), isPicture = kind == QLatin1String("pic");
    if (!(isShape || isGroup || isConnector || isPicture)) return nullptr;
    if (m_count >= kMaxShapes) {
        if (m_ctx.budget) m_ctx.budget->ranOut[DrawingBudget::Objects] = true;
        return nullptr;
    }
    // Every object of the import is one of its budget's, those of a group and the group itself.
    if (m_ctx.budget && !m_ctx.budget->take(DrawingBudget::Objects)) return nullptr;
    ++m_count;
    if (isGroup) return group(e);
    if (isPicture) return pic(e, inheritsFrom);
    if (isConnector) return drawn(e, childNamed(e, "nvCxnSpPr"), inheritsFrom, true);
    const QDomElement nv = childNamed(e, "nvSpPr");
    ItemPtr it = e.namespaceURI() == kDsp ? diagramShape(e) : drawn(e, nv.isNull() ? e : nv, inheritsFrom, false);
    // A shape filled with the page's background (a design's panel the color of the slide behind it).
    if (it && parseBool(attr(e, QStringLiteral("useBgFill")))) it->fill = m_ctx.background;
    return it;
}

ItemList Reader::tree(const QDomElement &parent)
{
    ItemList out;
    appendChildren(parent, &out, 0);
    return out;
}

void Reader::appendChildren(const QDomElement &parent, ItemList *out, int nesting)
{
    for (QDomElement c = parent.firstChildElement(); !c.isNull() && m_count < kMaxShapes; c = c.nextSiblingElement()) {
        // What a newer program wrote with a stand-in for older ones: the stand-in.
        if (c.localName() == QLatin1String("AlternateContent")) {
            if (nesting < 4) appendChildren(childNamed(c, "Fallback"), out, nesting + 1);
            continue;
        }
        if (ItemPtr it = shape(c, {})) out->push_back(it);
    }
}

void Reader::join()
{
    for (const Pending &p : m_pending) {
        auto glue = [&](qint64 id, qint64 index, LineItem::Glue *end) {
            const auto it = m_byId.constFind(id);
            if (it == m_byId.constEnd()) return;
            const int site = connectionSiteOf(**it, index);
            if (site >= 0) *end = LineItem::Glue{(*it)->id, site};
        };
        glue(p.startId, p.startIndex, &p.line->start);
        glue(p.endId, p.endIndex, &p.line->end);
    }
    m_pending.clear();
}

QString Reader::hyperlinkOf(const QDomElement &cNvPr) const
{
    const QDomElement click = childNamed(cNvPr, "hlinkClick");
    const QString id = attrNs(click, ns::R, QStringLiteral("id"));
    if (id.isEmpty() || !m_ctx.package) return QString();
    const Relationship rel = m_ctx.package->relationship(m_ctx.part, id);
    return !rel.id.isEmpty() && rel.external ? rel.target : QString();
}

void Reader::remember(const QDomElement &cNvPr, const ItemPtr &it)
{
    bool ok = false;
    const qint64 id = parseInt(attr(cNvPr, QStringLiteral("id")), -1, &ok);
    if (ok && it) m_byId.insert(id, it);
    if (ok && it && id > 0 && id <= INT_MAX) it->sourceId = int(id);
}

// Whether the object says its proportions are kept (a:spLocks, a:picLocks, or a:cxnSpLocks, beside the p:cNvPr in the object's non-visual
// properties); nothing when it doesn't say.
static std::optional<bool> aspectLockOf(const QDomElement &cNvPr)
{
    const QDomElement nv = cNvPr.parentNode().toElement();
    for (const char *props : {"cNvSpPr", "cNvPicPr", "cNvCxnSpPr"})
        for (const char *locks : {"spLocks", "picLocks", "cxnSpLocks"}) {
            const QDomElement l = childNamed(childNamed(nv, props), locks);
            if (!l.isNull() && hasAttr(l, QStringLiteral("noChangeAspect"))) return parseBool(attr(l, QStringLiteral("noChangeAspect")));
        }
    return std::nullopt;
}

void Reader::decorate(Item &it, const QDomElement &cNvPr, const Frame &frame, const QVector<QDomElement> &properties, const QDomElement &style)
{
    it.rect = frame.rect;
    it.rotation = frame.rotation;
    it.flipH = frame.flipH;
    it.flipV = frame.flipV;
    it.name = attr(cNvPr, QStringLiteral("name"));
    it.altText = attr(cNvPr, QStringLiteral("descr"));
    it.altTitle = attr(cNvPr, QStringLiteral("title"));
    it.decorative = decorativeOf(cNvPr);
    if (const std::optional<bool> lock = aspectLockOf(cNvPr)) it.lockAspect = *lock;
    it.hyperlink = hyperlinkOf(cNvPr);
    it.wrap.mode = Wrap::None;
    const StyleContext sc = styleContext(frame.rect.size());
    it.fill = resolveFill(properties, style, sc);
    it.stroke = resolveLine(properties, style, sc);
    it.fx = resolveEffects(properties, style, sc);
    it.styleRef = readStyleRef(style);
}

void Reader::fillText(const QDomElement &body, const QString &storyId, const QDomElement &style)
{
    if (body.isNull() || storyId.isEmpty() || !m_ctx.storyDoc) return;
    QTextDocument *doc = m_ctx.storyDoc(storyId);
    if (!doc) return;
    QDomDocument keep;
    readTextBody(body, doc, m_ctx, withFontReference(m_styles, style, childNamed(body, "lstStyle"), &keep));
}

ItemPtr Reader::group(const QDomElement &e)
{
    const QDomElement nv = childNamed(e, "nvGrpSpPr");
    const QDomElement cNvPr = childNamed(nv.isNull() ? e : nv, "cNvPr");
    if (parseBool(attr(cNvPr, QStringLiteral("hidden"))) || m_depth >= kMaxGroupDepth) return nullptr;
    const QDomElement props = childNamed(e, "grpSpPr");
    // What its children's a:grpFill is: its own fill, if it has one, else that of the group around it.
    const Fill *outerFill = m_groupFill;
    Fill ownFill;
    if (hasOwnFill(props)) {
        ownFill = resolveFill({props}, QDomElement(), styleContext());
        m_groupFill = &ownFill;
    }
    ++m_depth;
    ItemList kids;
    appendChildren(e, &kids, 0);
    --m_depth;
    m_groupFill = outerFill;
    if (kids.empty()) return nullptr;
    const GroupMap map = readGroupMap(childNamed(props, "xfrm"));
    auto g = std::make_shared<GroupItem>();
    for (const ItemPtr &k : kids) placeInGroup(*k, map);
    g->children = kids;
    g->name = attr(cNvPr, QStringLiteral("name"));
    g->altText = attr(cNvPr, QStringLiteral("descr"));
    g->altTitle = attr(cNvPr, QStringLiteral("title"));
    g->decorative = decorativeOf(cNvPr);
    g->hyperlink = hyperlinkOf(cNvPr);
    g->wrap.mode = Wrap::None;
    g->syncRect();
    remember(cNvPr, g);
    return g;
}

ItemPtr Reader::drawn(const QDomElement &e, const QDomElement &nv, const QVector<QDomElement> &inheritsFrom, bool connector)
{
    const QDomElement cNvPr = childNamed(nv, "cNvPr");
    if (parseBool(attr(cNvPr, QStringLiteral("hidden")))) return nullptr;
    const QVector<QDomElement> properties = propertiesOf(e, inheritsFrom);
    const QDomElement body = m_noText ? QDomElement() : childNamed(e, "txBody");
    const QDomElement ph = childNamed(childNamed(nv, "nvPr"), "ph");
    const bool textBox = !connector && !m_noText && parseBool(attr(childNamed(nv, "cNvSpPr"), QStringLiteral("txBox")));
    const QDomElement style = firstWith(QVector<QDomElement>{e} + inheritsFrom, "style");
    // The geometry is the first said, among the shape's own and those it inherits from.
    QDomElement geom;
    for (const QDomElement &p : properties) {
        geom = childNamed(p, "custGeom");
        if (geom.isNull()) geom = childNamed(p, "prstGeom");
        if (!geom.isNull()) break;
    }
    const bool custom = geom.localName() == QLatin1String("custGeom");
    // A connector that says nothing of its shape is straight.
    const QString prst = custom ? QString() : connector && geom.isNull() ? QStringLiteral("straightConnector1") : attr(geom, QStringLiteral("prst"));
    const Frame frame = readFrame(firstWith(properties, "xfrm"));

    // A picture placeholder without its picture.
    if (!connector && attr(ph, QStringLiteral("type")) == QLatin1String("pic")) {
        auto p = std::make_shared<PictureItem>();
        decorate(*p, cNvPr, frame, properties, style);
        p->imgRect = QRectF(0, 0, frame.rect.width(), frame.rect.height());
        remember(cNvPr, p);
        return p;
    }
    const QHash<QString, double> adjust = readAdjustments(childNamed(geom, "avLst"));
    const bool isLine = !custom && (prst == QLatin1String("line") || prst == QLatin1String("lineInv") || prst == QLatin1String("straightConnector1") ||
                                    prst == QLatin1String("bentConnector2") || prst == QLatin1String("bentConnector3") ||
                                    prst == QLatin1String("curvedConnector2") || prst == QLatin1String("curvedConnector3"));
    if (isLine) {
        ItemPtr l = line(frame, prst, adjust, cNvPr, nv, properties, style);
        remember(cNvPr, l);
        return l;
    }

    const bool plainRect = !custom && (prst.isEmpty() || prst == QLatin1String("rect"));
    QVector<QDomElement> bodyPrs;
    bodyPrs.append(body.isNull() ? childNamed(e, "bodyPr") : childNamed(body, "bodyPr"));
    for (const QDomElement &from : inheritsFrom) bodyPrs.append(childNamed(childNamed(from, "txBody"), "bodyPr"));
    const BodyProperties props = readBodyProperties(bodyPrs);

    // The older text art, as the presentation program keeps it: text warped
    // into its box and stretched to fill it (fromWordArt), the art's look that
    // of its first letters.
    const QDomElement warp = childNamed(bodyPrs.value(0), "prstTxWarp");
    if (!connector && ph.isNull() && parseBool(attr(bodyPrs.value(0), QStringLiteral("fromWordArt"))) && !warp.isNull()
        && attr(warp, QStringLiteral("prst")) != QLatin1String("textNoShape") && hasText(body)) {
        static const QStringList warps = {
            "textPlain", "textStop", "textTriangle", "textTriangleInverted", "textChevron", "textChevronInverted", "textRingInside",
            "textRingOutside", "textArchUp", "textArchDown", "textCircle", "textButton", "textArchUpPour", "textArchDownPour",
            "textCirclePour", "textButtonPour", "textCurveUp", "textCurveDown", "textCascadeUp", "textCascadeDown", "textWave1",
            "textWave2", "textDoubleWave1", "textWave4", "textInflate", "textDeflate", "textInflateBottom", "textDeflateBottom",
            "textInflateTop", "textDeflateTop", "textDeflateInflate", "textDeflateInflateDeflate", "textFadeRight", "textFadeLeft",
            "textFadeUp", "textFadeDown", "textSlantUp", "textSlantDown", "textCanUp", "textCanDown"};
        auto a = std::make_shared<TextArtItem>();
        decorate(*a, cNvPr, frame, properties, style);
        const int index = int(warps.indexOf(attr(warp, QStringLiteral("prst"))));
        const QString transform = index >= 0 ? textArtForShapeType(136 + index) : QString();
        a->transform_ = transform.isEmpty() ? QStringLiteral("plain") : transform;
        a->vertical = attr(bodyPrs.value(0), QStringLiteral("vert")).startsWith(QLatin1String("wordArtVert"));
        QStringList lines;
        QDomElement firstRun, firstPara;
        for (QDomElement para = childNamed(body, "p"); !para.isNull(); para = para.nextSiblingElement()) {
            if (para.localName() != QLatin1String("p")) continue;
            if (firstPara.isNull()) firstPara = para;
            QString text;
            for (QDomElement r = para.firstChildElement(); !r.isNull(); r = r.nextSiblingElement()) {
                if (r.localName() == QLatin1String("br")) text += QLatin1Char('\n');
                if (r.localName() != QLatin1String("r") && r.localName() != QLatin1String("fld")) continue;
                if (firstRun.isNull()) firstRun = r;
                text += childNamed(r, "t").text();
            }
            lines << text;
        }
        a->text = lines.join(QLatin1Char('\n'));
        const QString algn = attr(childNamed(firstPara, "pPr"), QStringLiteral("algn"));
        a->align = algn == QLatin1String("l") ? 0 : algn == QLatin1String("r") ? 2 : algn == QLatin1String("just") ? 3 : algn == QLatin1String("dist") ? 4 : 1;
        const QDomElement rPr = childNamed(firstRun, "rPr");
        const double size = parseInt(attr(rPr, QStringLiteral("sz")), 3600) / 100.0;
        a->size = std::clamp(size, 1.0, 4000.0);
        a->bold = parseBool(attr(rPr, QStringLiteral("b")));
        a->italic = parseBool(attr(rPr, QStringLiteral("i")));
        if (const QString spc = attr(rPr, QStringLiteral("spc")); !spc.isEmpty()) a->spacing = std::clamp(1.0 + parseInt(spc, 0) / 100.0 / a->size, 0.5, 2.0);
        const QString typeface = attr(childNamed(rPr, "latin"), QStringLiteral("typeface"));
        const QString font = typeface.startsWith(QLatin1String("+mj")) ? m_ctx.theme.majorLatin : typeface.startsWith(QLatin1String("+mn")) ? m_ctx.theme.minorLatin : typeface;
        if (!font.isEmpty()) a->font = font;
        // The letters' fill and outline (a:rPr holds them as a shape's properties do).
        a->fill = readFill(rPr, QDomElement(), m_ctx);
        a->stroke = readLine(rPr, QDomElement(), m_ctx);
        if (!childNamed(rPr, "effectLst").isNull()) a->fx = readEffects(rPr, QDomElement(), m_ctx);
        remember(cNvPr, a);
        return a;
    }

    if (!connector && plainRect && (textBox || !ph.isNull() || hasText(body))) {
        // A text box is its text: past the import's stories it is left out.
        if (m_ctx.newStory && m_ctx.budget && !m_ctx.budget->take(DrawingBudget::Stories)) return nullptr;
        ItemPtr t = makeTextBox(cNvPr, frame, properties, style, style, props, body);
        remember(cNvPr, t);
        return t;
    }

    // Anything else is a shape: one the shape library draws, or the paths of its geometry.
    const QSizeF size = frame.rect.size();
    ShapeItem base;
    decorate(base, cNvPr, frame, properties, style);
    base.insets = props.insets;
    base.valign = props.valign;
    base.noWrap = props.noWrap;
    base.centerBlock = props.centerBlock;
    base.columns = props.columns;
    base.columnGap = props.columnGap;
    // Text the program shrank to fit keeps the scale it was given (a:normAutofit's fontScale and lnSpcReduction).
    base.shrinks = props.autofit == TextItem::ShrinkOnOverflow;
    base.fitScale = props.fitScale;
    base.fitLineCut = props.fitLineCut;
    base.shape = QStringLiteral("rect");
    const QString libraryShape = shapeForPreset(prst);
    Geometry geometry;
    if (custom) geometry = readCustomGeometry(geom, size);
    else if ((prefersPresetGeometry(prst) || libraryShape.isEmpty()) && hasPresetGeometry(prst)) geometry = presetGeometry(prst, adjust, size);
    // The paths, each with how it is drawn: those drawn alike are one object.
    std::vector<QPainterPath> faces;
    std::vector<GeometryPath::Fill> faceFill;
    std::vector<bool> faceStroke;
    if (!geometry.isEmpty()) {
        // (Not an open shape's name: the library draws no fill in one, and an arc has a filled sector.)
        const ShapeDef *def = shapeDef(libraryShape);
        if (def && !def->open) base.shape = libraryShape;
        if (!geometry.textRect.isNull()) {
            const QRectF tr = geometry.textRect;
            base.insets = QMarginsF(std::max(0.0, base.insets.left() + tr.left()), std::max(0.0, base.insets.top() + tr.top()),
                                    std::max(0.0, base.insets.right() + size.width() - tr.right()), std::max(0.0, base.insets.bottom() + size.height() - tr.bottom()));
        }
        // Paths that show: filled where there is a fill, stroked where there is an outline.
        const bool filled = !base.fill.isNone(), stroked = !base.stroke.isNone();
        const auto fillOf = [&](const GeometryPath &gp) { return filled ? gp.fill : GeometryPath::None; };
        const auto strokeOf = [&](const GeometryPath &gp) { return stroked && gp.stroke; };
        QVector<GeometryPath> shown;
        for (const GeometryPath &gp : geometry.paths)
            if (fillOf(gp) != GeometryPath::None || strokeOf(gp)) shown.append(gp);
        if (shown.isEmpty()) shown = geometry.paths;
        for (const GeometryPath &gp : shown) {
            size_t at = 0;
            while (at < faces.size() && !(faceFill[at] == fillOf(gp) && faceStroke[at] == strokeOf(gp))) ++at;
            if (at == faces.size()) {
                faces.emplace_back();
                faceFill.push_back(fillOf(gp));
                faceStroke.push_back(strokeOf(gp));
            }
            faces[at].addPath(gp.path);
        }
    } else if (!libraryShape.isEmpty()) {
        base.shape = libraryShape;
        base.adj = libraryAdjustments(prst, adjust, size);
    }

    ItemList parts;
    if (faces.empty()) {
        parts.push_back(std::make_shared<ShapeItem>(base));
    } else {
        for (size_t i = 0; i < faces.size(); ++i) {
            auto s = std::make_shared<ShapeItem>(base);
            s->id = newId();
            s->customPath = faces[i];
            s->fill = shaded(base.fill, faceFill[i]);
            if (!faceStroke[i]) s->stroke = Stroke();
            if (i > 0) s->fx = Effects();
            parts.push_back(s);
        }
    }
    // A shape past the import's stories stays, without its text.
    if (!body.isNull() && (hasText(body) || !ph.isNull() || textBox) && m_ctx.newStory && (!m_ctx.budget || m_ctx.budget->take(DrawingBudget::Stories))) {
        auto *first = static_cast<ShapeItem *>(parts[0].get());
        first->storyId = m_ctx.newStory();
        fillText(body, first->storyId, style);
    }
    ItemPtr result = parts[0];
    if (parts.size() > 1) {
        auto g = std::make_shared<GroupItem>();
        g->children = parts;
        g->name = base.name;
        g->altText = base.altText;
        g->altTitle = base.altTitle;
        g->decorative = base.decorative;
        g->hyperlink = base.hyperlink;
        g->wrap.mode = Wrap::None;
        g->syncRect();
        result = g;
    }
    remember(cNvPr, result);
    return result;
}

ItemPtr Reader::makeTextBox(const QDomElement &cNvPr, const Frame &frame, const QVector<QDomElement> &properties, const QDomElement &decoration, const QDomElement &style,
                        const BodyProperties &props, const QDomElement &body)
{
    auto t = std::make_shared<TextItem>();
    decorate(*t, cNvPr, frame, properties, decoration);
    t->hyphenate = false;
    t->insets = props.insets;
    t->valign = props.valign;
    t->autofit = props.autofit;
    t->fitScale = props.fitScale;
    t->fitLineCut = props.fitLineCut;
    // Text the program shrank keeps the size it was given (a:normAutofit's fontScale) until it is edited.
    t->fitAsStored = t->autofit == TextItem::ShrinkOnOverflow;
    t->columns = props.columns;
    t->columnGap = props.columnGap;
    t->vertical = props.vertical;
    t->noWrap = props.noWrap;
    t->centerBlock = props.centerBlock;
    if (m_ctx.newStory) {
        t->storyId = m_ctx.newStory();
        fillText(body, t->storyId, style);
    }
    return t;
}

// A shape of a diagram's drawing (dsp:sp) is a p:sp, with the rectangle its text is laid out in (dsp:txXfrm), which is in the shape's own space:
// the shape's turn carries it round the shape's center, and its own turn is added to the shape's. A text rectangle that is the shape's own (a
// plain rectangle) leaves the text in the shape; any other is a text box of its own over the shape, which keeps its fill and line to itself.
ItemPtr Reader::diagramShapeText(const QDomElement &e)
{
    const QDomElement nv = childNamed(e, "nvSpPr"), body = childNamed(e, "txBody"), txXfrm = childNamed(e, "txXfrm");
    const QDomElement from = nv.isNull() ? e : nv;
    if (txXfrm.isNull() || !hasText(body)) return drawn(e, from, {}, false);
    const QVector<QDomElement> properties = propertiesOf(e, {});
    const Frame shapeFrame = readFrame(firstWith(properties, "xfrm")), textFrame = readFrame(txXfrm);
    QDomElement geom = firstWith(properties, "custGeom");
    if (geom.isNull()) geom = firstWith(properties, "prstGeom");
    const QString prst = geom.localName() == QLatin1String("custGeom") ? QStringLiteral("custom") : attr(geom, QStringLiteral("prst"));
    const bool ownRect = (prst.isEmpty() || prst == QLatin1String("rect")) && textFrame.rect == shapeFrame.rect && textFrame.rotation == 0;
    if (ownRect) return drawn(e, from, {}, false);

    m_noText = true;
    ItemPtr shape = drawn(e, from, {}, false);
    m_noText = false;
    if (!shape) return nullptr;
    // A text box is one more object, and a story: past the import's, the shape stays without its text.
    if (m_count >= kMaxShapes) {
        if (m_ctx.budget) m_ctx.budget->ranOut[DrawingBudget::Objects] = true;
        return shape;
    }
    if (m_ctx.budget && (!m_ctx.budget->take(DrawingBudget::Objects) || (m_ctx.newStory && !m_ctx.budget->take(DrawingBudget::Stories)))) return shape;
    ++m_count;

    QPointF center = textFrame.rect.center();
    const QPointF shapeCenter = shapeFrame.rect.center();
    if (shapeFrame.flipH) center.setX(2 * shapeCenter.x() - center.x());
    if (shapeFrame.flipV) center.setY(2 * shapeCenter.y() - center.y());
    if (shapeFrame.rotation != 0) {
        QTransform turned;
        turned.translate(shapeCenter.x(), shapeCenter.y());
        turned.rotate(shapeFrame.rotation);
        turned.translate(-shapeCenter.x(), -shapeCenter.y());
        center = turned.map(center);
    }
    Frame place;
    place.rect = QRectF(position(center.x() - textFrame.rect.width() / 2), position(center.y() - textFrame.rect.height() / 2), textFrame.rect.width(), textFrame.rect.height());
    place.rotation = turn(shapeFrame.rotation + textFrame.rotation);
    const QDomElement style = firstWith(QVector<QDomElement>{e}, "style");
    const BodyProperties props = readBodyProperties({childNamed(body, "bodyPr")});
    ItemPtr text = makeTextBox(childNamed(nv, "cNvPr"), place, {}, QDomElement(), style, props, body);
    auto g = std::make_shared<GroupItem>();
    g->children = {shape, text};
    g->name = shape->name;
    g->altText = shape->altText;
    g->altTitle = shape->altTitle;
    g->decorative = shape->decorative;
    g->hyperlink = shape->hyperlink;
    g->wrap.mode = Wrap::None;
    g->syncRect();
    return g;
}

// A shape of a diagram's drawing, with a picture fill that is cut or spread past the shape (a:srcRect, a:fillRect: a picture cut to fill the shape
// is spread past it) is the picture, cut to the shape's outline, with the shape (its line and its text) over it: a fill has no such cut here.
ItemPtr Reader::diagramShape(const QDomElement &e)
{
    ItemPtr item = diagramShapeText(e);
    const QDomElement spPr = childNamed(e, "spPr");
    return item ? withCutPicture(item, spPr) : item;
}

ItemPtr Reader::withCutPicture(const ItemPtr &item, const QDomElement &spPr)
{
    const QDomElement blipFill = childNamed(spPr, "blipFill");
    if (blipFill.isNull() || !childNamed(spPr, "custGeom").isNull()) return item;
    auto moved = [](const QDomElement &e) {
        if (e.isNull()) return false;
        const QDomNamedNodeMap attrs = e.attributes();
        for (int i = 0; i < attrs.length(); ++i)
            if (parsePercent(attrs.item(i).nodeValue(), 0) != 0) return true;
        return false;
    };
    if (!moved(childNamed(blipFill, "srcRect")) && !moved(childNamed(childNamed(blipFill, "stretch"), "fillRect"))) return item;
    // The shape the picture is on: the item itself, or the first of a shape and its text box.
    const bool paired = item->type() == ItemType::Group;
    if (paired && (static_cast<GroupItem &>(*item).children.size() != 2 || static_cast<GroupItem &>(*item).children[1]->type() != ItemType::Text)) return item;
    ItemPtr shape = paired ? static_cast<GroupItem &>(*item).children.front() : item;
    if (shape->type() != ItemType::Shape && shape->type() != ItemType::Text) return item;
    if (shape->fill.type != Fill::Picture || shape->fill.imageId.isEmpty() || shape->fill.tile) return item;
    const QString prst = attr(childNamed(spPr, "prstGeom"), QStringLiteral("prst"));
    const QString mask = prst.isEmpty() ? QStringLiteral("rect") : shapeForPreset(prst);
    if (mask.isEmpty() || (m_ctx.budget && !m_ctx.budget->take(DrawingBudget::Objects))) return item;
    auto pic = std::make_shared<PictureItem>();
    pic->name = shape->name;
    pic->altText = shape->altText;
    pic->altTitle = shape->altTitle;
    pic->decorative = shape->decorative;
    // A picture keeps its proportions unless the file says the shape it was cut from doesn't (a shape that says nothing about it is not a lock off).
    if (const std::optional<bool> lock = aspectLockOf(childNamed(childNamed(spPr.parentNode().toElement(), "nvSpPr"), "cNvPr"))) pic->lockAspect = *lock;
    pic->rect = shape->rect;
    pic->rotation = shape->rotation;
    pic->flipH = shape->flipH;
    pic->flipV = shape->flipV;
    pic->wrap.mode = Wrap::None;
    pic->imageId = shape->fill.imageId;
    pic->transparency = shape->fill.transparency;
    pic->blipEffects = shape->fill.blipEffects;
    pic->maskShape = mask;
    pic->imgRect = pictureRect(blipFill, shape->rect.size());
    shape->fill = Fill();
    if (paired) {
        auto &g = static_cast<GroupItem &>(*item);
        g.children.insert(g.children.begin(), pic);
        g.syncRect();
        return item;
    }
    auto g = std::make_shared<GroupItem>();
    g->children = {pic, item};
    g->name = item->name;
    g->altText = item->altText;
    g->altTitle = item->altTitle;
    g->decorative = item->decorative;
    g->hyperlink = item->hyperlink;
    g->wrap.mode = Wrap::None;
    g->syncRect();
    return g;
}

ItemPtr Reader::line(const Frame &frame, const QString &prst, const QHash<QString, double> &adjust, const QDomElement &cNvPr, const QDomElement &nv,
                     const QVector<QDomElement> &properties, const QDomElement &style)
{
    auto l = std::make_shared<LineItem>();
    decorate(*l, cNvPr, frame, properties, style);
    l->fill = Fill();
    // The ends are the frame's corners as it is turned and flipped.
    ShapeItem placed;
    placed.rect = frame.rect;
    placed.rotation = frame.rotation;
    placed.flipH = frame.flipH;
    placed.flipV = frame.flipV;
    const QTransform t = placed.transform();
    const bool inverted = prst == QLatin1String("lineInv");
    const QPointF a = t.map(QPointF(0, inverted ? frame.rect.height() : 0)), b = t.map(QPointF(frame.rect.width(), inverted ? 0 : frame.rect.height()));
    l->p1 = QPointF(position(a.x()), position(a.y()));
    l->p2 = QPointF(position(b.x()), position(b.y()));
    const bool elbow = prst.startsWith(QLatin1String("bent")), curved = prst.startsWith(QLatin1String("curved"));
    if (elbow || curved) {
        l->route = elbow ? LineItem::Elbow : LineItem::Curved;
        const auto bend = adjust.constFind(QStringLiteral("adj1"));
        l->bend = std::clamp(bend == adjust.constEnd() ? 0.5 : *bend / 100000, -10.0, 11.0);
        // The ends leave level, but for the last of a corner, which is upright; a quarter turn exchanges them.
        const bool quarter = int(std::lround(frame.rotation / 90.0)) % 2 != 0;
        l->startVertical = quarter;
        l->endVertical = prst.endsWith(QLatin1Char('2')) ? !quarter : quarter;
    }
    l->flipH = l->flipV = false;
    l->syncRect();
    // What it is joined to, which is known once every shape is read.
    const QDomElement props = childNamed(nv, "cNvCxnSpPr");
    const QDomElement st = childNamed(props, "stCxn"), en = childNamed(props, "endCxn");
    if (!st.isNull() || !en.isNull()) {
        Pending p;
        p.line = l;
        p.startId = parseInt(attr(st, QStringLiteral("id")), -1);
        p.startIndex = parseInt(attr(st, QStringLiteral("idx")), -1);
        p.endId = parseInt(attr(en, QStringLiteral("id")), -1);
        p.endIndex = parseInt(attr(en, QStringLiteral("idx")), -1);
        m_pending.append(p);
    }
    return l;
}

ItemPtr Reader::pic(const QDomElement &e, const QVector<QDomElement> &inheritsFrom)
{
    const QDomElement cNvPr = childNamed(childNamed(e, "nvPicPr"), "cNvPr");
    if (parseBool(attr(cNvPr, QStringLiteral("hidden")))) return nullptr;
    const QVector<QDomElement> properties = propertiesOf(e, inheritsFrom);
    const QDomElement style = firstWith(QVector<QDomElement>{e} + inheritsFrom, "style");
    const Frame frame = readFrame(firstWith(properties, "xfrm"));
    const QDomElement blipFill = childNamed(e, "blipFill"), blip = childNamed(blipFill, "blip");
    auto p = std::make_shared<PictureItem>();
    decorate(*p, cNvPr, frame, properties, style);
    p->imageId = loadImage(attrNs(blip, ns::R, QStringLiteral("embed")), styleContext());
    media(childNamed(childNamed(e, "nvPicPr"), "nvPr"), *p);

    // The whole picture's place in the frame.
    p->imgRect = pictureRect(blipFill, frame.rect.size());

    // The shape it is cut to.
    QDomElement geom;
    for (const QDomElement &props : properties) {
        geom = childNamed(props, "prstGeom");
        if (!geom.isNull()) break;
    }
    const QString mask = shapeForPreset(attr(geom, QStringLiteral("prst")));
    if (!mask.isEmpty()) p->maskShape = mask;

    // What is done to its colors.
    const QDomElement alpha = childNamed(blip, "alphaModFix");
    if (!alpha.isNull()) p->transparency = std::clamp(1.0 - parsePercent(attr(alpha, QStringLiteral("amt")), 1), 0.0, 1.0);
    p->takeEffects(readBlipEffects(blip, styleContext()));
    remember(cNvPr, p);
    return p;
}

// The movie or sound a picture plays: p:nvPr's a:videoFile, a:quickTimeFile,
// a:audioFile, or a:wavAudioFile. The copy the file holds is kept (the
// p14:media of the extension list names it, or the link itself when it is to
// a part); else the address it links to outside.
void Reader::media(const QDomElement &nvPr, PictureItem &p)
{
    if (!m_ctx.package || nvPr.isNull()) return;
    QDomElement file;
    QString kind;
    for (QDomElement c = nvPr.firstChildElement(); !c.isNull() && kind.isEmpty(); c = c.nextSiblingElement()) {
        const QString n = c.localName();
        if (n == QLatin1String("videoFile") || n == QLatin1String("quickTimeFile")) kind = QStringLiteral("video");
        else if (n == QLatin1String("audioFile") || n == QLatin1String("wavAudioFile")) kind = QStringLiteral("audio");
        if (!kind.isEmpty()) file = c;
    }
    if (file.isNull()) return;
    QStringList ids;
    for (QDomElement ext = childNamed(childNamed(nvPr, "extLst"), "ext"); !ext.isNull(); ext = ext.nextSiblingElement()) {
        const QDomElement copy = childNamed(ext, "media");
        if (copy.isNull()) continue;
        ids << attrNs(copy, ns::R, QStringLiteral("embed"));
        // Where it plays from and to, and how its volume rises and falls: milliseconds, which a file may write with a fraction.
        auto millis = [](const QDomElement &e, const char *name) {
            bool ok = false;
            const double v = attr(e, QLatin1String(name)).toDouble(&ok);
            return ok && v > 0 ? int(std::min(v, 86400000.0) + 0.5) : 0;
        };
        if (const QDomElement trim = childNamed(copy, "trim"); !trim.isNull()) {
            p.mediaTrimStart = millis(trim, "st");
            p.mediaTrimEnd = millis(trim, "end");
        }
        if (const QDomElement fade = childNamed(copy, "fade"); !fade.isNull()) {
            p.mediaFadeIn = std::max(millis(fade, "in"), millis(fade, "inDur"));
            p.mediaFadeOut = std::max(millis(fade, "out"), millis(fade, "outDur"));
        }
    }
    ids << attrNs(file, ns::R, QStringLiteral("link")) << attrNs(file, ns::R, QStringLiteral("embed"));
    for (const QString &id : std::as_const(ids)) {
        if (id.isEmpty()) continue;
        const Relationship r = m_ctx.package->relationship(m_ctx.part, id);
        if (r.id.isEmpty()) continue;
        if (r.external) {
            if (p.mediaLink.isEmpty()) p.mediaLink = r.target;
            continue;
        }
        // The same movie on another picture is the one already kept.
        QHash<QString, QString> &kept = m_ctx.images ? *m_ctx.images : m_images;
        const QString key = QStringLiteral("media:") + r.target.toLower();
        if (const auto known = kept.constFind(key); known != kept.constEnd()) {
            p.mediaId = *known;
            p.mediaLink.clear();
            break;
        }
        const QByteArray bytes = m_ctx.package->part(r.target);
        if (bytes.isEmpty() || !m_ctx.addMedia) continue;
        const qsizetype dot = r.target.lastIndexOf(QLatin1Char('.'));
        const QString extension = dot > r.target.lastIndexOf(QLatin1Char('/')) ? r.target.mid(dot + 1).toLower() : QString();
        p.mediaId = m_ctx.addMedia(bytes, m_ctx.package->contentType(r.target), extension);
        if (!p.mediaId.isEmpty()) kept.insert(key, p.mediaId);
        p.mediaLink.clear();
        break;
    }
    if (!p.mediaId.isEmpty() || !p.mediaLink.isEmpty()) p.mediaKind = kind;
}

} // namespace

ItemPtr readShape(const QDomElement &e, const DrawingContext &ctx, const TextStyleStack &styles, const QVector<QDomElement> &inheritsFrom)
{
    Reader reader(ctx, styles);
    ItemPtr it = reader.shape(e, inheritsFrom);
    reader.join();
    return it;
}

ItemList readShapeTree(const QDomElement &tree, const DrawingContext &ctx, const TextStyleStack &styles)
{
    Reader reader(ctx, styles);
    ItemList out = reader.tree(tree);
    reader.join();
    return out;
}

} // namespace jp::office
