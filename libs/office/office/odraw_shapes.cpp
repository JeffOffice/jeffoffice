// The shapes of a binary drawing (odraw.h): shape containers and groups become
// the placed objects of libs/draw, with what their property tables say.

#include "office/odraw.h"

#include "core/blipeffects.h"
#include "core/shapetypes.h"
#include "office/odraw_path.h"
#include "office/odraw_props.h"
#include "office/odraw_read.h"
#include "office/odraw_style.h"

#include <QtMath>
#include <algorithm>
#include <functional>
#include <memory>

namespace jp::office {

namespace {

using namespace odraw;

constexpr int kMaxDepth = 64;
constexpr int kMaxShapes = 100000;
constexpr double kEmuPerPoint = 12700;

// The flags of a shape record (OfficeArtFSP).
enum : quint32 {
    fGroup = 0x1, fPatriarch = 0x4, fDeleted = 0x8, fOleShape = 0x10, fFlipH = 0x40, fFlipV = 0x80, fBackground = 0x400,
};

// What a shape container holds.
struct Records {
    bool shape = false;           // has a shape record
    int type = 0;
    quint32 flags = 0;
    bool groupRect = false;       // a group's child coordinates
    double gl = 0, gt = 0, gr = 0, gb = 0;
    bool childAnchor = false;     // a place in its group's child coordinates
    double cl = 0, ct = 0, cr = 0, cb = 0;
    bool clientAnchor = false;
    QByteArray anchor, textbox, data;
    bool hasTextbox = false, hasData = false;
    Props props;
};

qsizetype endOf(const OdrawRecord &r) { return qsizetype(qint64(r.offset) + r.length); }

Records readRecords(const QByteArray &d, const OdrawRecord &container, const Props *defaults)
{
    Records r;
    r.props.setFallback(defaults);
    for (const OdrawRecord &c : odrawRecords(d, container.offset, endOf(container))) {
        switch (c.type) {
        case 0xF00A:
            if (c.length >= 8) {
                r.shape = true;
                r.type = c.instance;
                r.flags = u32(d, c.offset + 4);
            }
            break;
        case 0xF009:
            if (c.length >= 16) {
                r.groupRect = true;
                r.gl = i32(d, c.offset);
                r.gt = i32(d, c.offset + 4);
                r.gr = i32(d, c.offset + 8);
                r.gb = i32(d, c.offset + 12);
            }
            break;
        case 0xF00B: case 0xF121: case 0xF122:
            r.props.add(d, c);
            break;
        case 0xF00F:
            if (c.length >= 16) {
                r.childAnchor = true;
                r.cl = i32(d, c.offset);
                r.ct = i32(d, c.offset + 4);
                r.cr = i32(d, c.offset + 8);
                r.cb = i32(d, c.offset + 12);
            }
            break;
        case 0xF010:
            r.clientAnchor = true;
            r.anchor = d.mid(c.offset, qsizetype(c.length));
            break;
        case 0xF00D:
            r.hasTextbox = true;
            r.textbox = d.mid(c.offset, qsizetype(c.length));
            break;
        case 0xF011:
            r.hasData = true;
            r.data = d.mid(c.offset, qsizetype(c.length));
            break;
        default:
            break;
        }
    }
    return r;
}

// A group's child coordinates and where they fall on the page.
struct Frame {
    bool valid = false;
    double left = 0, top = 0, width = 1, height = 1;
    QRectF page;
    QRectF map(double l, double t, double r, double b) const
    {
        const double sx = width != 0 ? page.width() / width : 1, sy = height != 0 ? page.height() / height : 1;
        return QRectF(QPointF(page.left() + (l - left) * sx, page.top() + (t - top) * sy),
                      QPointF(page.left() + (r - left) * sx, page.top() + (b - top) * sy)).normalized();
    }
};

double emuToPoints(qint64 emu) { return double(emu) / kEmuPerPoint; }

QMarginsF textInsets(const Props &p)
{
    return QMarginsF(emuToPoints(p.integer(0x0081, 91440)), emuToPoints(p.integer(0x0082, 45720)), emuToPoints(p.integer(0x0083, 91440)),
                     emuToPoints(p.integer(0x0084, 45720)));
}

VAlign verticalAlign(const Props &p)
{
    switch (p.value(0x0087, 0)) {
    case 1: case 4: return VAlign::Middle;
    case 2: case 5: case 7: case 9: return VAlign::Bottom;
    default: return VAlign::Top;
    }
}

// Whether the text is anchored centered (the kinds of anchor that say "centered"): the block of it is in the middle of the frame across.
bool centeredBlock(const Props &p)
{
    const quint32 anchor = p.value(0x0087, 0);
    return anchor == 3 || anchor == 4 || anchor == 5 || anchor == 8 || anchor == 9;
}

// Turns an item and what is in it over, about the vertical (or horizontal) line at `axis`.
void flipItem(Item &it, bool horizontal, double axis)
{
    if (it.type() == ItemType::Group) {
        auto &g = static_cast<GroupItem &>(it);
        for (const ItemPtr &c : g.children) flipItem(*c, horizontal, axis);
        g.syncRect();
        return;
    }
    if (it.type() == ItemType::Line) {
        auto &l = static_cast<LineItem &>(it);
        auto mirror = [&](QPointF p) { return horizontal ? QPointF(2 * axis - p.x(), p.y()) : QPointF(p.x(), 2 * axis - p.y()); };
        l.p1 = mirror(l.p1);
        l.p2 = mirror(l.p2);
        l.syncRect();
        return;
    }
    QPointF c = it.rect.center();
    if (horizontal) c.setX(2 * axis - c.x()); else c.setY(2 * axis - c.y());
    it.rect.moveCenter(c);
    if (horizontal) it.flipH = !it.flipH; else it.flipV = !it.flipV;
    it.rotation = std::fmod(360 - it.rotation, 360.0);
}

// The drawing group's default properties, from the context's records.
Props defaultProps(const OdrawContext &ctx)
{
    Props p;
    for (const OdrawRecord &r : odrawRecords(ctx.defaults))
        if (r.type == 0xF00B || r.type == 0xF121 || r.type == 0xF122) p.add(ctx.defaults, r);
    return p;
}

class Reader {
public:
    Reader(const QByteArray &data, const OdrawContext &ctx) : d(data), ctx(ctx), defaults(defaultProps(ctx)) {}

    // The shapes of a group container into `out`; the top group's (the
    // patriarch's) go straight in, any other group's become a group item.
    void readGroup(const OdrawRecord &group, int depth, const Frame &parent, ItemList &out)
    {
        if (depth > kMaxDepth || spent()) return;
        if (depth > 0) take();
        const QVector<OdrawRecord> kids = odrawRecords(d, group.offset, endOf(group));
        // Its first shape container describes the group (when it is made for one).
        Records own;
        int first = 0;
        if (!kids.isEmpty() && kids[0].type == 0xF004) {
            own = readRecords(d, kids[0], &defaults);
            if (own.groupRect || (own.flags & (fGroup | fPatriarch))) first = 1; else own = Records();
        }
        const bool patriarch = depth == 0 || (own.flags & fPatriarch);
        if (own.flags & fDeleted) return;
        if (own.props.flag(0x03BF, 0x02, false) && !patriarch) return;
        QRectF frame;
        const bool placed = place(own, parent, &frame);
        Frame inner;
        if (!patriarch && placed && own.groupRect) {
            inner.valid = true;
            inner.left = own.gl;
            inner.top = own.gt;
            inner.width = own.gr - own.gl;
            inner.height = own.gb - own.gt;
            inner.page = frame;
        }
        ItemList children;
        for (int i = first; i < kids.size() && !spent(); ++i) {
            if (kids[i].type == 0xF004) {
                if (ItemPtr it = readShape(kids[i], inner)) children.push_back(it);
            } else if (kids[i].type == 0xF003) {
                readGroup(kids[i], depth + 1, inner, children);
            }
        }
        if (patriarch) {
            out.insert(out.end(), children.begin(), children.end());
            return;
        }
        if (children.empty()) return;
        auto g = std::make_shared<GroupItem>();
        g->children = children;
        if (placed) {
            // Turned, then flipped, as a shape is.
            const double rotation = std::fmod(own.props.fixed(0x0004, 0), 360.0);
            if (rotation != 0) g->rotateAround(rotation, frame.center());
            if (own.flags & fFlipH) for (const ItemPtr &c : g->children) flipItem(*c, true, frame.center().x());
            if (own.flags & fFlipV) for (const ItemPtr &c : g->children) flipItem(*c, false, frame.center().y());
        }
        g->syncRect();
        g->name = own.props.text(0x0380);
        g->altText = own.props.text(0x0381);
        g->hyperlink = hyperlinkOf(own.props.complex(0x0382));
        client(*g, own);
        out.push_back(g);
    }

    // One shape container: its item, or none when it is deleted, hidden, the
    // background, or no shape.
    ItemPtr readShape(const OdrawRecord &container, const Frame &parent)
    {
        if (spent()) return nullptr;
        Records s = readRecords(d, container, &defaults);
        if (!s.shape || (s.flags & (fDeleted | fBackground | fGroup))) return nullptr;
        // What it leaves out, the shape it takes after has.
        if (s.hasData && ctx.inherits) {
            const QByteArray table = ctx.inherits(s.data);
            Props base;
            for (const OdrawRecord &r : odrawRecords(table))
                if (r.type == 0xF00B || r.type == 0xF121 || r.type == 0xF122) base.add(table, r);
            s.props.inherit(base);
        }
        if (s.props.flag(0x03BF, 0x02, false)) return nullptr;   // hidden
        take();
        QRectF rect;
        place(s, parent, &rect);
        ItemPtr it = makeItem(s, rect);
        if (it && ctx.richLook)
            if (const QByteArray xml = s.props.complex(0x03A9); !xml.isEmpty()) ctx.richLook(*it, xml);
        if (it) client(*it, s);
        return it;
    }

private:
    // Whether no more shapes are read: this drawing has as many as one has (a drawing's own limit), or the host's budget is spent.
    bool spent() const
    {
        if (shapes < kMaxShapes && (!ctx.budget || ctx.budget->left > 0)) return false;
        if (ctx.budget) ctx.budget->refused = true;
        return true;
    }
    void take()
    {
        ++shapes;
        if (ctx.budget) --ctx.budget->left;
    }

    // Where a shape is: in its group's child coordinates when it has them there
    // and the group has its place, else by the host's anchor.
    bool place(const Records &s, const Frame &parent, QRectF *rect) const
    {
        if (s.childAnchor && parent.valid) {
            *rect = parent.map(s.cl, s.ct, s.cr, s.cb);
            return true;
        }
        if (s.clientAnchor && ctx.clientAnchor) {
            *rect = ctx.clientAnchor(s.anchor);
            return true;
        }
        return false;
    }

    void client(Item &it, const Records &s) const
    {
        if (ctx.client && (s.hasTextbox || s.hasData)) ctx.client(it, s.textbox, s.data);
    }

    // The fill, line, and shadow. A kind of shape that has no fill or no line of itself keeps that over what the drawing's defaults
    // say (a picture frame or a line isn't filled whatever they are).
    static void look(Item &it, const Records &s, const OdrawContext &ctx, bool filledByDefault, bool lineByDefault)
    {
        Props kind;
        if (!filledByDefault) kind.set(0x01BF, 0x00100000);
        if (!lineByDefault) kind.set(0x01FF, 0x00080000);
        kind.setFallback(s.props.fallback());
        Props p = s.props;
        p.setFallback(&kind);
        it.fill = readFill(p, ctx, filledByDefault, it.rect);
        it.stroke = readStroke(p, ctx, lineByDefault);
        readShadow(p, ctx, it.fx);
    }

    // The frame, turn, and flips of an item. The format turns a shape and then
    // flips it (so the turn of a shape flipped one way shows the other way
    // round), where an item flips and then turns; a shape turned by about a
    // quarter keeps its frame with the sides exchanged. `mirrored` is for a
    // preset drawn the other way up than the format's.
    static void frameOf(Item &it, const Records &s, const QRectF &rect, bool mirrored = false)
    {
        double angle = std::fmod(s.props.fixed(0x0004, 0), 360.0);
        if (angle < 0) angle += 360;
        it.rect = rect;
        if (!rect.isNull() && ((angle >= 45 && angle < 135) || (angle >= 225 && angle < 315))) {
            const QPointF c = rect.center();
            it.rect = QRectF(c.x() - rect.height() / 2, c.y() - rect.width() / 2, rect.height(), rect.width());
        }
        it.flipH = (s.flags & fFlipH) != 0;
        it.flipV = (s.flags & fFlipV) != 0;
        if (mirrored) {
            it.flipV = !it.flipV;
            angle = std::fmod(360 - angle, 360.0);
        }
        it.rotation = it.flipH != it.flipV ? std::fmod(360 - angle, 360.0) : angle;
    }

    ItemPtr makeItem(const Records &s, const QRectF &rect) const
    {
        const Props &p = s.props;
        const int type = s.type;
        const bool hasText = s.hasTextbox || p.value(0x0080, 0) != 0;
        const quint32 blip = p.value(0x0104, 0);
        ItemPtr item;
        if (type == 75 || (blip > 0 && blip < 0x7FFFFFFF)) {
            item = makePicture(s, rect);
        } else if (type == 20 || type == 32) {
            item = makeLine(s, rect);
        } else if (type >= 33 && type <= 40) {
            auto sh = std::make_shared<ShapeItem>();
            frameOf(*sh, s, rect);
            look(*sh, s, ctx, false, true);
            sh->customPath = connectorPath(type, p, sh->rect.size());
            item = sh;
        } else if (type >= 136 && type <= 175 && !p.text(0x00C0).isEmpty()) {
            item = makeTextArt(s, rect);
        } else if (type == 202 || (hasText && (type == 1 || type == 201))) {
            auto t = std::make_shared<TextItem>();
            frameOf(*t, s, rect);
            look(*t, s, ctx, false, false);
            t->insets = textInsets(p);
            t->valign = verticalAlign(p);
            t->noWrap = p.value(0x0085, 0) == 2;
            t->centerBlock = ctx.presentation && centeredBlock(p);
            const quint32 flow = p.value(0x0088, 0);
            t->vertical = flow == 1 || flow == 2 || flow == 3 || flow == 5;
            if (p.flag(0x00BF, 0x01, false)) t->autofit = TextItem::GrowBox;
            item = t;
        } else {
            auto sh = std::make_shared<ShapeItem>();
            // The numbered trapezoid is wide at the top; JeffOffice's is wide at the bottom.
            frameOf(*sh, s, rect, type == 8 && !p.has(0x0145));
            look(*sh, s, ctx, true, true);
            const QString preset = presetForDrawingShapeType(type);
            sh->shape = preset.isEmpty() ? QStringLiteral("rect") : preset;
            sh->adj = adjustments(type, p, sh->rect.size());
            // (A presentation's arc keeps the ellipse it is part of, in its vertices, with no segments to make a path of.)
            sh->customPath = ctx.presentation && type == 19 ? arcPath(p, sh->rect.size()) : freeformPath(p, sh->rect.size());
            sh->insets = textInsets(p);
            sh->valign = verticalAlign(p);
            sh->noWrap = p.value(0x0085, 0) == 2;
            sh->centerBlock = ctx.presentation && centeredBlock(p);
            item = sh;
        }
        item->name = p.text(0x0380);
        item->altText = p.text(0x0381);
        item->hyperlink = hyperlinkOf(p.complex(0x0382));
        return item;
    }

    // Text with a shape (shape numbers 136 to 175): the words, font, size, and settings.
    ItemPtr makeTextArt(const Records &s, const QRectF &rect) const
    {
        const Props &p = s.props;
        auto art = std::make_shared<TextArtItem>();
        frameOf(*art, s, rect);
        look(*art, s, ctx, true, false);
        art->text = p.text(0x00C0).replace(QLatin1Char('\r'), QLatin1Char('\n'));
        if (const QString font = p.text(0x00C5); !font.isEmpty()) art->font = font;
        const QString transform = textArtForShapeType(s.type);
        if (!transform.isEmpty()) art->transform_ = transform;
        art->size = std::clamp(p.fixed(0x00C3, 36), 1.0, 1000.0);
        art->spacing = std::clamp(p.fixed(0x00C4, 1.0), 0.5, 2.0);
        // Stretch, center, left, right, letter, and word: here left, center, right, word, letter, and stretch.
        static const int align[] = {5, 1, 0, 2, 4, 3};
        const quint32 alignment = p.value(0x00C2, 1);
        if (alignment < 6) art->align = align[alignment];
        art->italic = p.flag(0x00FF, 0x10, false);
        art->bold = p.flag(0x00FF, 0x20, false);
        art->evenHeight = p.flag(0x00FF, 0x80, false);
        art->vertical = p.flag(0x00FF, 0x2000, false);
        return art;
    }

    ItemPtr makePicture(const Records &s, const QRectF &rect) const
    {
        const Props &p = s.props;
        auto pic = std::make_shared<PictureItem>();
        frameOf(*pic, s, rect);
        look(*pic, s, ctx, false, false);
        const quint32 index = p.value(0x0104, 0);
        if (ctx.picture && index > 0 && index < 0x7FFFFFFF) pic->imageId = ctx.picture(int(index));
        // The crop is a share of the picture taken off each edge.
        const double l = p.fixed(0x0102, 0), r = p.fixed(0x0103, 0), t = p.fixed(0x0100, 0), b = p.fixed(0x0101, 0);
        const double w = pic->rect.width() / std::max(0.01, 1 - l - r), h = pic->rect.height() / std::max(0.01, 1 - t - b);
        pic->imgRect = QRectF(-l * w, -t * h, w, h);
        pic->brightness = pictureBrightness(p);
        pic->contrast = pictureContrast(p);
        // Where the host makes the picture as adjusted, it does.
        if (ctx.adjusted && !pic->imageId.isEmpty() && (pic->brightness != 0 || pic->contrast != 0)) {
            const QString changed = ctx.adjusted(pic->imageId, pic->brightness, pic->contrast);
            if (!changed.isEmpty() && changed != pic->imageId) {
                pic->imageId = changed;
                pic->brightness = pic->contrast = 0;
            }
        }
        // Left to the item, they are drawn as a slide's picture is, where a contrast is a share (see blipeffects.h), not the factor the file
        // has: so as to look as the picture the host would have made, it is the share that makes the same factor.
        if (pic->contrast != 0) pic->contrast = contrastOfFactor((100 + pic->contrast) / 100) * 100;
        if (p.flag(0x013F, 0x04, false)) pic->recolor = PictureItem::Grayscale;
        else if (p.flag(0x013F, 0x02, false)) pic->recolor = PictureItem::BlackWhite;
        if (p.has(0x0107)) {
            pic->hasTransparentColor = true;
            pic->transparentColor = colorProperty(p, ctx, 0x0107);
        }
        return pic;
    }

    ItemPtr makeLine(const Records &s, const QRectF &rect) const
    {
        auto line = std::make_shared<LineItem>();
        frameOf(*line, s, rect);
        look(*line, s, ctx, false, true);
        // From the frame's top left to its bottom right, flipped and turned with it.
        const QTransform t = line->transform();
        line->p1 = t.map(QPointF(0, 0));
        line->p2 = t.map(QPointF(line->rect.width(), line->rect.height()));
        line->flipH = line->flipV = false;
        line->syncRect();
        return line;
    }

    // The adjustment values a preset takes, from the format's (in 21600ths of
    // the shape's own coordinates), for the shapes whose meaning is the same.
    QVector<double> adjustments(int type, const Props &p, const QSizeF &size) const
    {
        auto share = [&](int i, double fallback) { return std::clamp(p.integer(quint16(0x0147 + i), int(fallback)) / 21600.0, 0.0, 1.0); };
        const double m = std::max(1e-6, std::min(size.width(), size.height()));
        switch (type) {
        case 2: case 5: case 7: case 9: case 10: case 11:
            return p.has(0x0147) ? QVector<double>{share(0, 0)} : QVector<double>();
        case 13: case 66: case 67: case 68: {
            // An arrow's shaft (a share of its width) and head (a share of the shorter side).
            const bool across = type == 13 || type == 66;   // runs along the width
            const double length = across ? size.width() : size.height();
            const double headAt = share(0, type == 13 || type == 67 ? 16200 : 5400);
            const double head = (type == 13 || type == 67 ? 1 - headAt : headAt) * length / m;
            return {std::clamp(1 - 2 * share(1, 5400), 0.0, 1.0), std::clamp(head, 0.0, 1.0)};
        }
        case 15: case 55: {
            // The pentagon arrow's and the chevron's point: the first value is where it starts, 16200 of 21600 of the width unless it says
            // (JeffOffice's own is half the shorter side).
            if (!ctx.presentation) return {};
            return {std::clamp((1 - share(0, 16200)) * size.width() / m, 0.0, 1e4)};
        }
        default:
            return {};
        }
    }

    const QByteArray &d;
    const OdrawContext &ctx;
    const Props defaults;
    int shapes = 0;
};

bool hasBackgroundFlag(const QByteArray &d, const OdrawRecord &container)
{
    for (const OdrawRecord &c : odrawRecords(d, container.offset, endOf(container)))
        if (c.type == 0xF00A) return c.length >= 8 && (u32(d, c.offset + 4) & fBackground);
    return false;
}

// The first shape container with the background flag, in a drawing's containers.
bool findBackground(const QByteArray &d, const OdrawRecord &r, int depth, const OdrawContext &ctx, const Props &defaults, Fill *fill, int *budget)
{
    if (depth > kMaxDepth) return false;
    for (const OdrawRecord &c : odrawRecords(d, r.offset, endOf(r))) {
        if (--*budget < 0) return false;
        // The host's budget pays for the records looked at too: a file with a drawing for each of thousands of slides, none with a
        // background, would have them all walked.
        if (ctx.budget && --ctx.budget->left < 0) {
            ctx.budget->refused = true;
            return false;
        }
        if (c.type == 0xF004) {
            if (hasBackgroundFlag(d, c)) {
                *fill = readFill(readRecords(d, c, &defaults).props, ctx, true);
                return true;
            }
        } else if (c.type == 0xF002 || c.type == 0xF003) {
            if (findBackground(d, c, depth + 1, ctx, defaults, fill, budget)) return true;
        }
    }
    return false;
}

} // namespace

ItemList readOdrawShapes(const QByteArray &data, const OdrawContext &ctx)
{
    ItemList out;
    Reader reader(data, ctx);
    for (const OdrawRecord &r : odrawRecords(data)) {
        if (r.type == 0xF002) {
            for (const OdrawRecord &c : odrawRecords(data, r.offset, endOf(r)))
                if (c.type == 0xF003) {
                    reader.readGroup(c, 0, Frame(), out);
                    break;
                }
        } else if (r.type == 0xF003) {
            reader.readGroup(r, 0, Frame(), out);
        } else if (r.type == 0xF004) {
            if (ItemPtr it = reader.readShape(r, Frame())) out.push_back(it);
        } else {
            continue;
        }
        break;
    }
    return out;
}

Fill odrawBackground(const QByteArray &data, const OdrawContext &ctx)
{
    Fill fill;
    if (ctx.budget && ctx.budget->left <= 0) {
        ctx.budget->refused = true;
        return fill;
    }
    const Props defaults = defaultProps(ctx);
    int budget = kMaxShapes * 2;
    for (const OdrawRecord &r : odrawRecords(data)) {
        if (r.type == 0xF002 || r.type == 0xF003) {
            findBackground(data, r, 0, ctx, defaults, &fill, &budget);
            break;
        }
    }
    return fill;
}

} // namespace jp::office
