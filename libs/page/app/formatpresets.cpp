#include "app/formatpresets.h"

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

} // namespace

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

} // namespace jp::formatpresets
