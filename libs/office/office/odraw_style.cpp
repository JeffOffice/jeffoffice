// Colors, fills, lines, shadows, and hyperlinks of binary drawings (odraw_style.h).

#include "office/odraw_style.h"

#include "office/odraw_read.h"

#include <QtMath>
#include <algorithm>

namespace jp::office::odraw {

namespace {

// Property numbers.
enum : quint16 {
    kFillType = 0x0180, kFillColor = 0x0181, kFillOpacity = 0x0182, kFillBackColor = 0x0183, kFillBackOpacity = 0x0184,
    kFillBlip = 0x0186, kFillAngle = 0x018B, kFillFocus = 0x018C, kFillShadeColors = 0x0197, kFillShadeType = 0x019C, kFillStyleFlags = 0x01BF,
    kLineColor = 0x01C0, kLineOpacity = 0x01C1, kLineBackColor = 0x01C2, kLineWidth = 0x01CB, kLineStyle = 0x01CD,
    kLineDashing = 0x01CE, kLineStartArrow = 0x01D0, kLineEndArrow = 0x01D1, kLineStartArrowWidth = 0x01D2,
    kLineStartArrowLength = 0x01D3, kLineEndArrowWidth = 0x01D4, kLineEndArrowLength = 0x01D5, kLineJoin = 0x01D6,
    kLineCap = 0x01D7, kLineStyleFlags = 0x01FF,
    kShadowColor = 0x0201, kShadowOpacity = 0x0204, kShadowOffsetX = 0x0205, kShadowOffsetY = 0x0206, kShadowFlags = 0x023F,
};

constexpr double kEmuPerPoint = 12700;

quint32 defaultColor(quint16 id)
{
    switch (id) {
    case kFillColor: case kFillBackColor: case kLineBackColor: return 0x00FFFFFF;
    case kShadowColor: return 0x00808080;
    default: return 0;
    }
}

// The system colors by the numbers Windows gives them.
QColor windowsColor(int index)
{
    static const QRgb colors[] = {0xD4D0C8, 0x3A6EA5, 0x0A246A, 0x808080, 0xD4D0C8, 0xFFFFFF, 0x000000, 0x000000, 0x000000, 0xFFFFFF, 0xD4D0C8, 0xD4D0C8, 0x808080,
                                  0x0A246A, 0xFFFFFF, 0xD4D0C8, 0x808080, 0x808080, 0x000000, 0xD4D0C8, 0xFFFFFF, 0x404040, 0xD4D0C8, 0x000000, 0xFFFFE1};
    return index >= 0 && index < int(sizeof(colors) / sizeof(colors[0])) ? QColor::fromRgb(colors[index]) : QColor(Qt::black);
}

// A system color's function: the color made darker or lighter, shifted, or inverted.
QColor modified(const QColor &c, quint32 raw)
{
    const int function = (raw >> 8) & 0x0F, extra = (raw >> 8) & 0xF0, param = (raw >> 16) & 0xFF;
    int v[3] = {c.red(), c.green(), c.blue()};
    for (int &x : v) {
        switch (function) {
        case 1: x = (param * x) >> 8; break;
        case 2: x = ((255 - param) * 255 + param * x) >> 8; break;
        case 3: x = std::min(255, x + param); break;
        case 4: x = std::max(0, x - param); break;
        case 5: x = std::max(0, param - x); break;
        case 6: x = x >= param ? 255 : 0; break;
        default: break;
        }
        if (extra & 0x40) x ^= 0x80;
        if (extra & 0x80) x = 255 - x;
    }
    return QColor(v[0], v[1], v[2]);
}

QColor resolveColor(quint32 raw, const Props &p, const OdrawContext &ctx, const QColor &fallback, int depth);

QColor colorFor(const Props &p, const OdrawContext &ctx, quint16 id, int depth)
{
    if (depth > 3) return QColor(Qt::black);   // colors that refer to each other
    return resolveColor(p.value(id, defaultColor(id)), p, ctx, QColor::fromRgb(defaultColor(id)), depth);
}

// A system color that stands for one of the shape's own.
QColor referenced(int index, const Props &p, const OdrawContext &ctx, int depth)
{
    switch (index) {
    case 0xF0: case 0xF4: return colorFor(p, ctx, kFillColor, depth);
    case 0xF1: return colorFor(p, ctx, p.flag(kLineStyleFlags, 0x08, true) ? kLineColor : kFillColor, depth);
    case 0xF2: return colorFor(p, ctx, kLineColor, depth);
    case 0xF3: return colorFor(p, ctx, kShadowColor, depth);
    case 0xF5: return colorFor(p, ctx, kFillBackColor, depth);
    case 0xF6: return colorFor(p, ctx, kLineBackColor, depth);
    case 0xF7: return colorFor(p, ctx, p.flag(kFillStyleFlags, 0x10, true) ? kFillColor : kLineColor, depth);
    default: return QColor(Qt::black);
    }
}

QColor resolveColor(quint32 raw, const Props &p, const OdrawContext &ctx, const QColor &fallback, int depth)
{
    const int red = raw & 0xFF, green = (raw >> 8) & 0xFF, blue = (raw >> 16) & 0xFF;
    const quint32 flags = raw >> 24;
    if (flags & 0x10) {
        const QColor base = red >= 0xF0 ? referenced(red, p, ctx, depth + 1) : windowsColor(red);
        return modified(base, raw);
    }
    if (flags & 0x08) {
        const QColor c = ctx.schemeColor ? ctx.schemeColor(red) : QColor();
        return c.isValid() ? QColor(c.red(), c.green(), c.blue()) : fallback;
    }
    return QColor(red, green, blue);
}

double opacityOf(const Props &p, quint16 id) { return std::clamp(p.fixed(id, 1.0), 0.0, 1.0); }

// A shaded fill's stops with each stretch between two colors made a curve: the position eased by the sigma shape (when the shade type has
// its bit), and the colors blended in linear light (the gamma bit), as the program draws them.
QVector<GradientStop> shaded(const QVector<GradientStop> &stops, quint32 shadeType)
{
    const bool gamma = shadeType & 1, sigma = shadeType & 2;
    if ((!gamma && !sigma) || stops.size() < 2) return stops;
    constexpr int kSteps = 8;
    constexpr double kGamma = 2.2;
    QVector<GradientStop> out;
    for (int i = 0; i + 1 < stops.size(); ++i) {
        const GradientStop &a = stops[i], &b = stops[i + 1];
        out << a;
        const QColor from = a.color.rgbValue(), to = b.color.rgbValue();
        if (from == to || b.pos - a.pos < 1e-9) continue;
        for (int k = 1; k < kSteps; ++k) {
            const double u = double(k) / kSteps, e = sigma ? u * u * (3 - 2 * u) : u;
            auto blend = [&](double x, double y) { return gamma ? std::pow((1 - e) * std::pow(x, kGamma) + e * std::pow(y, kGamma), 1 / kGamma) : x + (y - x) * e; };
            out << GradientStop{a.pos + (b.pos - a.pos) * u, ColorRef::rgb(QColor::fromRgbF(float(blend(from.redF(), to.redF())), float(blend(from.greenF(), to.greenF())),
                                                                                                float(blend(from.blueF(), to.blueF())))),
                                a.transparency + (b.transparency - a.transparency) * u};
        }
    }
    out << stops.last();
    return out;
}

// A gradient: the colors run from the fill color's end to the back color's, or as the list of colors says, as the focus lays them out.
Fill gradientFill(const Props &p, const OdrawContext &ctx, int type, const QColor &fillColor, const QColor &backColor, double opacity, double backOpacity)
{
    // The colors along the ramp, from 0 (the fill color's end) to 1 (the back color's), with their opacity.
    struct Color { double at; QColor color; double alpha; };
    QVector<Color> ramp;
    const QByteArray list = p.complex(kFillShadeColors);
    const ArrayView colors = arrayOf(list);
    const bool hasList = colors.count >= 2 && colors.size >= 8;
    if (hasList) {
        for (int i = 0; i < colors.count; ++i) {
            const double at = std::clamp(i32(list, colors.offset(i) + 4) / 65536.0, 0.0, 1.0);
            ramp << Color{at, resolveColor(u32(list, colors.offset(i)), p, ctx, QColor(Qt::white), 0), opacity + (backOpacity - opacity) * at};
        }
        std::stable_sort(ramp.begin(), ramp.end(), [](const Color &a, const Color &b) { return a.at < b.at; });
    } else {
        ramp << Color{0, fillColor, opacity} << Color{1, backColor, backOpacity};
    }
    // The focus: 0 runs the colors across; at 50 they run out to the middle and back (the first color is at both ends and the last in
    // the middle); at 100 they run the other way; below zero they start from the other end.
    const int focus = std::clamp(p.integer(kFillFocus, 0), -100, 100);
    const double a = std::abs(focus) / 100.0;
    if (focus < 0) {
        std::reverse(ramp.begin(), ramp.end());
        for (Color &c : ramp) c.at = 1 - c.at;
    }
    QVector<GradientStop> stops;
    QColor last;
    auto stop = [&](double pos, const Color &c) {
        if (!stops.isEmpty() && std::abs(stops.last().pos - pos) < 1e-9 && last == c.color && std::abs(stops.last().transparency - (1 - c.alpha)) < 1e-9) return;
        stops << GradientStop{pos, ColorRef::rgb(c.color), 1 - c.alpha};
        last = c.color;
    };
    if (1 - a > 1e-9)
        for (const Color &c : std::as_const(ramp)) stop(c.at * (1 - a), c);
    if (a > 1e-9)
        for (int i = int(ramp.size()) - 1; i >= 0; --i) stop(1 - ramp[i].at * a, ramp[i]);
    // The shading curve of two colors: the one the shape names (its writers name gamma and sigma). Where it names none an opaque gradient has
    // gamma and sigma, and one with a color clear in part is straight, as the program draws the old files that never say (its pictures of
    // one deck's opaque background are curved, and of another's translucent gradients are straight). An opacity of 0xFF00 of 0x10000 or
    // more counts as none: a writer that rounds 100 percent to 0xFFFF has not made its color clear. A list of colors is drawn as it stands.
    constexpr double kOpaque = 0xFF00 / 65536.0;
    if (!hasList) stops = shaded(stops, p.value(kFillShadeType, opacity < kOpaque || backOpacity < kOpaque ? 0 : 3) & 0xF);
    // From the center or along the outline, the first color is the outer one.
    const bool fromCenter = type == 5 || type == 6;
    if (fromCenter) {
        for (GradientStop &s : stops) s.pos = 1 - s.pos;
        std::reverse(stops.begin(), stops.end());
    }
    Fill f;
    f.type = Fill::Gradient;
    f.stops = stops;
    f.color = stops.first().color;
    f.color2 = stops.last().color;
    f.gradType = type == 5 ? Fill::Rectangular : type == 6 ? Fill::PathGrad : Fill::Linear;
    if (!fromCenter) {
        // The first color runs toward 270 - a degrees for a stored angle a of
        // 0 or more, and toward 90 - a for a negative one.
        const double a = p.fixed(kFillAngle, 0);
        f.angle = std::fmod(std::fmod((a >= 0 ? 270 - a : 90 - a), 360.0) + 360.0, 360.0);
    }
    return f;
}

Arrow arrowOf(quint32 end)
{
    switch (end) {
    case 1: return Arrow::Triangle;
    case 2: return Arrow::Stealth;
    case 3: return Arrow::Diamond;
    case 4: return Arrow::Oval;
    case 5: case 6: case 7: return Arrow::Open;
    default: return Arrow::None;
    }
}

} // namespace

QColor colorProperty(const Props &props, const OdrawContext &ctx, quint16 id) { return colorFor(props, ctx, id, 0); }

double pictureBrightness(const Props &p) { return p.has(0x0109) ? std::clamp(p.integer(0x0109, 0) / 327.68, -100.0, 100.0) : 0; }

double pictureContrast(const Props &p) { return p.has(0x0108) ? std::clamp((p.fixed(0x0108, 1.0) - 1.0) * 100, -100.0, 100.0) : 0; }

Fill readFill(const Props &p, const OdrawContext &ctx, bool filledByDefault, const QRectF &shape)
{
    if (!p.flag(kFillStyleFlags, 0x10, filledByDefault)) return Fill();
    const int type = int(p.value(kFillType, 0));
    const QColor fillColor = colorProperty(p, ctx, kFillColor), backColor = colorProperty(p, ctx, kFillBackColor);
    const double opacity = opacityOf(p, kFillOpacity), backOpacity = opacityOf(p, kFillBackOpacity);
    switch (type) {
    case 1: case 2: case 3: {
        // A pattern, a texture, or a picture: the picture the fill's blip names.
        const int index = int(p.value(kFillBlip, 0));
        QString image = index > 0 && ctx.picture ? ctx.picture(index) : QString();
        if (image.isEmpty()) break;
        // A pattern is a bitmap of two tones that the shape's colors are put in.
        bool recolored = false;
        if (type == 1 && ctx.patterned) {
            QColor front = fillColor, back = backColor;
            front.setAlphaF(opacity);
            back.setAlphaF(backOpacity);
            const QString colored = ctx.patterned(image, front, back);
            if (!colored.isEmpty() && colored != image) {
                image = colored;
                recolored = true;
            }
        }
        // A picture set to be recolored as a picture, and that has the color to recolor it in (0x011A), is of two colors: that and the back color's.
        if (!recolored && type != 1 && ctx.duotoned && p.flag(kFillStyleFlags, 0x40, false) && p.has(0x011A)) {
            const QString two = ctx.duotoned(image, colorProperty(p, ctx, 0x011A), backColor);
            if (!two.isEmpty()) image = two;
        }
        if (ctx.adjusted && (pictureBrightness(p) != 0 || pictureContrast(p) != 0)) {
            const QString changed = ctx.adjusted(image, pictureBrightness(p), pictureContrast(p));
            if (!changed.isEmpty()) image = changed;
        }
        Fill f;
        f.type = type == 3 ? Fill::Picture : Fill::Texture;
        f.color = ColorRef::rgb(fillColor);
        f.imageId = image;
        f.tile = type != 3;
        f.transparency = recolored ? 0 : 1 - opacity;
        return f;
    }
    case 4: case 5: case 6: case 7: case 8:
        return gradientFill(p, ctx, type, fillColor, backColor, opacity, backOpacity);
    case 9:
        return ctx.background ? ctx.background(shape) : Fill();   // the page's own background shows through
    default:
        break;
    }
    return Fill::solid(ColorRef::rgb(fillColor), 1 - opacity);
}

Stroke readStroke(const Props &p, const OdrawContext &ctx, bool lineByDefault)
{
    Stroke s;
    if (!p.flag(kLineStyleFlags, 0x08, lineByDefault)) return s;
    s.color = ColorRef::rgb(colorProperty(p, ctx, kLineColor));
    const double width = p.integer(kLineWidth, 9525) / kEmuPerPoint;
    s.width = width > 0 ? std::max(width, 0.25) : 0.75;
    s.transparency = 1 - opacityOf(p, kLineOpacity);
    static const Stroke::Dash dashes[] = {Stroke::SolidLine, Stroke::DashLine, Stroke::SquareDot, Stroke::DashDot, Stroke::LongDashDotDot, Stroke::RoundDot,
                                          Stroke::DashLine, Stroke::LongDash, Stroke::DashDot, Stroke::LongDashDot, Stroke::LongDashDotDot};
    const quint32 dashing = p.value(kLineDashing, 0);
    if (dashing < sizeof(dashes) / sizeof(dashes[0])) s.dash = dashes[dashing];
    const quint32 compound = p.value(kLineStyle, 0);
    if (compound <= 4) s.compound = Stroke::Compound(compound);
    switch (p.value(kLineJoin, 1)) {
    case 0: s.join = Qt::BevelJoin; break;
    case 2: s.join = Qt::MiterJoin; break;
    default: s.join = Qt::RoundJoin; break;
    }
    switch (p.value(kLineCap, 2)) {
    case 0: s.cap = Qt::RoundCap; break;
    case 1: s.cap = Qt::SquareCap; break;
    default: s.cap = Qt::FlatCap; break;
    }
    // An arrowhead's width and length (0 narrow or short, 1 medium, 2 wide or long) become one size.
    auto size = [&](quint16 width, quint16 length) { return int(std::min<quint32>(2, (p.value(width, 1) + p.value(length, 1) + 1) / 2)); };
    s.startArrow = arrowOf(p.value(kLineStartArrow, 0));
    s.endArrow = arrowOf(p.value(kLineEndArrow, 0));
    s.startSize = size(kLineStartArrowWidth, kLineStartArrowLength);
    s.endSize = size(kLineEndArrowWidth, kLineEndArrowLength);
    return s;
}

void readShadow(const Props &p, const OdrawContext &ctx, Effects &fx)
{
    if (!p.flag(kShadowFlags, 0x02, false)) return;
    ShadowFx &s = fx.shadow;
    s.on = true;
    s.inner = false;
    s.blur = 0;
    s.color = ColorRef::rgb(colorProperty(p, ctx, kShadowColor));
    s.transparency = 1 - opacityOf(p, kShadowOpacity);
    const double dx = p.integer(kShadowOffsetX, 25400) / kEmuPerPoint, dy = p.integer(kShadowOffsetY, 25400) / kEmuPerPoint;
    s.distance = std::hypot(dx, dy);
    if (s.distance > 0) s.angle = qRadiansToDegrees(std::atan2(dy, dx));
}

namespace {

// A string of an IHlink: its length in characters (the end mark included), then them.
QString hyperlinkString(const QByteArray &d, qint64 *pos)
{
    const quint32 chars = u32(d, *pos);
    *pos += 4;
    if (qint64(chars) * 2 > qint64(d.size()) - *pos) {
        *pos = d.size() + 1;
        return QString();
    }
    const QString s = utf16Text(d.mid(qsizetype(*pos), qsizetype(qint64(chars) * 2)));
    *pos += qint64(chars) * 2;
    return s;
}

bool isGuid(const QByteArray &d, qint64 pos, quint32 a, quint16 b, quint16 c, quint8 e0)
{
    return u32(d, pos) == a && u16(d, pos + 4) == b && u16(d, pos + 6) == c && u8(d, pos + 8) == e0;
}

} // namespace

QString hyperlinkOf(const QByteArray &d)
{
    if (d.size() < 8) return QString();
    const quint32 flags = u32(d, 4);
    qint64 pos = 8;
    if (flags & 0x10) hyperlinkString(d, &pos);   // the text shown
    if (flags & 0x80) hyperlinkString(d, &pos);   // the frame to open it in
    QString target;
    if (flags & 0x01) {
        if (flags & 0x100) {
            target = hyperlinkString(d, &pos);
        } else if (pos + 16 <= d.size()) {
            if (isGuid(d, pos, 0x79EAC9E0, 0xBAF9, 0x11CE, 0x8C)) {
                // A URL: its length in bytes, then the address in UTF-16.
                const quint32 bytes = u32(d, pos + 16);
                pos += 20;
                if (qint64(bytes) <= qint64(d.size()) - pos) target = utf16Text(d.mid(qsizetype(pos), qsizetype(bytes)));
                pos += bytes;
            } else if (isGuid(d, pos, 0x00000303, 0, 0, 0xC0)) {
                // A file: the up-level count, the path in the ANSI code page, a
                // fixed part, then the path in UTF-16 when there is one.
                pos += 16 + 2;
                const quint32 ansi = u32(d, pos);
                pos += 4;
                if (qint64(ansi) > qint64(d.size()) - pos) return QString();
                target = QString::fromLatin1(d.mid(qsizetype(pos), qsizetype(ansi)));
                if (const qsizetype nul = target.indexOf(QChar(0)); nul >= 0) target.truncate(nul);
                pos += qint64(ansi) + 2 + 2 + 20;
                const quint32 unicode = u32(d, pos);
                pos += 4;
                if (unicode > 0 && qint64(unicode) <= qint64(d.size()) - pos) {
                    const QString wide = utf16Text(d, pos + 6);   // past its length and a key
                    if (!wide.isEmpty()) target = wide;
                }
                pos += unicode;
            } else {
                return QString();
            }
        }
    }
    const QString place = (flags & 0x08) && pos <= d.size() ? hyperlinkString(d, &pos) : QString();
    return place.isEmpty() ? target : target + QLatin1Char('#') + place;
}

} // namespace jp::office::odraw
