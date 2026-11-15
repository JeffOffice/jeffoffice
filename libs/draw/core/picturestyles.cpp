#include "core/picturestyles.h"

#include <QCoreApplication>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

struct Style {
    const char *id;
    const char *name;
    void (*apply)(PictureItem *);
};

ColorRef white() { return ColorRef::rgb(Qt::white); }
ColorRef dark() { return ColorRef::rgb(QColor(40, 40, 40)); }

void shadow(PictureItem *p, double blur, double distance, double transparency = 0.6, double angle = 45)
{
    p->fx.shadow.on = true;
    p->fx.shadow.blur = blur;
    p->fx.shadow.distance = distance;
    p->fx.shadow.transparency = transparency;
    p->fx.shadow.angle = angle;
}

const Style kStyles[] = {
    {"plain", QT_TRANSLATE_NOOP("jp::PictureStyles", "Plain"), [](PictureItem *) {}},
    {"thin-frame", QT_TRANSLATE_NOOP("jp::PictureStyles", "Thin Dark Frame"), [](PictureItem *p) { p->stroke = Stroke::line(dark(), 1); }},
    {"heavy-frame", QT_TRANSLATE_NOOP("jp::PictureStyles", "Heavy Dark Frame"), [](PictureItem *p) { p->stroke = Stroke::line(dark(), 8); }},
    {"white-frame", QT_TRANSLATE_NOOP("jp::PictureStyles", "White Frame"), [](PictureItem *p) { p->stroke = Stroke::line(white(), 6); }},
    {"snapshot", QT_TRANSLATE_NOOP("jp::PictureStyles", "Snapshot"), [](PictureItem *p) { p->stroke = Stroke::line(white(), 8); shadow(p, 8, 4); }},
    {"double-frame", QT_TRANSLATE_NOOP("jp::PictureStyles", "Double Frame"), [](PictureItem *p) {
        p->stroke = Stroke::line(ColorRef::scheme(Accent1), 4.5);
        p->stroke.compound = Stroke::Double;
    }},
    {"soft-shadow", QT_TRANSLATE_NOOP("jp::PictureStyles", "Soft Shadow"), [](PictureItem *p) { shadow(p, 10, 6); }},
    {"hard-shadow", QT_TRANSLATE_NOOP("jp::PictureStyles", "Hard Shadow"), [](PictureItem *p) { shadow(p, 0, 5, 0.5); }},
    {"reflected", QT_TRANSLATE_NOOP("jp::PictureStyles", "Reflected"), [](PictureItem *p) {
        p->fx.reflection.on = true;
        p->fx.reflection.size = 0.4;
        p->fx.reflection.distance = 2;
    }},
    {"reflected-frame", QT_TRANSLATE_NOOP("jp::PictureStyles", "Reflected in a Frame"), [](PictureItem *p) {
        p->stroke = Stroke::line(white(), 5);
        p->fx.reflection.on = true;
        p->fx.reflection.size = 0.35;
        p->fx.reflection.distance = 2;
    }},
    {"soft-edges", QT_TRANSLATE_NOOP("jp::PictureStyles", "Soft Edges"), [](PictureItem *p) { p->fx.softEdge = 10; }},
    {"feathered-oval", QT_TRANSLATE_NOOP("jp::PictureStyles", "Feathered Oval"), [](PictureItem *p) {
        p->maskShape = QStringLiteral("ellipse");
        p->fx.softEdge = 12;
    }},
    {"rounded", QT_TRANSLATE_NOOP("jp::PictureStyles", "Rounded Corners"), [](PictureItem *p) { p->maskShape = QStringLiteral("roundRect"); }},
    {"rounded-frame", QT_TRANSLATE_NOOP("jp::PictureStyles", "Rounded with Frame"), [](PictureItem *p) {
        p->maskShape = QStringLiteral("roundRect");
        p->stroke = Stroke::line(white(), 4);
        shadow(p, 6, 3);
    }},
    {"oval", QT_TRANSLATE_NOOP("jp::PictureStyles", "Oval"), [](PictureItem *p) { p->maskShape = QStringLiteral("ellipse"); }},
    {"oval-frame", QT_TRANSLATE_NOOP("jp::PictureStyles", "Oval with Frame"), [](PictureItem *p) {
        p->maskShape = QStringLiteral("ellipse");
        p->stroke = Stroke::line(white(), 5);
        shadow(p, 6, 3);
    }},
    {"one-round-corner", QT_TRANSLATE_NOOP("jp::PictureStyles", "One Round Corner"), [](PictureItem *p) {
        p->maskShape = QStringLiteral("round1");
        p->stroke = Stroke::line(white(), 3);
        shadow(p, 4, 2, 0.7);
    }},
    {"snipped", QT_TRANSLATE_NOOP("jp::PictureStyles", "Snipped Corners"), [](PictureItem *p) {
        p->maskShape = QStringLiteral("snip2diag");
        p->stroke = Stroke::line(ColorRef::scheme(Accent1), 2);
    }},
    {"hexagon", QT_TRANSLATE_NOOP("jp::PictureStyles", "Hexagon"), [](PictureItem *p) {
        p->maskShape = QStringLiteral("hexagon");
        p->stroke = Stroke::line(ColorRef::scheme(Accent2), 2);
    }},
    {"octagon", QT_TRANSLATE_NOOP("jp::PictureStyles", "Octagon"), [](PictureItem *p) { p->maskShape = QStringLiteral("octagon"); }},
    {"plaque", QT_TRANSLATE_NOOP("jp::PictureStyles", "Plaque"), [](PictureItem *p) {
        p->maskShape = QStringLiteral("plaque");
        p->stroke = Stroke::line(ColorRef::scheme(Accent3), 2);
    }},
    {"turn-right", QT_TRANSLATE_NOOP("jp::PictureStyles", "Turned Right"), [](PictureItem *p) {
        p->fx.rot3d.y = -25;
        p->fx.rot3d.perspective = 40;
        shadow(p, 6, 4);
    }},
    {"turn-left", QT_TRANSLATE_NOOP("jp::PictureStyles", "Turned Left"), [](PictureItem *p) {
        p->fx.rot3d.y = 25;
        p->fx.rot3d.perspective = 40;
        shadow(p, 6, 4);
    }},
    {"glow", QT_TRANSLATE_NOOP("jp::PictureStyles", "Glow"), [](PictureItem *p) {
        p->fx.glow.on = true;
        p->fx.glow.size = 10;
    }},
    {"bevel", QT_TRANSLATE_NOOP("jp::PictureStyles", "Beveled"), [](PictureItem *p) {
        p->fx.bevel.type = 1;
        p->fx.bevel.width = 6;
        p->fx.bevel.height = 6;
    }},
    {"bevel-frame", QT_TRANSLATE_NOOP("jp::PictureStyles", "Beveled Frame"), [](PictureItem *p) {
        p->stroke = Stroke::line(white(), 6);
        p->fx.bevel.type = 2;
        p->fx.bevel.width = 5;
        p->fx.bevel.height = 5;
        shadow(p, 6, 3);
    }},
};

constexpr int kCount = int(sizeof(kStyles) / sizeof(kStyles[0]));

} // namespace

int pictureStyleCount() { return kCount; }

QString pictureStyleId(int k) { return k >= 0 && k < kCount ? QString::fromLatin1(kStyles[k].id) : QString(); }

QString pictureStyleName(int k) { return k >= 0 && k < kCount ? QCoreApplication::translate("jp::PictureStyles", kStyles[k].name) : QString(); }

int pictureStyleIndex(const QString &id)
{
    for (int k = 0; k < kCount; ++k)
        if (id == QLatin1String(kStyles[k].id)) return k;
    return -1;
}

bool applyPictureStyle(PictureItem *pic, int k)
{
    if (!pic || k < 0 || k >= kCount) return false;
    pic->maskShape = QStringLiteral("rect");
    pic->stroke = Stroke::none();
    pic->fx = Effects();
    kStyles[k].apply(pic);
    return true;
}

bool resetPictureLook(PictureItem *pic, const QSize &px, bool andSize)
{
    if (!pic) return false;
    const QJsonObject before = pic->toJson();
    pic->maskShape = QStringLiteral("rect");
    pic->stroke = Stroke::none();
    pic->fx = Effects();
    pic->brightness = pic->contrast = 0;
    pic->recolor = PictureItem::NoRecolor;
    pic->recolorColor = ColorRef();
    pic->hasTransparentColor = false;
    pic->transparentColor = QColor();
    pic->transparency = 0;
    pic->blackWhiteLevel = 0.5;
    pic->blipEffects.clear();
    if (px.isEmpty() || pic->imgRect.width() <= 0 || pic->imgRect.height() <= 0) {
        // A picture of no known size has no whole picture to show: the crop is taken off all the same.
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
    } else {
        // The frame grows (or shrinks) to hold the whole picture, which keeps its scale and its middle.
        const QPointF middle = pic->rect.center();
        const double scale = andSize ? 0.75 * px.width() / pic->imgRect.width() : 1.0;
        const QSizeF whole(pic->imgRect.width() * scale, pic->imgRect.height() * scale);
        pic->rect = QRectF(middle.x() - whole.width() / 2, middle.y() - whole.height() / 2, whole.width(), whole.height());
        pic->imgRect = QRectF(0, 0, whole.width(), whole.height());
    }
    return pic->toJson() != before;
}

void changePictureImage(PictureItem *pic, const QString &imageId, const QSize &px)
{
    if (!pic) return;
    const QSizeF whole = pic->imgRect.size();
    const bool same = !px.isEmpty() && whole.width() > 0 && whole.height() > 0 &&
                      std::abs(double(px.width()) / px.height() - whole.width() / whole.height()) < 0.02 * whole.width() / whole.height();
    pic->imageId = imageId;
    // The picture of the same proportions goes where the old one was; any other fills the frame.
    if (!same) pic->fitImage(px, true);
    pic->chartXml.clear();
}

} // namespace jp
