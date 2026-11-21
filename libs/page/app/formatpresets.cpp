#include "app/formatpresets.h"

#include "app/formatops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <algorithm>
#include <iterator>

namespace jp::formatpresets {

namespace {

QString say(const char *text) { return QCoreApplication::translate("FormatPane", text); }

struct ShadowPreset {
    const char *name;
    double angle, distance, blur;
};

const ShadowPreset kShadows[] = {
    {QT_TRANSLATE_NOOP("FormatPane", "Offset Bottom Right"), 45, 3, 4},  {QT_TRANSLATE_NOOP("FormatPane", "Offset Bottom"), 90, 3, 4},
    {QT_TRANSLATE_NOOP("FormatPane", "Offset Bottom Left"), 135, 3, 4},  {QT_TRANSLATE_NOOP("FormatPane", "Offset Right"), 0, 3, 4},
    {QT_TRANSLATE_NOOP("FormatPane", "Offset Center"), 0, 0, 5},         {QT_TRANSLATE_NOOP("FormatPane", "Offset Left"), 180, 3, 4},
    {QT_TRANSLATE_NOOP("FormatPane", "Offset Top Right"), 315, 3, 4},    {QT_TRANSLATE_NOOP("FormatPane", "Offset Top"), 270, 3, 4},
    {QT_TRANSLATE_NOOP("FormatPane", "Offset Top Left"), 225, 3, 4},
};

struct ReflectionPreset {
    const char *name;
    double size;
};

const ReflectionPreset kReflections[] = {
    {QT_TRANSLATE_NOOP("FormatPane", "Tight Reflection"), 0.35},
    {QT_TRANSLATE_NOOP("FormatPane", "Half Reflection"), 0.55},
    {QT_TRANSLATE_NOOP("FormatPane", "Full Reflection"), 0.9},
};

const double kGlows[] = {5, 10, 18};
const double kSoftEdges[] = {1, 2.5, 5, 10, 25, 50};

struct BevelPreset {
    const char *name;
    int type;
    double size;
};

const BevelPreset kBevels[] = {
    {QT_TRANSLATE_NOOP("FormatPane", "Circle"), 1, 6},      {QT_TRANSLATE_NOOP("FormatPane", "Relaxed Inset"), 2, 6}, {QT_TRANSLATE_NOOP("FormatPane", "Cool Slant"), 3, 6},
    {QT_TRANSLATE_NOOP("FormatPane", "Angle"), 4, 6},       {QT_TRANSLATE_NOOP("FormatPane", "Soft Round"), 5, 6},    {QT_TRANSLATE_NOOP("FormatPane", "Convex"), 6, 6},
    {QT_TRANSLATE_NOOP("FormatPane", "Slope"), 7, 5},       {QT_TRANSLATE_NOOP("FormatPane", "Divot"), 8, 5},
};

struct RotationPreset {
    const char *name;
    double x, y, perspective;
};

const RotationPreset kRotations[] = {
    {QT_TRANSLATE_NOOP("FormatPane", "No Rotation"), 0, 0, 0},          {QT_TRANSLATE_NOOP("FormatPane", "Perspective Left"), 0, 30, 30},
    {QT_TRANSLATE_NOOP("FormatPane", "Perspective Right"), 0, -30, 30}, {QT_TRANSLATE_NOOP("FormatPane", "Perspective Above"), 30, 0, 30},
    {QT_TRANSLATE_NOOP("FormatPane", "Perspective Below"), -30, 0, 30}, {QT_TRANSLATE_NOOP("FormatPane", "Off Axis"), 15, 25, 30},
};

} // namespace

int bevelCount() { return int(std::size(kBevels)); }

QString bevelName(int i) { return say(kBevels[std::clamp(i, 0, bevelCount() - 1)].name); }

void applyBevel(Effects &fx, int i)
{
    const BevelPreset &p = kBevels[std::clamp(i, 0, bevelCount() - 1)];
    fx.bevel.type = p.type;
    fx.bevel.width = fx.bevel.height = p.size;
}

int rotationCount() { return int(std::size(kRotations)); }

QString rotationName(int i) { return say(kRotations[std::clamp(i, 0, rotationCount() - 1)].name); }

void applyRotation(Effects &fx, int i)
{
    const RotationPreset &p = kRotations[std::clamp(i, 0, rotationCount() - 1)];
    fx.rot3d.x = p.x;
    fx.rot3d.y = p.y;
    fx.rot3d.perspective = p.perspective;
}

int shadowCount() { return int(std::size(kShadows)); }

QString shadowName(int i) { return say(kShadows[std::clamp(i, 0, shadowCount() - 1)].name); }

void applyShadow(Effects &fx, int i)
{
    const ShadowPreset &p = kShadows[std::clamp(i, 0, shadowCount() - 1)];
    ShadowFx s;
    s.on = true;
    s.color = ColorRef::rgb(Qt::black);
    s.transparency = 0.6;
    s.blur = p.blur;
    s.distance = p.distance;
    s.angle = p.angle;
    s.size = 100;
    s.inner = false;
    fx.shadow = s;
}

int reflectionCount() { return int(std::size(kReflections)); }

QString reflectionName(int i) { return say(kReflections[std::clamp(i, 0, reflectionCount() - 1)].name); }

void applyReflection(Effects &fx, int i)
{
    ReflectionFx r;
    r.on = true;
    r.transparency = 0.48;
    r.size = kReflections[std::clamp(i, 0, reflectionCount() - 1)].size;
    r.distance = 0;
    r.blur = 0.5;
    fx.reflection = r;
}

int glowCount() { return int(std::size(kGlows)); }

QString glowName(int i)
{
    return QCoreApplication::translate("FormatPane", "%1 Point Glow").arg(kGlows[std::clamp(i, 0, glowCount() - 1)]);
}

void applyGlow(Effects &fx, int i, const ColorRef &color)
{
    GlowFx g;
    g.on = true;
    g.color = color;
    g.size = kGlows[std::clamp(i, 0, glowCount() - 1)];
    g.transparency = 0.6;
    fx.glow = g;
}

void applyTextShadow(TextEffects &fx, int i)
{
    const ShadowPreset &p = kShadows[std::clamp(i, 0, shadowCount() - 1)];
    TextShadow s;
    s.on = true;
    s.color = QColor(0, 0, 0, 102);   // (black, the shape shadow's 60 percent clear)
    s.blur = p.blur;
    s.distance = p.distance;
    s.angle = p.angle;
    fx.outer = s;
    fx.xml.clear();
}

void applyTextReflection(TextEffects &fx, int i)
{
    TextReflection r;
    r.on = true;
    r.blur = 0.5;
    r.startAlpha = 0.52;   // (the shape reflection's 48 percent clear)
    r.endAlpha = 0.003;
    r.startPos = 0;
    r.endPos = kReflections[std::clamp(i, 0, reflectionCount() - 1)].size;
    r.distance = 0;
    r.sy = -1;   // turned over, about the foot of the letters
    fx.reflection = r;
    fx.xml.clear();
}

void applyTextGlow(TextEffects &fx, int i, const QColor &color)
{
    TextGlow g;
    g.on = true;
    g.color = color;
    g.color.setAlphaF(0.4f);   // (the shape glow's 60 percent clear)
    g.radius = kGlows[std::clamp(i, 0, glowCount() - 1)];
    fx.glow = g;
    fx.xml.clear();
}

int softEdgeCount() { return int(std::size(kSoftEdges)); }

QString softEdgeName(int i)
{
    return QCoreApplication::translate("FormatPane", "%1 Point").arg(kSoftEdges[std::clamp(i, 0, softEdgeCount() - 1)]);
}

void applySoftEdge(Effects &fx, int i) { fx.softEdge = kSoftEdges[std::clamp(i, 0, softEdgeCount() - 1)]; }

namespace {

// What a text style sets: the fill (a scheme slot, or -1 for clear letters), the outline (a slot, or -1 for none) and its weight, and which effect it adds.
enum class Look { Plain, Shadow, SoftShadow, Glow, Reflection, Emboss, Engrave };
struct TextStyleDef {
    const char *name;
    int fill, outline;
    double weight;
    Look look;
};
const TextStyleDef kTextStyles[] = {
    {QT_TRANSLATE_NOOP("TextStyle", "Plain"), Main, -1, 0, Look::Plain},
    {QT_TRANSLATE_NOOP("TextStyle", "Outline"), -1, Main, 1, Look::Plain},
    {QT_TRANSLATE_NOOP("TextStyle", "Bold Outline"), -1, Accent1, 2, Look::Plain},
    {QT_TRANSLATE_NOOP("TextStyle", "Filled Outline"), Accent1, Main, 1, Look::Plain},
    {QT_TRANSLATE_NOOP("TextStyle", "Shadow"), Main, -1, 0, Look::Shadow},
    {QT_TRANSLATE_NOOP("TextStyle", "Soft Shadow"), Accent1, -1, 0, Look::SoftShadow},
    {QT_TRANSLATE_NOOP("TextStyle", "Outlined Shadow"), Accent2, Main, 0.75, Look::Shadow},
    {QT_TRANSLATE_NOOP("TextStyle", "Glow"), Main, -1, 0, Look::Glow},
    {QT_TRANSLATE_NOOP("TextStyle", "Colored Glow"), Accent3, -1, 0, Look::Glow},
    {QT_TRANSLATE_NOOP("TextStyle", "Reflection"), Accent1, -1, 0, Look::Reflection},
    {QT_TRANSLATE_NOOP("TextStyle", "Emboss"), Main, -1, 0, Look::Emboss},
    {QT_TRANSLATE_NOOP("TextStyle", "Engrave"), Main, -1, 0, Look::Engrave},
};

} // namespace

int textStyleCount() { return int(std::size(kTextStyles)); }

QString textStyleName(int i)
{
    return QCoreApplication::translate("TextStyle", kTextStyles[std::clamp(i, 0, textStyleCount() - 1)].name);
}

void applyTextStyle(QTextCharFormat &cf, int i, const ColorScheme &scheme)
{
    const TextStyleDef &d = kTextStyles[std::clamp(i, 0, textStyleCount() - 1)];
    // What the three leave behind from another style goes first: the outline, the effects of the list, and the marks that draw letters raised or cut in.
    formatops::setTextOutlineNone(cf);
    cf.clearProperty(tp::GlowRef);
    cf.setProperty(tp::Emboss, d.look == Look::Emboss);
    cf.setProperty(tp::Engrave, d.look == Look::Engrave);
    if (d.fill < 0) formatops::setTextFillNone(cf);
    else formatops::setTextFill(cf, ColorRef::scheme(SchemeSlot(d.fill)));
    if (d.outline >= 0) {
        formatops::setTextOutline(cf, ColorRef::scheme(SchemeSlot(d.outline)));
        formatops::setTextOutlineWeight(cf, d.weight);
    }
    formatops::changeTextEffects(cf, [&](TextEffects &fx) {
        fx = TextEffects();
        if (d.look == Look::Shadow) applyTextShadow(fx, 0);
        else if (d.look == Look::SoftShadow) applyTextShadow(fx, 1);
        else if (d.look == Look::Glow) applyTextGlow(fx, 1, ColorRef::scheme(d.fill == Main ? Accent1 : SchemeSlot(d.fill)).resolve(scheme));
        else if (d.look == Look::Reflection) applyTextReflection(fx, 0);
    });
}

} // namespace jp::formatpresets
