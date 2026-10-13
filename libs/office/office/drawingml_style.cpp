#include "office/drawingml_style.h"

#include "office/xml.h"

#include <QBuffer>
#include <QImage>
#include <QImageReader>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace jp::office {

namespace {

constexpr int kMaxStops = 256;
constexpr double kMaxLineWidth = 1584;   // points: the standard's 20,116,800 EMU
// A blur, glow, or soft edge of more than 200 points is drawn as one of 200: the
// reference presentations' largest are 20, 10, and 7, and one of 1,584 points
// on a small shape makes a picture of gigabytes that takes most of a minute.
constexpr double kMaxEffectRadius = 200;
// A picture that a duotone recolors is decoded and drawn again as a copy; one of
// more pixels than this is kept as it is (the reference presentations' largest has 9 million).
constexpr qint64 kMaxDuotonePixels = 64LL * 1024 * 1024;

ColorRef rgbRef(const QColor &c)
{
    return ColorRef::rgb(QColor(c.red(), c.green(), c.blue()));
}

double transparencyOf(const QColor &c)
{
    return std::clamp(1.0 - c.alphaF(), 0.0, 1.0);
}

QColor colorIn(const QDomElement &e, const StyleContext &sc)
{
    return readColor(e, sc.ctx->theme, sc.ctx->colors, sc.placeholder);
}

// The two colors of a picture's a:duotone (dark, then light), or none.
QVector<QColor> duotoneOf(const QDomElement &blip, const StyleContext &sc)
{
    QVector<QColor> out;
    const QDomElement duo = childNamed(blip, "duotone");
    for (QDomElement c = duo.firstChildElement(); !c.isNull() && out.size() < 2; c = c.nextSiblingElement()) {
        // readColor reads the color a parent holds: give each one its own.
        QDomDocument holder;
        QDomElement wrap = holder.createElement(QStringLiteral("holder"));
        holder.appendChild(wrap);
        wrap.appendChild(holder.importNode(c, true));
        const QColor color = colorIn(wrap, sc);
        if (color.isValid()) out << color;
    }
    return out.size() == 2 ? out : QVector<QColor>();
}

bool isFillElement(const QDomElement &e)
{
    const QString n = e.localName();
    return n == QLatin1String("noFill") || n == QLatin1String("solidFill") || n == QLatin1String("gradFill") || n == QLatin1String("blipFill")
        || n == QLatin1String("pattFill") || n == QLatin1String("grpFill");
}

// The standard's patterns, as the nearest of those the fill has (patternBrushes: the
// seven densities from 94% down to 6%, then horizontal, vertical, cross, and the diagonals).
int patternIndex(const QString &prst)
{
    static const struct { const char *name; int index; } table[] = {
        {"pct5", 6}, {"pct10", 5}, {"pct20", 5}, {"pct25", 4}, {"pct30", 4}, {"pct40", 3}, {"pct50", 3}, {"pct60", 2}, {"pct70", 2},
        {"pct75", 2}, {"pct80", 1}, {"pct90", 0},
        {"horz", 7}, {"ltHorz", 7}, {"dkHorz", 7}, {"narHorz", 7}, {"dashHorz", 7},
        {"vert", 8}, {"ltVert", 8}, {"dkVert", 8}, {"narVert", 8}, {"dashVert", 8},
        {"cross", 9}, {"smGrid", 9}, {"lgGrid", 9}, {"dotGrid", 9}, {"horzBrick", 9},
        {"dnDiag", 11}, {"ltDnDiag", 11}, {"dkDnDiag", 11}, {"wdDnDiag", 11}, {"dashDnDiag", 11}, {"shingle", 11},
        {"upDiag", 10}, {"ltUpDiag", 10}, {"dkUpDiag", 10}, {"wdUpDiag", 10}, {"dashUpDiag", 10},
        {"diagCross", 12}, {"smCheck", 12}, {"lgCheck", 12}, {"trellis", 12}, {"plaid", 12}, {"weave", 12}, {"diagBrick", 12}, {"openDmnd", 12},
        {"smConfetti", 5}, {"lgConfetti", 5}, {"dotDmnd", 5}, {"sphere", 5}, {"divot", 5}, {"solidDmnd", 3},
        {"wave", 7}, {"zigZag", 7}};
    for (const auto &t : table)
        if (prst == QLatin1String(t.name)) return t.index;
    return -1;
}

bool fillFromElement(const QDomElement &e, const StyleContext &sc, Fill *out)
{
    const QString kind = e.localName();
    if (kind == QLatin1String("noFill")) {
        *out = Fill();
        return true;
    }
    if (kind == QLatin1String("solidFill")) {
        const QColor c = colorIn(e, sc);
        *out = c.isValid() ? Fill::solid(rgbRef(c), transparencyOf(c)) : Fill();
        return true;
    }
    if (kind == QLatin1String("grpFill")) {
        *out = sc.groupFill ? *sc.groupFill : Fill();
        return true;
    }
    if (kind == QLatin1String("gradFill")) {
        QVector<GradientStop> stops;
        const QDomElement list = childNamed(e, "gsLst");
        for (QDomElement gs = list.firstChildElement(); !gs.isNull() && stops.size() < kMaxStops; gs = gs.nextSiblingElement()) {
            if (gs.localName() != QLatin1String("gs")) continue;
            const QColor c = colorIn(gs, sc);
            if (!c.isValid()) continue;
            stops.append({std::clamp(parsePercent(attr(gs, QStringLiteral("pos")), 0), 0.0, 1.0), rgbRef(c), transparencyOf(c)});
        }
        std::stable_sort(stops.begin(), stops.end(), [](const GradientStop &a, const GradientStop &b) { return a.pos < b.pos; });
        if (stops.isEmpty()) {
            *out = Fill();
        } else if (stops.size() == 1) {
            *out = Fill::solid(stops[0].color, stops[0].transparency);
        } else {
            Fill f;
            f.type = Fill::Gradient;
            f.stops = stops;
            f.color = stops.first().color;
            f.color2 = stops.last().color;
            const QDomElement path = childNamed(e, "path"), lin = childNamed(e, "lin");
            if (!path.isNull()) {
                const QString kind2 = attr(path, QStringLiteral("path"));
                f.gradType = kind2 == QLatin1String("circle") ? Fill::Radial : kind2 == QLatin1String("shape") ? Fill::PathGrad : Fill::Rectangular;
            } else {
                double angle = std::fmod(parseAngle(attr(lin, QStringLiteral("ang")), 0), 360.0);
                if (angle < 0) angle += 360;
                // A scaled angle is for a square: on any other shape, the direction that
                // is perpendicular to the same slant of the square's lines.
                if (parseBool(attr(lin, QStringLiteral("scaled"))) && sc.frame.width() > 0 && sc.frame.height() > 0) {
                    const double a = angle * M_PI / 180;
                    angle = std::atan2(std::sin(a) / sc.frame.height(), std::cos(a) / sc.frame.width()) * 180 / M_PI;
                    if (angle < 0) angle += 360;
                }
                f.angle = angle;
            }
            *out = f;
        }
        return true;
    }
    if (kind == QLatin1String("blipFill")) {
        const QDomElement blip = childNamed(e, "blip");
        const QString id = loadImage(attrNs(blip, ns::R, QStringLiteral("embed")), sc, duotoneOf(blip, sc), blip);
        Fill f;
        if (!id.isEmpty()) {
            f.type = Fill::Picture;
            f.imageId = id;
            const QDomElement alpha = childNamed(blip, "alphaModFix");
            if (!alpha.isNull()) f.transparency = std::clamp(1.0 - parsePercent(attr(alpha, QStringLiteral("amt")), 1), 0.0, 1.0);
            const QDomElement tile = childNamed(e, "tile");
            if (!tile.isNull()) {
                f.tile = true;
                f.tileScale = std::clamp((parsePercent(attr(tile, QStringLiteral("sx")), 1) + parsePercent(attr(tile, QStringLiteral("sy")), 1)) / 2, 0.01, 100.0);
            }
        }
        *out = f;
        return true;
    }
    if (kind == QLatin1String("pattFill")) {
        const QColor fg = colorIn(childNamed(e, "fgClr"), sc), bg = colorIn(childNamed(e, "bgClr"), sc);
        const QColor front = fg.isValid() ? fg : QColor(Qt::black), back = bg.isValid() ? bg : QColor(Qt::white);
        const int index = patternIndex(attr(e, QStringLiteral("prst")));
        if (index < 0) {
            *out = Fill::solid(rgbRef(front), transparencyOf(front));
        } else {
            Fill f;
            f.type = Fill::Pattern;
            f.pattern = index;
            f.color = rgbRef(front);
            f.color2 = rgbRef(back);
            f.transparency = transparencyOf(front);
            *out = f;
        }
        return true;
    }
    return false;
}

bool fillFromProperties(const QDomElement &properties, const StyleContext &sc, Fill *out)
{
    for (QDomElement c = properties.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (isFillElement(c)) return fillFromElement(c, sc, out);
    return false;
}

// The n-th (from 1) style in a list of the format scheme, or null.
QDomElement styleIn(const QDomElement &formats, const char *list, qint64 n)
{
    if (n < 1) return QDomElement();
    QDomElement c = childNamed(formats, list).firstChildElement();
    for (qint64 i = 1; !c.isNull() && i < n; ++i) c = c.nextSiblingElement();
    return c;
}

StyleContext withPlaceholder(const StyleContext &sc, const QDomElement &ref)
{
    StyleContext out = sc;
    out.placeholder = readColor(ref, sc.ctx->theme, sc.ctx->colors);
    out.fromTheme = true;   // a reference is to one of the theme's styles
    return out;
}


// What a fillRef (or a background's bgRef) points to.
Fill referencedFill(const QDomElement &ref, const StyleContext &sc)
{
    Fill f;
    if (ref.isNull() || sc.ctx->formats.isNull()) return f;
    const qint64 idx = parseInt(attr(ref, QStringLiteral("idx")), 0);
    const QDomElement entry = idx >= 1001 ? styleIn(sc.ctx->formats, "bgFillStyleLst", idx - 1000) : styleIn(sc.ctx->formats, "fillStyleLst", idx);
    if (!entry.isNull()) fillFromElement(entry, withPlaceholder(sc, ref), &f);
    return f;
}

Qt::PenCapStyle capOf(const QString &s, Qt::PenCapStyle fallback)
{
    return s == QLatin1String("rnd") ? Qt::RoundCap : s == QLatin1String("sq") ? Qt::SquareCap : s == QLatin1String("flat") ? Qt::FlatCap : fallback;
}

Arrow arrowOf(const QString &s)
{
    if (s == QLatin1String("triangle")) return Arrow::Triangle;
    if (s == QLatin1String("arrow")) return Arrow::Open;
    if (s == QLatin1String("stealth")) return Arrow::Stealth;
    if (s == QLatin1String("diamond")) return Arrow::Diamond;
    if (s == QLatin1String("oval")) return Arrow::Oval;
    return Arrow::None;
}

int arrowSize(const QString &s)
{
    return s == QLatin1String("sm") ? 0 : s == QLatin1String("lg") ? 2 : 1;
}

void applyEnd(const QDomElement &end, Arrow *arrow, int *size)
{
    if (end.isNull()) return;
    *arrow = arrowOf(attr(end, QStringLiteral("type")));
    *size = (arrowSize(attr(end, QStringLiteral("w"))) + arrowSize(attr(end, QStringLiteral("len"))) + 1) / 2;
}

// Changes `s` by what an a:ln says.
void applyLine(const QDomElement &ln, const StyleContext &sc, Stroke *s)
{
    if (ln.isNull()) return;
    if (hasAttr(ln, QStringLiteral("w"))) s->width = std::clamp(parseLength(attr(ln, QStringLiteral("w")), s->width), 0.25, kMaxLineWidth);
    s->cap = capOf(attr(ln, QStringLiteral("cap")), s->cap);
    const QString cmpd = attr(ln, QStringLiteral("cmpd"));
    if (cmpd == QLatin1String("dbl")) s->compound = Stroke::Double;
    else if (cmpd == QLatin1String("thickThin")) s->compound = Stroke::ThickThin;
    else if (cmpd == QLatin1String("thinThick")) s->compound = Stroke::ThinThick;
    else if (cmpd == QLatin1String("tri")) s->compound = Stroke::Triple;
    else if (cmpd == QLatin1String("sng")) s->compound = Stroke::Single;
    if (hasAttr(ln, QStringLiteral("algn"))) s->inset = attr(ln, QStringLiteral("algn")) == QLatin1String("in");
    for (QDomElement c = ln.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString name = c.localName();
        if (name == QLatin1String("noFill")) {
            s->color = ColorRef();
        } else if (name == QLatin1String("solidFill") || name == QLatin1String("gradFill") || name == QLatin1String("pattFill")) {
            // A gradient line is the color of its first stop; a pattern's is its foreground.
            QDomElement holder = c;
            if (name == QLatin1String("gradFill")) holder = childNamed(childNamed(c, "gsLst"), "gs");
            else if (name == QLatin1String("pattFill")) holder = childNamed(c, "fgClr");
            const QColor color = colorIn(holder, sc);
            if (color.isValid()) {
                s->color = rgbRef(color);
                s->transparency = transparencyOf(color);
            }
        } else if (name == QLatin1String("prstDash")) {
            static const struct { const char *name; Stroke::Dash dash; } dashes[] = {
                {"solid", Stroke::SolidLine}, {"dot", Stroke::RoundDot}, {"sysDot", Stroke::SquareDot}, {"dash", Stroke::DashLine},
                {"sysDash", Stroke::DashLine}, {"dashDot", Stroke::DashDot}, {"sysDashDot", Stroke::DashDot}, {"lgDash", Stroke::LongDash},
                {"lgDashDot", Stroke::LongDashDot}, {"lgDashDotDot", Stroke::LongDashDotDot}, {"sysDashDotDot", Stroke::LongDashDotDot}};
            s->dash = Stroke::SolidLine;
            for (const auto &d : dashes)
                if (attr(c, QStringLiteral("val")) == QLatin1String(d.name)) s->dash = d.dash;
        } else if (name == QLatin1String("custDash")) {
            s->dash = Stroke::DashLine;
        } else if (name == QLatin1String("round")) {
            s->join = Qt::RoundJoin;
        } else if (name == QLatin1String("bevel")) {
            s->join = Qt::BevelJoin;
        } else if (name == QLatin1String("miter")) {
            s->join = Qt::MiterJoin;
        } else if (name == QLatin1String("headEnd")) {
            applyEnd(c, &s->startArrow, &s->startSize);
        } else if (name == QLatin1String("tailEnd")) {
            applyEnd(c, &s->endArrow, &s->endSize);
        }
    }
}

// Effects an a:effectLst has: a shadow (outer, else inner), a glow, and soft edges.
void applyEffects(const QDomElement &list, const StyleContext &sc, Effects *fx)
{
    const QDomElement outer = childNamed(list, "outerShdw"), inner = childNamed(list, "innerShdw");
    const QDomElement shadow = outer.isNull() ? inner : outer;
    if (!shadow.isNull()) {
        const QColor c = colorIn(shadow, sc);
        fx->shadow.on = true;
        fx->shadow.inner = outer.isNull();
        fx->shadow.color = c.isValid() ? rgbRef(c) : ColorRef::rgb(Qt::black);
        fx->shadow.transparency = c.isValid() ? transparencyOf(c) : 0.0;
        fx->shadow.blur = std::clamp(parseLength(attr(shadow, QStringLiteral("blurRad")), 0), 0.0, kMaxEffectRadius);
        fx->shadow.distance = std::clamp(parseLength(attr(shadow, QStringLiteral("dist")), 0), 0.0, 1584.0);
        fx->shadow.angle = std::fmod(parseAngle(attr(shadow, QStringLiteral("dir")), 0), 360.0);
    }
    const QDomElement glow = childNamed(list, "glow");
    if (!glow.isNull()) {
        const QColor c = colorIn(glow, sc);
        fx->glow.on = true;
        fx->glow.size = std::clamp(parseLength(attr(glow, QStringLiteral("rad")), 0), 0.0, kMaxEffectRadius);
        if (c.isValid()) {
            fx->glow.color = rgbRef(c);
            fx->glow.transparency = transparencyOf(c);
        }
    }
    const QDomElement soft = childNamed(list, "softEdge");
    if (!soft.isNull()) fx->softEdge = std::clamp(parseLength(attr(soft, QStringLiteral("rad")), 0), 0.0, kMaxEffectRadius);
}

} // namespace

QDomElement childNamed(const QDomElement &e, const char *local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String(local)) return c;
    return QDomElement();
}

QString imageFormat(const QByteArray &bytes)
{
    auto starts = [&](const char *magic, int n, int at = 0) { return bytes.size() >= at + n && std::memcmp(bytes.constData() + at, magic, size_t(n)) == 0; };
    if (starts("\x89PNG", 4)) return QStringLiteral("png");
    if (starts("\xFF\xD8\xFF", 3)) return QStringLiteral("jpeg");
    if (starts("GIF8", 4)) return QStringLiteral("gif");
    if (starts("II*\0", 4) || starts("MM\0*", 4)) return QStringLiteral("tiff");
    if (starts("\xD7\xCD\xC6\x9A", 4) || starts("\x01\x00\x09\x00", 4) || starts("\x02\x00\x09\x00", 4)) return QStringLiteral("wmf");
    if (starts("\x01\x00\x00\x00", 4) && starts(" EMF", 4, 40)) return QStringLiteral("emf");
    if (starts("BM", 2) && bytes.size() > 14) return QStringLiteral("bmp");
    if (bytes.left(2048).contains("<svg")) return QStringLiteral("svg");
    return QString();
}

QString loadImage(const QString &rId, const StyleContext &sc, const QVector<QColor> &duotone, const QDomElement &from)
{
    const DrawingContext &ctx = *sc.ctx;
    // A theme style's picture is named by the theme part's relationships, one
    // inherited from a layout or master by that part's.
    QString part = sc.fromTheme && !ctx.themePart.isEmpty() ? ctx.themePart : ctx.part;
    if (!sc.fromTheme && !from.isNull())
        for (const auto &inherited : ctx.inheritedParts)
            if (inherited.first == from.ownerDocument()) part = inherited.second;
    if (rId.isEmpty() || !ctx.package || !ctx.addImage || part.isEmpty()) return QString();
    QString key = part + QLatin1Char('#') + rId;
    for (const QColor &c : duotone) key += QLatin1Char('/') + c.name(QColor::HexArgb);
    // The read's own pictures, else the whole import's (a background's or a
    // table cell's picture fill is read without a shape's).
    QHash<QString, QString> *images = sc.images ? sc.images : ctx.images;
    if (images) {
        const auto known = images->constFind(key);
        if (known != images->constEnd()) return *known;
    }
    QString id;
    const Relationship rel = ctx.package->relationship(part, rId);
    if (!rel.id.isEmpty() && !rel.external) {
        QByteArray bytes = ctx.package->part(rel.target);
        QString format = imageFormat(bytes);
        if (duotone.size() == 2 && !bytes.isEmpty()) {
            // a:duotone: the picture's lightness runs from the first color
            // (black) to the second (white).
            QImage img;
            QBuffer in(&bytes);
            QImageReader reader(&in);
            const QSize pixels = reader.size();
            if (pixels.isValid() && pixels.width() * qint64(pixels.height()) <= kMaxDuotonePixels && reader.read(&img)) {
                img = img.convertToFormat(QImage::Format_ARGB32);
                const QColor dark = duotone[0], light = duotone[1];
                for (int y = 0; y < img.height(); ++y) {
                    auto *row = reinterpret_cast<QRgb *>(img.scanLine(y));
                    for (int x = 0; x < img.width(); ++x) {
                        const double t = qGray(row[x]) / 255.0;
                        row[x] = qRgba(int(dark.red() + (light.red() - dark.red()) * t + 0.5), int(dark.green() + (light.green() - dark.green()) * t + 0.5),
                                       int(dark.blue() + (light.blue() - dark.blue()) * t + 0.5), qAlpha(row[x]));
                    }
                }
                QBuffer out(&bytes);
                bytes.clear();
                out.open(QIODevice::WriteOnly);
                img.save(&out, "PNG");
                format = QStringLiteral("png");
            }
        }
        if (!bytes.isEmpty() && !format.isEmpty()) id = ctx.addImage(bytes, format);
    }
    if (images) images->insert(key, id);
    return id;
}

bool hasOwnFill(const QDomElement &properties)
{
    for (QDomElement c = properties.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (isFillElement(c)) return true;
    return false;
}

Fill resolveFill(const QVector<QDomElement> &properties, const QDomElement &style, const StyleContext &sc)
{
    Fill f;
    for (const QDomElement &p : properties)
        if (fillFromProperties(p, sc, &f)) return f;
    return referencedFill(childNamed(style, "fillRef"), sc);
}

Stroke resolveLine(const QVector<QDomElement> &properties, const QDomElement &style, const StyleContext &sc)
{
    Stroke s;
    const QDomElement ref = childNamed(style, "lnRef");
    StyleContext colors = sc;
    if (!ref.isNull()) {
        colors = withPlaceholder(sc, ref);
        if (!sc.ctx->formats.isNull()) applyLine(styleIn(sc.ctx->formats, "lnStyleLst", parseInt(attr(ref, QStringLiteral("idx")), 0)), colors, &s);
    }
    for (int i = properties.size() - 1; i >= 0; --i) applyLine(childNamed(properties[i], "ln"), colors, &s);
    return s;
}

Effects resolveEffects(const QVector<QDomElement> &properties, const QDomElement &style, const StyleContext &sc)
{
    Effects fx;
    for (const QDomElement &p : properties) {
        const QDomElement list = childNamed(p, "effectLst");
        if (!list.isNull()) {
            applyEffects(list, sc, &fx);
            return fx;
        }
    }
    const QDomElement ref = childNamed(style, "effectRef");
    if (!ref.isNull() && !sc.ctx->formats.isNull()) {
        const QDomElement entry = styleIn(sc.ctx->formats, "effectStyleLst", parseInt(attr(ref, QStringLiteral("idx")), 0));
        applyEffects(childNamed(entry, "effectLst"), withPlaceholder(sc, ref), &fx);
    }
    return fx;
}

TextStyleStack withFontReference(const TextStyleStack &base, const QDomElement &style, const QDomElement &own, QDomDocument *keep)
{
    const QDomElement ref = childNamed(style, "fontRef");
    if (ref.isNull()) return base;
    const QDomElement color = ref.firstChildElement();
    const bool major = attr(ref, QStringLiteral("idx")) == QLatin1String("major");
    if (color.isNull() && !major) return base;
    const QString a(ns::A);
    QDomElement root = keep->createElementNS(a, QStringLiteral("a:fontStyles"));
    keep->appendChild(root);
    QDomElement list = keep->createElementNS(a, QStringLiteral("a:lstStyle"));
    root.appendChild(list);
    for (int level = 1; level <= 9; ++level) {
        QDomElement lvl = keep->createElementNS(a, QStringLiteral("a:lvl%1pPr").arg(level));
        QDomElement run = keep->createElementNS(a, QStringLiteral("a:defRPr"));
        if (!color.isNull()) {
            QDomElement fill = keep->createElementNS(a, QStringLiteral("a:solidFill"));
            fill.appendChild(keep->importNode(color, true));
            run.appendChild(fill);
        }
        if (major) {
            QDomElement latin = keep->createElementNS(a, QStringLiteral("a:latin"));
            latin.setAttributeNS(QString(), QStringLiteral("typeface"), QStringLiteral("+mj-lt"));
            run.appendChild(latin);
        }
        lvl.appendChild(run);
        list.appendChild(lvl);
    }
    TextStyleStack out = base;
    if (!out.isEmpty() && !own.isNull() && out.last() == own) out.insert(out.size() - 1, list);
    else out.append(list);
    return out;
}

Fill readFill(const QDomElement &spPr, const QDomElement &style, const DrawingContext &ctx)
{
    StyleContext sc;
    sc.ctx = &ctx;
    return resolveFill({spPr}, style, sc);
}

Stroke readLine(const QDomElement &spPr, const QDomElement &style, const DrawingContext &ctx)
{
    StyleContext sc;
    sc.ctx = &ctx;
    return resolveLine({spPr}, style, sc);
}

Effects readEffects(const QDomElement &spPr, const QDomElement &style, const DrawingContext &ctx)
{
    StyleContext sc;
    sc.ctx = &ctx;
    return resolveEffects({spPr}, style, sc);
}

Fill readBackground(const QDomElement &bg, const DrawingContext &ctx)
{
    StyleContext sc;
    sc.ctx = &ctx;
    Fill f;
    if (fillFromProperties(childNamed(bg, "bgPr"), sc, &f)) return f;
    return referencedFill(childNamed(bg, "bgRef"), sc);
}

} // namespace jp::office
