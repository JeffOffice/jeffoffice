#include "office/drawingml_color.h"

#include "office/xml.h"

#include <algorithm>
#include <cmath>

namespace jp::office {

namespace {

// The color while it is worked on: sRGB channels and alpha, each 0 to 1, in
// floating point so that a chain of transforms doesn't round at every step.
struct Rgba {
    double r = 0, g = 0, b = 0, a = 1;
};

double clamp01(double v)
{
    return v < 0 ? 0 : v > 1 ? 1 : v;
}

Rgba fromQColor(const QColor &c)
{
    return {c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0, c.alpha() / 255.0};
}

double toLinear(double c)
{
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

double fromLinear(double c)
{
    return c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow(c, 1 / 2.4) - 0.055;
}

// Hue in degrees (0 to 360), saturation and lightness 0 to 1.
void toHsl(const Rgba &c, double *h, double *s, double *l)
{
    const double hi = std::max({c.r, c.g, c.b}), lo = std::min({c.r, c.g, c.b});
    const double d = hi - lo;
    *l = (hi + lo) / 2;
    const double span = 1 - std::fabs(2 * *l - 1);
    *s = d == 0 || span < 1e-12 ? 0 : d / span;
    if (d == 0) *h = 0;
    else if (hi == c.r) *h = 60 * std::fmod((c.g - c.b) / d, 6);
    else if (hi == c.g) *h = 60 * ((c.b - c.r) / d + 2);
    else *h = 60 * ((c.r - c.g) / d + 4);
    if (*h < 0) *h += 360;
}

Rgba fromHsl(double h, double s, double l, double a)
{
    s = clamp01(s);
    l = clamp01(l);
    h = std::fmod(h, 360);
    if (h < 0) h += 360;
    const double chroma = (1 - std::fabs(2 * l - 1)) * s;
    const double sector = h / 60;
    const double x = chroma * (1 - std::fabs(std::fmod(sector, 2) - 1));
    double r = 0, g = 0, b = 0;
    switch (int(sector)) {
    case 0: r = chroma; g = x; break;
    case 1: r = x; g = chroma; break;
    case 2: g = chroma; b = x; break;
    case 3: g = x; b = chroma; break;
    case 4: r = x; b = chroma; break;
    default: r = chroma; b = x; break;
    }
    const double m = l - chroma / 2;
    return {r + m, g + m, b + m, a};
}

// Six hex digits, as a:srgbClr and lastClr write a color.
bool hexRgb(const QString &s, Rgba *out)
{
    if (s.size() != 6) return false;
    // Not toUInt's idea of a number, which lets a "0x", a sign, and spaces in.
    for (const QChar c : s)
        if (!((c >= QLatin1Char('0') && c <= QLatin1Char('9')) || (c >= QLatin1Char('a') && c <= QLatin1Char('f'))
              || (c >= QLatin1Char('A') && c <= QLatin1Char('F'))))
            return false;
    const uint v = s.toUInt(nullptr, 16);
    *out = {((v >> 16) & 0xFF) / 255.0, ((v >> 8) & 0xFF) / 255.0, (v & 0xFF) / 255.0, 1};
    return true;
}

// The names of a:prstClr are the CSS color names, with "dk", "lt", and "med"
// for "dark", "light", and "medium".
bool presetColor(const QString &name, Rgba *out)
{
    if (name.isEmpty()) return false;
    for (const QChar c : name)
        if (!c.isLetter() || c.unicode() > 127) return false;
    QString css = name.toLower();
    if (name.startsWith(QLatin1String("dk"))) css = QStringLiteral("dark") + css.mid(2);
    else if (name.startsWith(QLatin1String("lt"))) css = QStringLiteral("light") + css.mid(2);
    else if (name.startsWith(QLatin1String("med"))) css = QStringLiteral("medium") + css.mid(3);
    const QColor c = QColor::fromString(css);
    // "transparent" is a name to Qt but not to the schema.
    if (!c.isValid() || c.alpha() != 255) return false;
    *out = fromQColor(c);
    return true;
}

// The color before any transform; false if the element is no color, or one
// that can't be told.
bool baseColor(const QDomElement &e, const QString &name, const Theme &theme, const ColorMap &map, const QColor &phClr, Rgba *out)
{
    if (name == QLatin1String("srgbClr")) return hexRgb(attr(e, QStringLiteral("val")), out);
    if (name == QLatin1String("prstClr")) return presetColor(attr(e, QStringLiteral("val")), out);
    if (name == QLatin1String("schemeClr")) {
        const QString val = attr(e, QStringLiteral("val"));
        if (val == QLatin1String("phClr")) {
            if (!phClr.isValid()) return false;
            *out = fromQColor(phClr);
            return true;
        }
        Theme::Color slot;
        if (!map.resolve(val, &slot) || !theme.colors[slot].isValid()) return false;
        *out = fromQColor(theme.colors[slot]);
        return true;
    }
    if (name == QLatin1String("sysClr")) {
        // The value it had when the file was saved, else what the name usually is.
        if (hexRgb(attr(e, QStringLiteral("lastClr")), out)) return true;
        const QString val = attr(e, QStringLiteral("val"));
        if (val == QLatin1String("window")) *out = {1, 1, 1, 1};
        else if (val == QLatin1String("windowText")) *out = {0, 0, 0, 1};
        else return false;
        return true;
    }
    if (name == QLatin1String("hslClr")) {
        bool hueOk = false, satOk = false, lumOk = false;
        const double h = parseAngle(attr(e, QStringLiteral("hue")), 0, &hueOk);
        const double s = parsePercent(attr(e, QStringLiteral("sat")), 0, &satOk);
        const double l = parsePercent(attr(e, QStringLiteral("lum")), 0, &lumOk);
        if (!hueOk || !satOk || !lumOk) return false;
        *out = fromHsl(h, s, l, 1);
        return true;
    }
    if (name == QLatin1String("scrgbClr")) {
        // Percentages of linear light.
        bool rOk = false, gOk = false, bOk = false;
        const double r = parsePercent(attr(e, QStringLiteral("r")), 0, &rOk);
        const double g = parsePercent(attr(e, QStringLiteral("g")), 0, &gOk);
        const double b = parsePercent(attr(e, QStringLiteral("b")), 0, &bOk);
        if (!rOk || !gOk || !bOk) return false;
        *out = {fromLinear(clamp01(r)), fromLinear(clamp01(g)), fromLinear(clamp01(b)), 1};
        return true;
    }
    return false;
}

// A transform sets a value, scales it ("Mod"), or moves it ("Off").
enum class Op { Set, Mod, Off };

double changed(double x, Op op, double v)
{
    return op == Op::Set ? v : op == Op::Mod ? x * v : x + v;
}

void applyTransform(Rgba *c, const QDomElement &t)
{
    const QString name = t.localName();
    if (name == QLatin1String("comp")) {
        double h, s, l;
        toHsl(*c, &h, &s, &l);
        *c = fromHsl(h + 180, s, l, c->a);
        return;
    }
    if (name == QLatin1String("inv")) {
        *c = {1 - c->r, 1 - c->g, 1 - c->b, c->a};
        return;
    }
    if (name == QLatin1String("gray")) {
        const double y = 0.299 * c->r + 0.587 * c->g + 0.114 * c->b;
        *c = {y, y, y, c->a};
        return;
    }

    Op op = Op::Set;
    QString target = name;
    if (name.endsWith(QLatin1String("Mod"))) {
        op = Op::Mod;
        target.chop(3);
    } else if (name.endsWith(QLatin1String("Off"))) {
        op = Op::Off;
        target.chop(3);
    }
    // A value is a percentage, except a hue's, which is an angle.
    bool ok = false;
    const QString val = attr(t, QStringLiteral("val"));
    const double v = target == QLatin1String("hue") && op != Op::Mod ? parseAngle(val, 0, &ok) : parsePercent(val, 0, &ok);
    if (!ok) return;

    if (name == QLatin1String("shade") || name == QLatin1String("tint")) {
        // In linear light: shade scales each channel, tint moves it toward 1.
        const double amount = clamp01(v);
        auto channel = [&](double x) {
            const double lin = toLinear(clamp01(x));
            return fromLinear(clamp01(name == QLatin1String("shade") ? lin * amount : lin * amount + (1 - amount)));
        };
        *c = {channel(c->r), channel(c->g), channel(c->b), c->a};
    } else if (target == QLatin1String("hue") || target == QLatin1String("sat") || target == QLatin1String("lum")) {
        double h, s, l;
        toHsl(*c, &h, &s, &l);
        if (target == QLatin1String("hue")) h = changed(h, op, v);
        else if (target == QLatin1String("sat")) s = changed(s, op, v);
        else l = changed(l, op, v);
        *c = fromHsl(h, s, l, c->a);
    } else if (target == QLatin1String("red")) {
        c->r = clamp01(changed(c->r, op, v));
    } else if (target == QLatin1String("green")) {
        c->g = clamp01(changed(c->g, op, v));
    } else if (target == QLatin1String("blue")) {
        c->b = clamp01(changed(c->b, op, v));
    } else if (target == QLatin1String("alpha")) {
        c->a = clamp01(changed(c->a, op, v));
    }
}

int to8(double v)
{
    return int(std::lround(clamp01(v) * 255));
}

} // namespace

bool ColorMap::resolve(const QString &name, Theme::Color *slot) const
{
    if (name == QLatin1String("bg1")) *slot = bg1;
    else if (name == QLatin1String("tx1")) *slot = tx1;
    else if (name == QLatin1String("bg2")) *slot = bg2;
    else if (name == QLatin1String("tx2")) *slot = tx2;
    else if (name == QLatin1String("hlink")) *slot = hlink;
    else if (name == QLatin1String("folHlink")) *slot = folHlink;
    else if (name.size() == 7 && name.startsWith(QLatin1String("accent")) && name[6] >= QLatin1Char('1') && name[6] <= QLatin1Char('6'))
        *slot = accent[name[6].unicode() - '1'];
    else return themeColorFromName(name, slot);   // dk1, lt1, dk2, lt2
    return true;
}

ColorMap readColorMap(const QDomElement &e)
{
    ColorMap m;
    auto read = [&](const char *attribute, Theme::Color *slot) {
        Theme::Color found;
        if (themeColorFromName(attr(e, QString::fromLatin1(attribute)), &found)) *slot = found;
    };
    read("bg1", &m.bg1);
    read("tx1", &m.tx1);
    read("bg2", &m.bg2);
    read("tx2", &m.tx2);
    for (int i = 0; i < 6; ++i) read(qPrintable(QStringLiteral("accent%1").arg(i + 1)), &m.accent[i]);
    read("hlink", &m.hlink);
    read("folHlink", &m.folHlink);
    return m;
}

QColor readColor(const QDomElement &choiceParent, const Theme &theme, const ColorMap &map, const QColor &phClr)
{
    static const QStringList choices = {QStringLiteral("srgbClr"), QStringLiteral("schemeClr"), QStringLiteral("sysClr"),
                                        QStringLiteral("prstClr"), QStringLiteral("hslClr"), QStringLiteral("scrgbClr")};
    for (QDomElement e = choiceParent.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (!sameNamespace(e.namespaceURI(), ns::A) || !choices.contains(e.localName())) continue;
        // The first color there is the color; if it can't be told, there is none.
        Rgba c;
        if (!baseColor(e, e.localName(), theme, map, phClr, &c)) return QColor();
        for (QDomElement t = e.firstChildElement(); !t.isNull(); t = t.nextSiblingElement())
            if (sameNamespace(t.namespaceURI(), ns::A)) applyTransform(&c, t);
        return QColor(to8(c.r), to8(c.g), to8(c.b), to8(c.a));
    }
    return QColor();
}

} // namespace jp::office
