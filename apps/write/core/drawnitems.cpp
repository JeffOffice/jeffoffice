#include "core/drawnitems.h"

#include "core/shapetypes.h"

#include <QColor>
#include <algorithm>
#include <cmath>

using namespace jp::office::wml;

namespace jp::write {

namespace {

ColorRef colorOf(const QString &rgb) { return ColorRef::rgb(QColor(QLatin1Char('#') + rgb)); }

} // namespace

void dressPicture(PictureItem &pic, const Shape &s)
{
    const double w = pic.rect.width(), h = pic.rect.height();
    const double keepW = 1 - s.cropL - s.cropR, keepH = 1 - s.cropT - s.cropB;
    pic.imgRect = QRectF(0, 0, w, h);
    // (The reader leaves a cut that is not at least a fiftieth of the picture: the whole picture is at most fifty times the frame.)
    if ((s.cropL != 0 || s.cropT != 0 || s.cropR != 0 || s.cropB != 0) && keepW >= 0.02 && keepH >= 0.02) {
        const double wholeW = w / keepW, wholeH = h / keepH;
        pic.imgRect = QRectF(-s.cropL * wholeW, -s.cropT * wholeH, wholeW, wholeH);
    }
    if (!s.clearColor.isEmpty()) {
        pic.hasTransparentColor = true;
        pic.transparentColor = QColor(QLatin1Char('#') + s.clearColor);
    }
    if (s.borderWeight > 0) pic.stroke = Stroke::line(colorOf(s.borderColor.isEmpty() ? QStringLiteral("000000") : s.borderColor), s.borderWeight);
}

ItemPtr letteringItem(const Shape &s)
{
    if (s.pathText.isEmpty() || s.kind == Shape::Picture || s.width <= 0 || s.height <= 0) return nullptr;
    auto art = std::make_shared<TextArtItem>();
    art->rect = QRectF(0, 0, s.width, s.height);
    art->text = s.pathText;
    art->font = s.pathFont.isEmpty() ? QStringLiteral("Arial") : s.pathFont;
    art->bold = s.pathBold;
    art->italic = s.pathItalic;
    art->fill = s.filled && !s.fillColor.isEmpty() ? Fill::solid(colorOf(s.fillColor)) : Fill::none();
    art->stroke = s.stroked && s.strokeWeight > 0 ? Stroke::line(colorOf(s.strokeColor), s.strokeWeight) : Stroke::none();
    if (s.shadow) {
        ShadowFx &shadow = art->fx.shadow;
        shadow.on = true;
        shadow.color = colorOf(s.shadowColor.isEmpty() ? QStringLiteral("808080") : s.shadowColor);
        shadow.blur = 0;
        shadow.transparency = 0.2;
        shadow.distance = std::hypot(s.shadowX, s.shadowY);
        shadow.angle = std::atan2(s.shadowY, s.shadowX) * 180 / 3.14159265358979323846;
    }
    art->rotation = s.rotation;
    art->flipH = s.flipH;
    art->flipV = s.flipV;
    art->lockAspect = false;
    art->wrap.mode = Wrap::None;
    return art;
}

ItemPtr ruleItem(const Shape &s, double textWidth)
{
    if (!s.rule) return nullptr;
    const double width = s.rulePercent > 0 ? textWidth * s.rulePercent / 100 : s.width;
    if (width <= 0 || s.height <= 0) return nullptr;
    auto rule = std::make_shared<ShapeItem>();
    rule->rect = QRectF(0, 0, width, s.height);
    rule->shape = QStringLiteral("rect");
    rule->fill = s.filled ? Fill::solid(colorOf(s.fillColor.isEmpty() ? QStringLiteral("A0A0A0") : s.fillColor)) : Fill::none();
    rule->stroke = s.stroked && s.strokeWeight > 0 ? Stroke::line(colorOf(s.strokeColor), s.strokeWeight) : Stroke::none();
    rule->wrap.mode = Wrap::None;
    // The room beside it sets it across the line (a little less than all of what is left, so that the line keeps it).
    const double room = std::max(0.0, textWidth - width - 0.02);
    rule->wrap.left = s.hAlign == Shape::Center ? room / 2 : s.hAlign == Shape::End ? room : 0;
    rule->wrap.right = s.hAlign == Shape::Center ? room / 2 : 0;
    rule->wrap.top = rule->wrap.bottom = 0;
    return rule;
}

QString presetOf(const Shape &s)
{
    if (s.kind == Shape::Oval) return QStringLiteral("ellipse");
    if (s.kind == Shape::Rect && s.shapeType > 0) {
        const QString own = presetForDrawingShapeType(s.shapeType);
        if (!own.isEmpty()) return own;
    }
    return QStringLiteral("rect");
}

} // namespace jp::write
