#include "app/picturetools.h"

#include "app/formatpresets.h"
#include "app/icons.h"
#include "core/picturestyles.h"
#include "office/drawingml_write.h"
#include "render/shapes.h"
#include "render/renderer.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QHash>
#include <QLinearGradient>
#include <QPainter>
#include <QWidgetAction>
#include <cmath>

namespace jp {

namespace {

// The sample picture the tiles show: sky, a sun, and two hills, so that a crop, a border, a bevel, and a turn all show.
QImage samplePicture()
{
    QImage img(160, 112, QImage::Format_ARGB32);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient sky(0, 0, 0, img.height());
    sky.setColorAt(0, QColor(88, 148, 214));
    sky.setColorAt(1, QColor(206, 228, 244));
    p.fillRect(img.rect(), sky);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 214, 92));
    p.drawEllipse(QPointF(118, 30), 15, 15);
    p.setBrush(QColor(74, 144, 92));
    p.drawEllipse(QPointF(40, 128), 90, 54);
    p.setBrush(QColor(46, 108, 72));
    p.drawEllipse(QPointF(128, 140), 80, 50);
    return img;
}

bool holdsPicture(const Item *it)
{
    const auto *p = dynamic_cast<const PictureItem *>(it);
    return p && !p->imageId.isEmpty() && p->mediaKind.isEmpty() && p->chartXml.isEmpty();
}

} // namespace

QPixmap pictureStylePixmap(int k, const ColorScheme &scheme, const QSize &tile)
{
    // A picture of the size a slide's would be (200 by 140 points) is drawn small, so that a border, a blur, and a bevel are as thick in the tile as they are in use.
    constexpr double kWidth = 200, kHeight = 140;
    const double ratio = 2;
    Document doc;
    doc.colors = scheme;
    const QImage sample = samplePicture();
    QByteArray bytes;
    {
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        sample.save(&buffer, "PNG");
    }
    PictureItem pic;
    pic.imageId = doc.addImage(bytes, QStringLiteral("png"), QString(), sample);
    pic.rect = QRectF(0, 0, kWidth, kHeight);
    pic.imgRect = pic.rect;
    if (k >= 0) applyPictureStyle(&pic, k);
    QPixmap pm((tile * ratio));
    pm.fill(Qt::transparent);
    pm.setDevicePixelRatio(ratio);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    const double scale = std::min(tile.width() * 0.70 / kWidth, tile.height() * 0.62 / kHeight);
    p.translate((tile.width() - kWidth * scale) / 2, tile.height() * 0.07);
    p.scale(scale, scale);
    PaintContext ctx;
    ctx.doc = &doc;
    Renderer::paintItem(&p, ctx, pic);
    return pm;
}

QVector<GalleryItem> pictureStyleItems(const ColorScheme &scheme, const QSize &tile)
{
    // The tiles are drawn once for a set of colors (and the names are as the language was when they were made).
    static QHash<QString, QVector<GalleryItem>> made;
    QString key = QStringLiteral("%1x%2").arg(tile.width()).arg(tile.height()) + QCoreApplication::translate("jp::PictureTools", "Picture Style");
    for (const QColor &c : scheme.c) key += c.name(QColor::HexRgb);
    const auto it = made.constFind(key);
    if (it != made.constEnd()) return *it;
    QVector<GalleryItem> items;
    for (int k = 0; k < pictureStyleCount(); ++k) items << GalleryItem{QString::number(k), pictureStyleName(k), QIcon(pictureStylePixmap(k, scheme, tile)), QString()};
    if (made.size() > 16) made.clear();
    made.insert(key, items);
    return items;
}

void applyPictureStyleToSelection(Editor *ed, int k)
{
    if (!ed || k < 0 || k >= pictureStyleCount()) return;
    ed->forEachSelected(QCoreApplication::translate("jp::PictureTools", "Picture Style"), [k](Item *it) {
        if (holdsPicture(it)) applyPictureStyle(static_cast<PictureItem *>(it), k);
    });
}

bool resetSelectedPictures(Editor *ed, bool andSize)
{
    if (!ed) return false;
    bool any = false;
    for (const Item *it : ed->selectedItems()) any = any || holdsPicture(it);
    if (!any) return false;
    ed->forEachSelected(andSize ? QCoreApplication::translate("jp::PictureTools", "Reset Picture & Size") : QCoreApplication::translate("jp::PictureTools", "Reset Picture"), [ed, andSize](Item *it) {
        if (holdsPicture(it)) resetPictureLook(static_cast<PictureItem *>(it), ed->doc()->imageSize(static_cast<PictureItem *>(it)->imageId), andSize);
    });
    return true;
}

bool changePictureTo(Editor *ed, const QString &pictureId, const QString &imageId)
{
    auto *pic = dynamic_cast<PictureItem *>(ed ? ed->doc()->item(pictureId) : nullptr);
    if (!pic || imageId.isEmpty()) return false;
    const QSize px = ed->doc()->imageSize(imageId);
    ed->change(QCoreApplication::translate("jp::PictureTools", "Change Picture"), [&] { changePictureImage(pic, imageId, px); });
    return true;
}

void fillPictureBorderMenu(QMenu *menu, Editor *ed, QWidget *dialogParent, const std::function<void()> &options)
{
    menu->clear();
    const ColorScheme scheme = ed->doc()->colors;
    const QString label = QCoreApplication::translate("jp::PictureTools", "Picture Border");
    auto border = [ed, label](std::function<void(Stroke &)> change) {
        ed->forEachSelected(label, [change](Item *it) {
            if (holdsPicture(it)) change(it->stroke);
        });
    };
    auto *grid = new QWidgetAction(menu);
    grid->setDefaultWidget(colorGridWidget(scheme, [menu, border](const ColorRef &c) {
        menu->close();
        border([c](Stroke &st) {
            if (st.isNone() || st.width <= 0) st = Stroke::line(c, 2.25);
            else st.color = c;
        });
    }));
    menu->addAction(grid);
    menu->addAction(QCoreApplication::translate("jp::PictureTools", "No Outline"), menu, [border] { border([](Stroke &st) { st.color = ColorRef::none(); }); });
    menu->addAction(icon("palette"), QCoreApplication::translate("jp::PictureTools", "More Outline Colors…"), menu, [border, dialogParent] {
        const QColor c = colorsDialog(dialogParent, Qt::black);
        if (!c.isValid()) return;
        border([c](Stroke &st) {
            if (st.isNone() || st.width <= 0) st = Stroke::line(ColorRef::rgb(c), 2.25);
            else st.color = ColorRef::rgb(c);
        });
    });
    menu->addSeparator();
    double shared = -1;
    bool same = true;
    for (const Item *it : ed->selectedItems())
        if (holdsPicture(it) && !it->stroke.isNone()) {
            if (shared < 0) shared = it->stroke.width;
            else if (std::abs(shared - it->stroke.width) > 1e-6) same = false;
        }
    QMenu *weight = menu->addMenu(icon("minus"), QCoreApplication::translate("jp::PictureTools", "Weight"));
    for (const double pt : {0.25, 0.5, 0.75, 1.0, 1.5, 2.25, 3.0, 4.5, 6.0}) {
        QAction *a = weight->addAction(QCoreApplication::translate("jp::PictureTools", "%1 pt").arg(pt), weight, [border, pt] {
            border([pt](Stroke &st) {
                if (st.isNone()) st = Stroke::line(ColorRef::scheme(Main), pt);
                st.width = pt;
            });
        });
        a->setCheckable(true);
        a->setChecked(same && std::abs(shared - pt) < 1e-6);
    }
    QMenu *dashes = menu->addMenu(icon("ellipsis"), QCoreApplication::translate("jp::PictureTools", "Dashes"));
    for (int d = 0; d <= int(Stroke::LongDashDotDot); ++d)
        dashes->addAction(dashName(Stroke::Dash(d)), dashes, [border, d] {
            border([d](Stroke &st) {
                if (st.isNone()) st = Stroke::line(ColorRef::scheme(Main), 2.25);
                st.dash = Stroke::Dash(d);
            });
        });
    if (options) {
        menu->addSeparator();
        menu->addAction(icon("sliders-horizontal"), QCoreApplication::translate("jp::PictureTools", "More Border Options…"), menu, options);
    }
}

void fillPictureEffectsMenu(QMenu *menu, Editor *ed, const std::function<void()> &options)
{
    fillPictureEffectsMenu(menu, [ed](const QString &label, const EffectChange &change) {
        ed->forEachSelected(label, [change](Item *it) {
            if (holdsPicture(it)) change(it->fx);
        });
    }, options);
}

void fillPictureEffectsMenu(QMenu *menu, const std::function<void(const QString &label, const EffectChange &change)> &effects, const std::function<void()> &options)
{
    menu->clear();
    const QString label = QCoreApplication::translate("jp::PictureTools", "Picture Effects");
    QMenu *shadow = menu->addMenu(icon("layers"), QCoreApplication::translate("jp::PictureTools", "Shadow"));
    shadow->addAction(QCoreApplication::translate("jp::PictureTools", "No Shadow"), shadow, [effects, label] { effects(label, [](Effects &fx) { fx.shadow.on = false; }); });
    shadow->addSeparator();
    for (int i = 0; i < formatpresets::shadowCount(); ++i)
        shadow->addAction(formatpresets::shadowName(i), shadow, [effects, label, i] { effects(label, [i](Effects &fx) { formatpresets::applyShadow(fx, i); }); });
    QMenu *reflection = menu->addMenu(icon("flip-vertical-2"), QCoreApplication::translate("jp::PictureTools", "Reflection"));
    reflection->addAction(QCoreApplication::translate("jp::PictureTools", "No Reflection"), reflection, [effects, label] { effects(label, [](Effects &fx) { fx.reflection.on = false; }); });
    reflection->addSeparator();
    for (int i = 0; i < formatpresets::reflectionCount(); ++i)
        reflection->addAction(formatpresets::reflectionName(i), reflection, [effects, label, i] { effects(label, [i](Effects &fx) { formatpresets::applyReflection(fx, i); }); });
    QMenu *glow = menu->addMenu(icon("sparkles"), QCoreApplication::translate("jp::PictureTools", "Glow"));
    glow->addAction(QCoreApplication::translate("jp::PictureTools", "No Glow"), glow, [effects, label] { effects(label, [](Effects &fx) { fx.glow.on = false; }); });
    glow->addSeparator();
    for (int i = 0; i < formatpresets::glowCount(); ++i)
        glow->addAction(formatpresets::glowName(i), glow, [effects, label, i] { effects(label, [i](Effects &fx) { formatpresets::applyGlow(fx, i, ColorRef::scheme(Accent1)); }); });
    QMenu *soft = menu->addMenu(icon("circle-dashed"), QCoreApplication::translate("jp::PictureTools", "Soft Edges"));
    soft->addAction(QCoreApplication::translate("jp::PictureTools", "No Soft Edges"), soft, [effects, label] { effects(label, [](Effects &fx) { fx.softEdge = 0; }); });
    soft->addSeparator();
    for (int i = 0; i < formatpresets::softEdgeCount(); ++i)
        soft->addAction(formatpresets::softEdgeName(i), soft, [effects, label, i] { effects(label, [i](Effects &fx) { formatpresets::applySoftEdge(fx, i); }); });
    QMenu *bevel = menu->addMenu(icon("square"), QCoreApplication::translate("jp::PictureTools", "Bevel"));
    bevel->addAction(QCoreApplication::translate("jp::PictureTools", "No Bevel"), bevel, [effects, label] { effects(label, [](Effects &fx) { fx.bevel.type = 0; }); });
    bevel->addSeparator();
    for (int i = 0; i < formatpresets::bevelCount(); ++i)
        bevel->addAction(formatpresets::bevelName(i), bevel, [effects, label, i] { effects(label, [i](Effects &fx) { formatpresets::applyBevel(fx, i); }); });
    QMenu *rotation = menu->addMenu(icon("box"), QCoreApplication::translate("jp::PictureTools", "3-D Rotation"));
    for (int i = 0; i < formatpresets::rotationCount(); ++i)
        rotation->addAction(formatpresets::rotationName(i), rotation, [effects, label, i] { effects(label, [i](Effects &fx) { formatpresets::applyRotation(fx, i); }); });
    if (options) {
        menu->addSeparator();
        menu->addAction(icon("sliders-horizontal"), QCoreApplication::translate("jp::PictureTools", "More Effects Options…"), menu, options);
    }
}

QVector<GalleryItem> cropShapeItems()
{
    QVector<GalleryItem> v;
    for (const auto &s : shapeLibrary())
        if (!s.open && !office::presetForShape(s.id).isEmpty()) v << GalleryItem{s.id, s.name, shapeIcon(s.id), s.category};
    return v;
}

GalleryButton *cropToShapeButton(const std::function<void(const QString &shapeId)> &chosen, const QString &objectName)
{
    auto *b = new GalleryButton(icon("shapes"), QCoreApplication::translate("jp::PictureTools", "Crop to Shape"), QSize(24, 24), 12, true);
    if (!objectName.isEmpty()) b->setObjectName(objectName);
    b->setItemsProvider([] { return cropShapeItems(); });
    QObject::connect(b, &GalleryButton::activated, b, [chosen](const QString &id) { chosen(id); });
    return b;
}

} // namespace jp
