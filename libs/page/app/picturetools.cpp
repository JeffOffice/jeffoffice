#include "app/picturetools.h"

#include "app/formatpresets.h"
#include "app/icons.h"
#include "core/blipeffects.h"
#include "core/picturefx.h"
#include "core/picturestyles.h"
#include "core/removebg.h"
#include "core/workthread.h"
#include "office/drawingml_write.h"
#include "render/shapes.h"
#include "render/renderer.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QHash>
#include <QLinearGradient>
#include <QPainter>
#include <QEventLoop>
#include <QProgressDialog>
#include <QWidgetAction>
#include <atomic>
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

QPixmap pictureStylePixmap(int k, const ColorScheme &scheme, const QSize &tile, const QImage &given)
{
    // A picture of the size a slide's would be (200 by 140 points) is drawn small, so that a border, a blur, and a bevel are as thick in the tile as they are in use.
    double kWidth = 200, kHeight = 140;
    const double ratio = 2;
    Document doc;
    doc.colors = scheme;
    const QImage sample = given.isNull() ? samplePicture() : given;
    if (!given.isNull()) {
        // (A picture given keeps its shape inside the same box.)
        const double fit = std::min(kWidth / sample.width(), kHeight / sample.height());
        kWidth = std::max(1.0, sample.width() * fit);
        kHeight = std::max(1.0, sample.height() * fit);
    }
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

QVector<GalleryItem> pictureStyleItems(const ColorScheme &scheme, const QSize &tile, const QImage &sample)
{
    // The tiles are drawn once for a set of colors (and the names are as the language was when they were made).
    static QHash<QString, QVector<GalleryItem>> made;
    QString key = QStringLiteral("%1x%2").arg(tile.width()).arg(tile.height()) + QCoreApplication::translate("jp::PictureTools", "Picture Style");
    for (const QColor &c : scheme.c) key += c.name(QColor::HexRgb);
    if (!sample.isNull()) key += QStringLiteral("|%1").arg(sample.cacheKey());
    const auto it = made.constFind(key);
    if (it != made.constEnd()) return *it;
    QVector<GalleryItem> items;
    for (int k = 0; k < pictureStyleCount(); ++k) items << GalleryItem{QString::number(k), pictureStyleName(k), QIcon(pictureStylePixmap(k, scheme, tile, sample)), QString()};
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
    ed->cancelRemoveBackgroundFor(ed->selection());   // (work still going on for the picture would put its answer back after the reset)
    ed->forEachSelected(andSize ? QCoreApplication::translate("jp::PictureTools", "Reset Picture & Size") : QCoreApplication::translate("jp::PictureTools", "Reset Picture"), [ed, andSize](Item *it) {
        if (holdsPicture(it)) resetPictureLook(static_cast<PictureItem *>(it), ed->doc()->imageSize(static_cast<PictureItem *>(it)->imageId), andSize);
    });
    return true;
}

bool changePictureTo(Editor *ed, const QString &pictureIdIn, const QString &imageId)
{
    if (!ed) return false;
    const QString pictureId = pictureIdIn;   // (a copy: the caller may hold the id of the picture itself, which the mode's end replaces)
    ed->cancelRemoveBackgroundFor({pictureId});   // (work still going on for the old picture would put its mask on the new one; first, as it takes the document's items back)
    auto *pic = dynamic_cast<PictureItem *>(ed->doc()->item(pictureId));
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

namespace {
std::atomic<int> threadedRuns{0};

QString signedPercent(int v) { return v > 0 ? QStringLiteral("+%1%").arg(v) : QStringLiteral("%1%").arg(v); }

bool plainPicture(const Item *it)
{
    const auto *p = dynamic_cast<const PictureItem *>(it);
    return p && p->mediaKind.isEmpty() && p->chartXml.isEmpty() && !p->imageId.isEmpty();
}
} // namespace

QVector<PictureFxCommand> pictureSharpenCommands()
{
    QVector<PictureFxCommand> v;
    const QString step = QCoreApplication::translate("jp::PictureTools", "Sharpen and Soften");
    for (int amount : {-50, -25, 0, 25, 50}) {
        const QString text = amount == 0 ? QCoreApplication::translate("jp::PictureTools", "Sharpen: 0% (Normal)")
                           : amount < 0 ? QCoreApplication::translate("jp::PictureTools", "Soften: %1%").arg(-amount)
                                        : QCoreApplication::translate("jp::PictureTools", "Sharpen: %1%").arg(amount);
        v.append({QStringLiteral("pic.sharpen.%1").arg(amount), text, step, [amount](PictureItem &p) { p.setSharpenMove(amount); }});
    }
    return v;
}

QVector<PictureFxCommand> pictureTemperatureCommands()
{
    QVector<PictureFxCommand> v;
    const QString step = QCoreApplication::translate("jp::PictureTools", "Color Temperature");
    for (int amount : {-100, -50, 0, 50, 100}) {
        const QString text = amount == 0 ? QCoreApplication::translate("jp::PictureTools", "Temperature: 0% (Normal)")
                           : amount < 0 ? QCoreApplication::translate("jp::PictureTools", "Temperature: %1 (Cooler)").arg(signedPercent(amount))
                                        : QCoreApplication::translate("jp::PictureTools", "Temperature: %1 (Warmer)").arg(signedPercent(amount));
        v.append({QStringLiteral("pic.temperature.%1").arg(amount), text, step, [amount](PictureItem &p) { p.setTemperatureMove(amount); }});
    }
    return v;
}

QVector<GalleryItem> artisticEffectItems(const QImage &sample, const QSize &tile)
{
    QImage base = sample;
    if (base.isNull()) {
        // A sample with a sun, a hill, and sky, so that every effect has something to show.
        base = QImage(96, 72, QImage::Format_ARGB32);
        base.fill(QColor(120, 180, 230));
        QPainter p(&base);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(90, 160, 80));
        p.drawEllipse(QRect(-20, 40, 140, 70));
        p.setBrush(QColor(250, 210, 60));
        p.drawEllipse(QRect(64, 8, 20, 20));
    }
    // The thumbnails are worked out on a small copy (a few thousand pixels), in the same filters the picture gets.
    const QImage small = base.scaled(tile * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
    QVector<GalleryItem> v;
    const QString none = QCoreApplication::translate("jp::PictureTools", "No Artistic Effect");
    BlipEffect e;
    e.kind = BlipEffect::Artistic;
    e.amount = 0.5;
    for (int style = -1; style <= int(ArtisticStyle::Last); ++style) {
        QImage shown = small;
        QString name = none;
        if (style >= 0) {
            e.style = style;
            shown = applyBlipEffects(small, {e});
            name = blipFilterName(e);
        }
        QPixmap pm = QPixmap::fromImage(shown.scaled(tile, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        v.append(GalleryItem{QString::number(style), name, QIcon(pm), QString()});
    }
    return v;
}

GalleryButton *artisticEffectsButton(const std::function<QImage()> &sample, const std::function<void(int)> &chosen, const QString &objectName)
{
    auto *b = new GalleryButton(icon("wand-sparkles"), QCoreApplication::translate("jp::PictureTools", "Artistic Effects"), QSize(48, 36), 5, true);
    if (!objectName.isEmpty()) b->setObjectName(objectName);
    b->setItemsProvider([sample] { return artisticEffectItems(sample ? sample() : QImage()); });
    QObject::connect(b, &GalleryButton::activated, b, [chosen](const QString &id) { chosen(id.toInt()); });
    return b;
}

void applyArtisticEffect(Editor *ed, QWidget *window, int style)
{
    changePicturesWithFilter(ed, window, QCoreApplication::translate("jp::PictureTools", "Artistic Effects"), [style](PictureItem &p) { p.setArtistic(style); });
}

int pictureFxThreadedRuns() { return threadedRuns; }

std::function<void(const std::atomic<bool> &)> pictureFxWorkerHook;

void changePicturesWithFilter(Editor *ed, QWidget *window, const QString &step, const std::function<void(PictureItem &)> &change)
{
    // What the pictures would have for filters after the change, and what that costs: worked out beside the document, which is not touched until the work is done.
    struct Job { QImage image; QVector<BlipEffect> filters; };
    std::vector<Job> jobs;
    qint64 work = 0;
    for (Item *it : ed->selectedItems()) {
        if (!plainPicture(it)) continue;
        PictureItem probe(*static_cast<PictureItem *>(it));
        change(probe);
        Job job{ed->doc()->image(probe.imageId), blipFilters(probe.blipEffects)};
        if (job.image.isNull() || !blipEffectsChange(job.filters) || probe.removal.isValid()) continue;   // (a removal changes the picture first: drawn, not prepared)
        work += blipEffectsWork(qint64(job.image.width()) * job.image.height(), job.filters);
        jobs.push_back(std::move(job));
    }
    if (work > kPictureFxBusyWork && window) {
        std::atomic<bool> stop{false};
        QEventLoop loop;
        QProgressDialog box(QCoreApplication::translate("jp::PictureTools", "Changing the picture…"), QCoreApplication::translate("jp::PictureTools", "Cancel"), 0, 0, window);
        box.setObjectName(QStringLiteral("pictureEffectBusy"));
        box.setWindowModality(Qt::WindowModal);
        box.setMinimumDuration(400);
        QObject::connect(&box, &QProgressDialog::canceled, &box, [&stop] { stop = true; });
        ++threadedRuns;
        bool finished = false;
        WorkThread thread([&jobs, &stop, &loop, &finished] {
            if (pictureFxWorkerHook) pictureFxWorkerHook(stop);
            for (const Job &j : jobs) blipAdjusted(j.image, j.filters, &stop);
            QMetaObject::invokeMethod(&loop, [&loop, &finished] { finished = true; loop.quit(); }, Qt::QueuedConnection);
        });
        if (!thread.started()) stop = true;
        else loop.exec();
        thread.join();
        if (stop || !finished) return;   // (cancelled: nothing changes)
    } else {
        for (const Job &j : jobs) blipAdjusted(j.image, j.filters);
    }
    ed->forEachSelected(step, [&change](Item *it) {
        if (plainPicture(it)) change(*static_cast<PictureItem *>(it));
    });
}

namespace {

// A picture of pixels (not a drawing kept as drawing: those have nothing to take away).
bool bakeable(const Document &doc, const PictureItem &p)
{
    if (!p.mediaKind.isEmpty() || !p.chartXml.isEmpty() || p.imageId.isEmpty()) return false;
    const QString format = doc.images.value(p.imageId).format;
    return format != QLatin1String("svg") && format != QLatin1String("wmf") && format != QLatin1String("emf") && format != QLatin1String("pdf") && doc.imageSize(p.imageId).isValid();
}

// A crop of less than this (in points) is the picture cut to whole pixels, not a crop to list.
constexpr double kCropSlack = 1.5;

// The part of the picture that shows, frame-local (as imgRect is).
QRectF shownPart(const PictureItem &p) { return QRectF(QPointF(0, 0), p.rect.size()).intersected(p.imgRect); }

bool keeps(const Document &doc, const PictureItem &p, PictureKept kind)
{
    if (!bakeable(doc, p)) return false;
    switch (kind) {
    case PictureKept::FilterOriginals: return blipEffectsChange(blipFilters(p.blipEffects));
    case PictureKept::RemovedBackgrounds: return p.removal.isValid() && p.removal.source == doc.imageSize(p.imageId);
    case PictureKept::CroppedAreas: {
        const QRectF vis = shownPart(p);
        return !p.imgRect.isEmpty() && vis.width() >= 1 && vis.height() >= 1 && (vis.width() < p.imgRect.width() - kCropSlack || vis.height() < p.imgRect.height() - kCropSlack);
    }
    }
    return false;
}

// The picture as a finished picture of its own: with the removal and the filters worked in (as they are drawn: removal first), and cut to what shows when `crop`. The removal is worked
// in whenever the filters are (it can't stay beside a picture made another size by them).
bool bakePicture(Document &doc, PictureItem &p, PictureKept kind)
{
    const bool filters = kind == PictureKept::FilterOriginals, removal = p.removal.isValid() && p.removal.source == doc.imageSize(p.imageId) && (filters || kind == PictureKept::RemovedBackgrounds),
               crop = kind == PictureKept::CroppedAreas;
    QImage img = doc.image(p.imageId);
    if (img.isNull()) return false;
    if (removal) img = applyBackgroundRemoval(img, p.removal);
    if (filters) img = applyBlipEffects(img, blipFilters(p.blipEffects));
    QRectF newRect = p.imgRect;
    if (crop) {
        const QRectF vis = shownPart(p);
        const double sx = img.width() / p.imgRect.width(), sy = img.height() / p.imgRect.height();
        const QRect px = QRect(int(std::floor((vis.x() - p.imgRect.x()) * sx)), int(std::floor((vis.y() - p.imgRect.y()) * sy)), std::max(1, int(std::ceil(vis.width() * sx))), std::max(1, int(std::ceil(vis.height() * sy)))).intersected(img.rect());
        if (px.isEmpty()) return false;
        img = img.copy(px);
        newRect = QRectF(p.imgRect.x() + px.x() / sx, p.imgRect.y() + px.y() / sy, px.width() / sx, px.height() / sy);
    }
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    if (!img.convertToFormat(QImage::Format_ARGB32).save(&buffer, "PNG")) return false;
    p.imageId = doc.addImage(png, QStringLiteral("png"));
    p.imgRect = newRect;
    if (removal) p.removal = BackgroundRemoval();
    if (filters) p.blipEffects = blipColorEffects(p.blipEffects);
    return true;
}

} // namespace

int picturesKeeping(const Document &doc, PictureKept kind)
{
    int n = 0;
    doc.forEachItem([&](Item *it, int, const QString &) {
        if (const auto *p = dynamic_cast<const PictureItem *>(it); p && keeps(doc, *p, kind)) ++n;
    });
    return n;
}

int picturesWithEffectsLeftOut(const Document &doc)
{
    int n = 0;
    doc.forEachItem([&](Item *it, int, const QString &) {
        const auto *p = dynamic_cast<const PictureItem *>(it);
        if (p && bakeable(doc, *p) && blipFiltersLeftOut(qint64(doc.imageSize(p->imageId).width()) * doc.imageSize(p->imageId).height(), p->blipEffects) > 0) ++n;
    });
    return n;
}

int removeKeptPictureParts(Editor *ed, PictureKept kind)
{
    QVector<PictureItem *> pics;
    ed->doc()->forEachItem([&](Item *it, int, const QString &) {
        if (auto *p = dynamic_cast<PictureItem *>(it); p && keeps(*ed->doc(), *p, kind)) pics << p;
    });
    int n = 0;
    const QString step = kind == PictureKept::FilterOriginals ? QCoreApplication::translate("jp::PictureTools", "Remove Original Pictures")
                       : kind == PictureKept::RemovedBackgrounds ? QCoreApplication::translate("jp::PictureTools", "Remove Pictures Under Removed Backgrounds")
                                                                 : QCoreApplication::translate("jp::PictureTools", "Remove Cropped Areas");
    ed->change(step, [&] {
        for (PictureItem *p : pics)
            if (bakePicture(*ed->doc(), *p, kind)) ++n;
    });
    return n;
}

QVector<InspectItem> pictureInspectItems(Editor *ed)
{
    auto row = [ed](PictureKept kind, const QString &title, const QString &detail, const QString &action) {
        InspectItem item;
        item.title = title;
        item.detail = detail;
        item.count = [ed, kind] { return picturesKeeping(*ed->doc(), kind); };
        item.remove = [ed, kind] { return removeKeptPictureParts(ed, kind); };
        item.action = action;
        return item;
    };
    QVector<InspectItem> rows{
        row(PictureKept::FilterOriginals, QCoreApplication::translate("jp::PictureTools", "Original pictures kept under picture effects"),
            QCoreApplication::translate("jp::PictureTools", "A picture effect such as Blur or Mosaic changes how a picture looks, but the original picture stays in the file, and anyone who gets the file can take it out. Removing it keeps only the finished picture: Reset Picture can then no longer bring the original back."),
            QCoreApplication::translate("jp::PictureTools", "Remove Originals")),
        row(PictureKept::RemovedBackgrounds, QCoreApplication::translate("jp::PictureTools", "Whole pictures kept under removed backgrounds"),
            QCoreApplication::translate("jp::PictureTools", "When you remove the background of a picture, the whole original picture stays in the file. Removing it keeps only the picture without its background: Remove Background can then no longer bring the background back."),
            QCoreApplication::translate("jp::PictureTools", "Remove Originals")),
        row(PictureKept::CroppedAreas, QCoreApplication::translate("jp::PictureTools", "Cropped-away parts of pictures"),
            QCoreApplication::translate("jp::PictureTools", "A cropped picture still holds the parts you cropped away, and anyone who gets the file can see them. Removing them cuts each picture down to what shows: the crop can then no longer be undone by dragging its edges out."),
            QCoreApplication::translate("jp::PictureTools", "Remove Cropped Parts")),
    };
    // (only a file can hold more effects than can be worked out: said, never left out silently)
    if (picturesWithEffectsLeftOut(*ed->doc()) > 0) {
        InspectItem left;
        left.title = QCoreApplication::translate("jp::PictureTools", "Picture effects that are left out");
        left.detail = QCoreApplication::translate("jp::PictureTools", "A picture in this file has more effects than can be worked out. It shows the first ones, and the rest are left out. Take effects off the picture to see it as it should look.");
        left.count = [ed] { return picturesWithEffectsLeftOut(*ed->doc()); };
        left.remove = [] { return 0; };
        left.removable = false;
        rows.push_back(left);
    }
    return rows;
}

} // namespace jp
