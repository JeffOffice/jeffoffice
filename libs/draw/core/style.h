#pragma once
// Fill, outline, effects and text-wrap settings shared by all frame types.

#include "core/color.h"
#include "core/fill.h"

#include <QBrush>
#include <QJsonObject>
#include <QPen>
#include <QPolygonF>
#include <QRectF>
#include <QVector>
#include <functional>

class QImage;

namespace jp {

enum class Arrow { None, Triangle, Open, Stealth, Diamond, Oval };

struct Stroke {
    enum Dash { SolidLine, RoundDot, SquareDot, DashLine, DashDot, LongDash, LongDashDot, LongDashDotDot };
    enum Compound { Single, Double, ThickThin, ThinThick, Triple };

    ColorRef color;
    double width = 0.75;           // points
    Dash dash = SolidLine;
    Compound compound = Single;
    double transparency = 0;
    Qt::PenJoinStyle join = Qt::MiterJoin;
    Qt::PenCapStyle cap = Qt::FlatCap;
    bool inset = false;            // a closed outline drawn inside the edge (.pub text box borders)
    Arrow startArrow = Arrow::None, endArrow = Arrow::None;
    int startSize = 1, endSize = 1; // 0 small, 1 medium, 2 large
    // A line drawn in a gradient: while its type is Gradient the line takes its colors from it, along the line's own box, and color holds the
    // first stop's (what an arrowhead is filled with, and what a file that can't keep a gradient line keeps). Else the line is its color.
    Fill gradient;

    static Stroke none() { return Stroke(); }
    static Stroke line(const ColorRef &c, double w = 0.75) { Stroke s; s.color = c; s.width = w; return s; }
    bool isNone() const { return color.isNone() || width <= 0; }
    bool hasGradient() const { return gradient.type == Fill::Gradient && !isNone(); }
    // How far in from the edge a box's text margin is measured with this line
    // around it: Publisher sets the text inside the line, from its inner edge,
    // half the weight for a line centered on the edge, all of it for an inset one.
    double textShift() const { return isNone() ? 0 : inset ? width : width / 2; }

    QPen pen(const ColorScheme &s) const;
    QJsonObject toJson() const;
    static Stroke fromJson(const QJsonObject &o);
    bool operator==(const Stroke &o) const { return toJson() == o.toJson(); }
};

struct ShadowFx {
    bool on = false;
    ColorRef color = ColorRef::rgb(Qt::black);
    double transparency = 0.6;
    double blur = 4;       // points
    double distance = 3;   // points
    double angle = 45;     // degrees; 45 = down-right
    double size = 100;     // percent of the object's size, about its middle (before the shadow is moved off by distance)
    bool inner = false;
    QPointF offset() const;
};
struct GlowFx { bool on = false; ColorRef color = ColorRef::scheme(Accent1, 40); double size = 6; double transparency = 0.4; };
struct ReflectionFx { bool on = false; double transparency = 0.5; double size = 0.5; double distance = 0; double blur = 0.5; };
struct BevelFx { int type = 0; double width = 6; double height = 6; };       // 0 none, 1 circle, 2 relaxed, 3 cool slant, 4 angle, 5 soft round, 6 convex, 7 slope, 8 divot, 9 riblet, 10 hard edge, 11 art deco, 12 cross
struct Rotation3D { double x = 0, y = 0, perspective = 0; };

struct Effects {
    ShadowFx shadow;
    GlowFx glow;
    double softEdge = 0;   // points
    ReflectionFx reflection;
    BevelFx bevel;
    Rotation3D rot3d;
    bool any() const { return shadow.on || glow.on || softEdge > 0 || reflection.on || bevel.type || rot3d.x || rot3d.y; }
    QJsonObject toJson() const;
    static Effects fromJson(const QJsonObject &o);
};

// A shape's reference to the presentation's format styles (what a file calls its style, p:style): which line, fill, and effect style of the
// theme it takes (1 to 3, 0 for none), the theme color each is made in (by its name in the theme, "accent1" or "lt1", with the changes made
// to it, as `shade` 50000), and the font and color its text has. The shape's fill, stroke, and fx are the look these give, or what the
// shape has had changed since: a file keeps the reference, and the properties where they differ from it.
// Whether a name is one a style's color may have: a theme color by the standard's name ("accent1", "lt1", "tx1"), or "#" and six hex digits; and
// whether a name is a change of a color the standard has (shade, lumMod, alpha...), which takes a number.
bool isStyleColorName(const QString &name);
bool isStyleColorChange(const QString &name);

struct StyleColor {
    QString name;                                   // empty for none
    QVector<QPair<QString, int>> mods;              // changes in the order they are made: an element's name and its value (shade, 50000)
    bool operator==(const StyleColor &o) const { return name == o.name && mods == o.mods; }
};
struct StyleRef {
    struct Part {
        int idx = 0;
        StyleColor color;
        bool operator==(const Part &o) const { return idx == o.idx && color == o.color; }
    };
    bool on = false;
    Part line, fill, effect;
    QString font;                                   // "minor", "major", or empty for none
    StyleColor fontColor;
    QJsonObject toJson() const;
    static StyleRef fromJson(const QJsonObject &o);
    bool operator==(const StyleRef &o) const { return on == o.on && line == o.line && fill == o.fill && effect == o.effect && font == o.font && fontColor == o.fontColor; }
    bool operator!=(const StyleRef &o) const { return !(*this == o); }
};

struct Wrap {
    enum Mode { None, Square, Tight, Through, TopBottom };
    enum Side { Both, LeftOnly, RightOnly, Largest };
    Mode mode = Square;
    Side side = Both;
    double top = 2.88, bottom = 2.88, left = 2.88, right = 2.88; // 0.04 in
    QPolygonF points;     // custom wrap points (frame-local), used by Tight/Through when edited
    QJsonObject toJson() const;
    static Wrap fromJson(const QJsonObject &o);
};

QVector<Qt::BrushStyle> patternBrushes();
QString dashName(Stroke::Dash d);

} // namespace jp
