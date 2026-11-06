#include "core/drawnitems.h"

#include "core/shapetypes.h"
#include "office/drawingml.h"
#include "office/drawingml_write.h"
#include "wml/xmltree.h"
#include "office/xml.h"

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
    // The colors it is changed to: its brightness and contrast, then gray or black and white (the order the standard's own writers give them).
    {
        QVector<BlipEffect> listed;
        if (s.bright != 0 || s.contrast != 0) {
            BlipEffect e;
            e.kind = BlipEffect::Luminance;
            e.brightness = s.bright;
            e.contrast = s.contrast;
            listed << e;
        }
        if (s.gray) {
            BlipEffect e;
            e.kind = BlipEffect::Grayscale;
            listed << e;
        }
        if (s.biLevel) {
            BlipEffect e;
            e.kind = BlipEffect::BiLevel;
            e.threshold = s.biLevelThreshold;
            listed << e;
        }
        if (!listed.isEmpty()) pic.takeEffects(listed);
    }
    if (!s.cropShape.isEmpty()) pic.maskShape = s.cropShape;
    pic.rotation = s.rotation;
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

ItemPtr wspItem(const Shape &s, const office::Theme *theme)
{
    if (s.wsp.isEmpty()) return nullptr;
    QDomDocument doc;
    if (!office::parseXml(s.wsp, &doc)) return nullptr;
    const QDomElement shape = doc.documentElement().firstChildElement();
    if (shape.isNull()) return nullptr;
    office::DrawingContext ctx;
    QDomDocument themeDoc;
    if (theme) {
        ctx.theme = *theme;
        // (The theme's format scheme: what a style reference's index points into.)
        if (!theme->xml.isEmpty() && office::parseXml(theme->xml, &themeDoc))
            for (QDomElement e = themeDoc.documentElement().firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
                if (e.localName() == QLatin1String("themeElements"))
                    for (QDomElement f = e.firstChildElement(); !f.isNull(); f = f.nextSiblingElement())
                        if (f.localName() == QLatin1String("fmtScheme")) ctx.formats = f;
    }
    ItemPtr item = office::readShape(shape, ctx);
    if (!item) return nullptr;
    if (item->type() == ItemType::Group) {
        const auto *group = static_cast<const GroupItem *>(item.get());
        if (group->children.empty()) return nullptr;
        item = group->children.front();
    }
    if (item->type() != ItemType::Line) item->rect = QRectF(0, 0, s.width, s.height);
    return item;
}

ItemPtr textArtItem(const Shape &s)
{
    if (s.kind != Shape::TextBox || s.wsp.isEmpty() || s.textBox.isEmpty()) return nullptr;
    XmlTree shape;
    if (!shape.parse(s.wsp, nullptr)) return nullptr;
    QString warp;
    {
        std::vector<int> stack{shape.root()};
        for (int steps = 0; !stack.empty() && steps < 400; ++steps) {
            const int n = stack.back();
            stack.pop_back();
            if (shape.localName(n) == "prstTxWarp") {
                warp = shape.attr(n, "prst");
                break;
            }
            for (int c = shape.firstChild(n); c >= 0; c = shape.next(c)) stack.push_back(c);
        }
    }
    const QString transform = warp.isEmpty() ? QString() : office::textArtTransformOfWarp(warp);
    if (transform.isEmpty() && warp != QLatin1String("textPlain")) return nullptr;
    XmlTree words;
    if (!words.parse(s.textBox, nullptr)) return nullptr;
    const int top = words.root();
    if (top < 0 || words.localName(top) != "txbxContent") return nullptr;
    auto art = std::make_shared<TextArtItem>();
    art->transform_ = transform.isEmpty() ? QStringLiteral("plain") : transform;
    QStringList lines;
    int firstRun = -1, firstPara = -1;
    for (int p = words.firstChild(top); p >= 0 && lines.size() < 200; p = words.next(p)) {
        if (words.localName(p) != "p") continue;
        if (firstPara < 0) firstPara = p;
        QString line;
        for (int r = words.firstChild(p); r >= 0; r = words.next(r)) {
            if (words.localName(r) != "r") continue;
            if (firstRun < 0) firstRun = r;
            for (int k = words.firstChild(r); k >= 0; k = words.next(k)) {
                const QByteArray local = words.localName(k);
                if (local == "t") line += words.text(k);
                else if (local == "br") line += QLatin1Char('\n');
                else if (local == "tab") line += QLatin1Char(' ');
            }
        }
        lines << line;
    }
    art->text = lines.join(QLatin1Char('\n'));
    if (const int ppr = firstPara >= 0 ? words.child(firstPara, "pPr") : -1; ppr >= 0)
        if (const int jc = words.child(ppr, "jc"); jc >= 0) {
            const QString v = words.val(jc);
            art->align = v == QLatin1String("left") || v == QLatin1String("start") ? 0 : v == QLatin1String("right") || v == QLatin1String("end") ? 2 : v == QLatin1String("both") ? 3 : 1;
        }
    QColor fill(Qt::black), line;
    double outline = 0;
    if (const int rpr = firstRun >= 0 ? words.child(firstRun, "rPr") : -1; rpr >= 0) {
        auto on = [&](const char *name) {
            const int n = words.child(rpr, name);
            if (n < 0) return false;
            const QString v = words.val(n);
            return v.isEmpty() || (v != QLatin1String("0") && v != QLatin1String("false") && v != QLatin1String("off"));
        };
        if (const int fonts = words.child(rpr, "rFonts"); fonts >= 0 && !words.attr(fonts, "ascii").isEmpty()) art->font = words.attr(fonts, "ascii");
        art->bold = on("b");
        art->italic = on("i");
        if (const int sz = words.child(rpr, "sz"); sz >= 0) art->size = std::clamp(words.val(sz).toDouble() / 2.0, 1.0, 4000.0);
        if (const int color = words.child(rpr, "color"); color >= 0 && QColor(QLatin1Char('#') + words.val(color)).isValid()) fill = QColor(QLatin1Char('#') + words.val(color));
        if (const int sp = words.child(rpr, "spacing"); sp >= 0) art->spacing = std::clamp(1.0 + words.val(sp).toDouble() / 20.0 / std::max(1.0, art->size), 0.5, 2.0);
        // The letters' own fill and outline (the newer markup's, which comes after the usual properties).
        for (int k = words.firstChild(rpr); k >= 0; k = words.next(k)) {
            const QByteArray local = words.localName(k);
            if (local != "textFill" && local != "textOutline") continue;
            std::vector<int> stack{k};
            QString rgb;
            for (int steps = 0; !stack.empty() && steps < 50 && rgb.isEmpty(); ++steps) {
                const int n = stack.back();
                stack.pop_back();
                if (words.localName(n) == "srgbClr") rgb = words.attrAny(n, "val");
                for (int c = words.firstChild(n); c >= 0; c = words.next(c)) stack.push_back(c);
            }
            const QColor c(QLatin1Char('#') + rgb);
            if (local == "textFill" && c.isValid()) fill = c;
            if (local == "textOutline" && c.isValid()) {
                line = c;
                outline = std::clamp(words.attrAny(k, "w").toDouble() / 12700.0, 0.0, 100.0);
            }
        }
    }
    art->fill = Fill::solid(ColorRef::rgb(fill));
    art->stroke = line.isValid() && outline > 0 ? Stroke::line(ColorRef::rgb(line), outline) : Stroke::none();
    return art;
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
