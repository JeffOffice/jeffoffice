#include "app/formatops.h"

#include "app/editor.h"
#include "render/shapes.h"
#include "text/textprops.h"

#include <QJsonDocument>

#include <QCoreApplication>

namespace jp::formatops {

namespace {

QString say(const char *text) { return QCoreApplication::translate("FormatPane", text); }

struct GradientPreset {
    const char *name;
    Fill::GradType type;
    double angle;
};

const GradientPreset kGradients[] = {
    {QT_TRANSLATE_NOOP("FormatPane", "Linear Down"), Fill::Linear, 90},       {QT_TRANSLATE_NOOP("FormatPane", "Linear Up"), Fill::Linear, 270},
    {QT_TRANSLATE_NOOP("FormatPane", "Linear Right"), Fill::Linear, 0},       {QT_TRANSLATE_NOOP("FormatPane", "Linear Left"), Fill::Linear, 180},
    {QT_TRANSLATE_NOOP("FormatPane", "Linear Diagonal"), Fill::Linear, 45},   {QT_TRANSLATE_NOOP("FormatPane", "From Center"), Fill::Radial, 0},
};

const char *const kArrowStyles[] = {QT_TRANSLATE_NOOP("FormatPane", "No Arrow"), QT_TRANSLATE_NOOP("FormatPane", "Arrow"), QT_TRANSLATE_NOOP("FormatPane", "Double Arrow"),
                                    QT_TRANSLATE_NOOP("FormatPane", "Open Arrow"), QT_TRANSLATE_NOOP("FormatPane", "Dot")};

// The group around an object is as big as what is in it.
void refit(Editor *ed, Item &it)
{
    for (GroupItem *g = ed->doc()->find(it.id).parent; g; g = ed->doc()->find(g->id).parent) g->syncRect();
}

} // namespace

bool takesFill(const Item &it)
{
    switch (it.type()) {
    case ItemType::Text:
    case ItemType::Picture:
    case ItemType::Shape:
    case ItemType::TextArt: return true;
    default: return false;
    }
}

bool takesLine(const Item &it)
{
    switch (it.type()) {
    case ItemType::Text:
    case ItemType::Picture:
    case ItemType::TextArt:
    case ItemType::Shape:
    case ItemType::Line: return true;
    default: return false;
    }
}

void setSolid(Fill &f, const ColorRef &color)
{
    if (color.isNone()) {
        f = Fill();
        return;
    }
    const double t = f.type == Fill::Solid || f.type == Fill::Pattern || f.type == Fill::Picture || f.type == Fill::Texture ? f.transparency : 0;
    f = Fill::solid(color, t);
}

int gradientCount() { return int(std::size(kGradients)); }

QString gradientName(int i) { return say(kGradients[std::clamp(i, 0, gradientCount() - 1)].name); }

void applyGradient(Fill &f, int i, const ColorScheme &scheme)
{
    const GradientPreset &p = kGradients[std::clamp(i, 0, gradientCount() - 1)];
    ColorRef base = f.type == Fill::Gradient && !f.stops.isEmpty() ? f.stops.first().color : f.color;
    if (base.isNone()) base = ColorRef::scheme(Accent1);
    const ColorRef lighter = ColorRef::rgb(mix(base.resolve(scheme), Qt::white, 0.6));
    const double t = f.type == Fill::Gradient ? 0 : f.transparency;
    Fill g;
    g.type = Fill::Gradient;
    g.gradType = p.type;
    g.angle = p.angle;
    g.stops = {{0, base, t}, {1, lighter, t}};
    g.color = base;
    g.color2 = lighter;
    f = g;
}

void setOutlineColor(Stroke &s, const ColorRef &color)
{
    if (color.isNone()) {
        setOutlineNone(s);
        return;
    }
    s.gradient = Fill();
    s.color = color;
    if (s.width <= 0) s.width = 0.75;
}

void setOutlineNone(Stroke &s)
{
    s.color = ColorRef();
    s.gradient = Fill();
}

int arrowStyleCount() { return int(std::size(kArrowStyles)); }

QString arrowStyleName(int i) { return say(kArrowStyles[std::clamp(i, 0, arrowStyleCount() - 1)]); }

void applyArrowStyle(Stroke &s, int i)
{
    s.startArrow = Arrow::None;
    s.endArrow = i == 1 || i == 2 ? Arrow::Triangle : i == 3 ? Arrow::Open : i == 4 ? Arrow::Oval : Arrow::None;
    if (i == 2) s.startArrow = Arrow::Triangle;
}

void setTextFill(QTextCharFormat &cf, const ColorRef &color)
{
    cf.setProperty(tp::ColorRefP, color.toString());
    cf.setProperty(tp::TextFill, QString());   // a solid color replaces a gradient fill
    cf.clearForeground();
}

void setTextFillNone(QTextCharFormat &cf)
{
    setTextFill(cf, ColorRef::rgb(QColor(0, 0, 0, 0)));
    cf.setForeground(QBrush(QColor(0, 0, 0, 0)));
}

void setTextFillGradient(QTextCharFormat &cf, const Fill &gradient, const ColorScheme &scheme)
{
    Q_UNUSED(scheme);
    // (Its colors as stops, as a file has them: a gradient of two colors says them in color and color2.)
    Fill g = gradient;
    if (g.stops.isEmpty()) g.stops = {{0, g.color, 0}, {1, g.color2, 0}};
    cf.setProperty(tp::TextFill, QString::fromUtf8(QJsonDocument(g.toJson()).toJson(QJsonDocument::Compact)));
    cf.setProperty(tp::ColorRefP, g.stops.first().color.toString());
    cf.clearForeground();
}

void setTextFillGradient(QTextCharFormat &cf, int preset, const ColorScheme &scheme)
{
    Fill f = Fill::solid(textColorOf(cf, ColorRef::scheme(Main)));
    if (f.color.isNone() || f.color.resolve(scheme).alpha() == 0) f.color = ColorRef::scheme(Main);
    applyGradient(f, preset, scheme);
    setTextFillGradient(cf, f, scheme);
}

ColorRef textColorOf(const QTextCharFormat &cf, const ColorRef &fallback)
{
    if (const QString ref = cf.stringProperty(tp::ColorRefP); !ref.isEmpty()) return ColorRef::fromString(ref);
    if (cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() != Qt::NoBrush) return ColorRef::rgb(cf.foreground().color());
    return fallback;
}

void setTextOutline(QTextCharFormat &cf, const ColorRef &color)
{
    cf.setProperty(tp::OutlineRef, color.toString());
    if (!cf.hasProperty(tp::OutlineWidth)) cf.setProperty(tp::OutlineWidth, 1.0);
}

void setTextOutlineWeight(QTextCharFormat &cf, double points)
{
    if (cf.stringProperty(tp::OutlineRef).isEmpty()) cf.setProperty(tp::OutlineRef, ColorRef::scheme(Main).toString());
    cf.setProperty(tp::OutlineWidth, points);
}

void setTextOutlineNone(QTextCharFormat &cf)
{
    cf.clearProperty(tp::OutlineRef);
    cf.clearProperty(tp::OutlineWidth);
}

void changeTextEffects(QTextCharFormat &cf, const std::function<void(TextEffects &)> &change)
{
    const QString list = cf.stringProperty(tp::TextFx);
    TextEffects fx = list.isEmpty() ? TextEffects() : TextEffects::fromJson(list);
    change(fx);
    fx.xml.clear();
    cf.setProperty(tp::TextFx, fx.any() ? fx.toJson() : TextEffects::noneJson());
    cf.clearProperty(tp::Shadow);
}

void setTextLook(QTextCharFormat &cf, const Fill &fill, const Stroke &stroke, const ShadowFx &shadow, const ColorScheme &scheme)
{
    if (fill.type == Fill::Gradient) setTextFillGradient(cf, fill, scheme);
    else if (fill.isNone()) setTextFillNone(cf);
    else setTextFill(cf, fill.color);
    if (stroke.isNone()) setTextOutlineNone(cf);
    else {
        cf.setProperty(tp::OutlineRef, stroke.color.toString());
        cf.setProperty(tp::OutlineWidth, stroke.width);
    }
    changeTextEffects(cf, [&](TextEffects &fx) {
        if (!shadow.on) {
            fx.outer.on = false;
            return;
        }
        TextShadow s;
        s.on = true;
        s.color = shadow.color.resolve(scheme);
        s.color.setAlphaF(float(s.color.alphaF() * (1 - shadow.transparency)));
        s.blur = shadow.blur;
        s.distance = shadow.distance;
        s.angle = shadow.angle;
        fx.outer = s;
    });
}

bool takesGeometry(const Item &it) { return it.type() == ItemType::Shape && !static_cast<const ShapeItem &>(it).isArt(); }

bool setGeometry(Item &it, const QString &shapeId)
{
    if (!takesGeometry(it) || !shapeDef(shapeId)) return false;
    auto &s = static_cast<ShapeItem &>(it);
    s.shape = shapeId;
    s.adj.clear();
    s.customPath = QPainterPath();
    return true;
}

bool keepsProportions(const Item &it) { return it.type() == ItemType::Table ? static_cast<const TableItem &>(it).lockSize : it.lockAspect; }

void resizeFrame(Editor *ed, Item &it, QSizeF size)
{
    size = QSizeF(std::clamp(size.width(), 0.0, 4000.0), std::clamp(size.height(), 0.0, 4000.0));
    const double rotation = it.rotation;
    it.rotation = 0;
    it.scaleInto(it.rect, QRectF(it.rect.topLeft(), size));
    it.rotation = rotation;
    refit(ed, it);
}

void moveFrame(Editor *ed, Item &it, double dx, double dy)
{
    it.moveBy(dx, dy);
    refit(ed, it);
}

} // namespace jp::formatops
