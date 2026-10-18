#include "core/drawnitems.h"

#include "core/shapetypes.h"

#include <QBuffer>
#include <QColor>
#include <QFontMetricsF>
#include <QImage>
#include <QPainter>
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
    // The border is the outline where all four sides are set (the heaviest weight); fewer are lines beside the picture (borderLines).
    if (s.borderWeight > 0 && s.borderSides[0] > 0 && s.borderSides[1] > 0 && s.borderSides[2] > 0 && s.borderSides[3] > 0)
        pic.stroke = Stroke::line(colorOf(s.borderColor.isEmpty() ? QStringLiteral("000000") : s.borderColor), s.borderWeight);
}

QVector<std::shared_ptr<LineItem>> borderLines(const Shape &s)
{
    QVector<std::shared_ptr<LineItem>> out;
    if (s.borderSides[0] > 0 && s.borderSides[1] > 0 && s.borderSides[2] > 0 && s.borderSides[3] > 0) return out;
    const double w = s.width, h = s.height;
    const QPointF ends[4][2] = {{QPointF(0, 0), QPointF(w, 0)}, {QPointF(0, 0), QPointF(0, h)}, {QPointF(0, h), QPointF(w, h)}, {QPointF(w, 0), QPointF(w, h)}};
    for (int side = 0; side < 4; ++side) {
        if (s.borderSides[side] <= 0) continue;
        auto line = std::make_shared<LineItem>();
        line->p1 = ends[side][0];
        line->p2 = ends[side][1];
        line->stroke = Stroke::line(colorOf(s.borderColor.isEmpty() ? QStringLiteral("000000") : s.borderColor), s.borderSides[side]);
        out << line;
    }
    return out;
}

bool isUndrawnPictureTarget(const QString &target) { return target.endsWith(QLatin1String(".pcz"), Qt::CaseInsensitive); }

const QByteArray &missingPictureMarkPng()
{
    static const QByteArray png = [] {
        // (Four pixels to the point: a mark 11 points across.)
        QImage image(44, 44, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter p(&image);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(120, 120, 120), 2));
        p.setBrush(QColor(245, 245, 245));
        p.drawRect(QRectF(1, 1, 42, 42));
        p.setPen(QPen(QColor(230, 20, 20), 6, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(QPointF(11, 11), QPointF(33, 33));
        p.drawLine(QPointF(33, 11), QPointF(11, 33));
        p.end();
        QByteArray out;
        QBuffer buffer(&out);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        return out;
    }();
    return png;
}

void showAsMissing(PictureItem &pic)
{
    pic.fill = Fill::solid(ColorRef::rgb(Qt::white));
    pic.stroke = Stroke::line(ColorRef::rgb(Qt::black), 0.75);
    pic.imgRect = QRectF(5, 6, 11, 11);
    pic.hasTransparentColor = false;
}

ItemPtr letteringItem(const Shape &s)
{
    if (s.pathText.isEmpty() || s.kind == Shape::Picture || s.width <= 0 || s.height <= 0) return nullptr;
    auto art = std::make_shared<TextArtItem>();
    art->rect = QRectF(0, 0, std::min(s.width, kMostLetteringSide), std::min(s.height, kMostLetteringSide));
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

ItemPtr ruleItem(const Shape &s, double textWidth, const QFont &font, bool alone)
{
    if (!s.rule) return nullptr;
    const double width = s.rulePercent > 0 ? textWidth * s.rulePercent / 100 : std::min(s.width, textWidth);   // (a rule of a width of its own goes no further than its line)
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
    // (A paragraph of only the rule has a line as tall as one of the text, the rule in the middle of it; a rule among the text takes the line of its own that it
    // wraps to, as high as it is.)
    rule->wrap.top = rule->wrap.bottom = 0;
    if (alone) {
        const QFontMetricsF metrics(font);
        rule->wrap.top = rule->wrap.bottom = std::max(0.0, (metrics.ascent() + metrics.descent() - s.height) / 2);
    }
    return rule;
}

double sectionTextWidth(const SectionProps &s)
{
    const double text = (s.pageW - s.left - s.right - s.gutter) / 20.0;
    const double columns = s.cols > 1 ? std::min(s.cols, 64) : 1;
    return std::max(1.0, (text - (columns - 1) * s.colSpace / 20.0) / columns);
}

bool holdsOnlyObjects(const QVector<Inline> &content)
{
    if (content.size() > 256) return false;
    for (const Inline &in : content) {
        if (in.kind == Inline::Text) {
            for (const QChar ch : in.text)
                if (!ch.isSpace()) return false;
        } else if (in.kind == Inline::RunAtom) {
            if (in.tag != QLatin1String("pict") && in.tag != QLatin1String("drawing") && in.tag != QLatin1String("object") && in.tag != QLatin1String("AlternateContent")) return false;
        }
    }
    return true;
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

void shapeOfKind(ShapeItem &item, const Shape &s)
{
    item.shape = presetOf(s);
    if (s.shapeType == 19 && s.kind == Shape::Rect) {
        const double w = item.rect.width(), h = item.rect.height();
        const QRectF ellipse(-w, 0, 2 * w, 2 * h);
        // Where it starts and where it ends: the shape's first two numbers, in sixty-five-thousandths of a degree clockwise from the right (the format's own: from the top, -90, to
        // the right, 0).
        auto angleOf = [&](int i, double standard) { return i < s.adjust.size() && std::isfinite(s.adjust[i]) ? s.adjust[i] / 65536.0 : standard; };
        const double start = angleOf(0, -90), end = angleOf(1, 0);
        double sweep = std::fmod(end - start, 360.0);
        if (sweep <= 0) sweep += 360;
        QPainterPath arc;
        arc.arcMoveTo(ellipse, -start);
        arc.arcTo(ellipse, -start, -sweep);
        item.customPath = arc;
    }
}

} // namespace jp::write
