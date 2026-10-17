// The Picture page of the Format pane: a picture's corrections, color, and crop.
#include "app/formatpane_p.h"
#include "app/paneframe.h"

#include <QFormLayout>
#include <QVBoxLayout>

namespace jp::format {

namespace {

using std::nullopt;

// The pictures that have corrections to make: not a movie's or a sound's poster, which have a tab of their own.
const PictureItem *correctable(const Item &it)
{
    if (it.type() != ItemType::Picture) return nullptr;
    const auto *p = static_cast<const PictureItem *>(&it);
    return p->mediaKind.isEmpty() ? p : nullptr;
}

NumberField *percent(QWidget *parent, const char *name, double lo, double hi)
{
    auto *f = new NumberField(parent);
    f->setObjectName(QLatin1String(name));
    f->setRange(lo, hi);
    f->setDecimals(0);
    f->setSuffix(QStringLiteral("%"));
    f->setSlider(true);
    return f;
}

NumberField *measure(QWidget *parent, const char *name, double lo, double hi)
{
    auto *f = new NumberField(parent);
    f->setObjectName(QLatin1String(name));
    f->setMeasure(true);
    f->setRange(lo, hi);
    return f;
}

} // namespace

void buildPicturePage(FormatPane *pane, PanePage *page)
{
    // ---- corrections
    PaneSection *corrections = page->addSection(FormatPane::tr("Picture Corrections"));
    {
        QWidget *w = corrections->bodyWidget();
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        auto *brightness = percent(w, "imageBrightness", -100, 100);
        auto *contrast = percent(w, "imageContrast", -100, 100);
        form->addRow(FormatPane::tr("Brightness"), brightness);
        form->addRow(FormatPane::tr("Contrast"), contrast);
        corrections->body()->addLayout(form);
        const QString label = FormatPane::tr("Picture Corrections");
        bindNumber(pane, brightness, label, [](const Item &it) -> std::optional<double> { const auto *p = correctable(it); return p ? std::optional<double>(p->brightness) : nullopt; },
                   [](Item &it, double v) { static_cast<PictureItem &>(it).brightness = v; });
        bindNumber(pane, contrast, label, [](const Item &it) -> std::optional<double> { const auto *p = correctable(it); return p ? std::optional<double>(p->contrast) : nullopt; },
                   [](Item &it, double v) { static_cast<PictureItem &>(it).contrast = v; });
    }

    // ---- color
    PaneSection *color = page->addSection(FormatPane::tr("Picture Color"));
    {
        QWidget *w = color->bodyWidget();
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        auto *recolor = new QComboBox(w);
        recolor->setObjectName(QStringLiteral("imageRecolor"));
        recolor->addItems({FormatPane::tr("No Recolor"), FormatPane::tr("Grayscale"), FormatPane::tr("Black and White")});
        ignoreWheelUnlessFocused(recolor);
        auto *threshold = percent(w, "imageThreshold", 0, 100);
        auto *transparency = percent(w, "imageTransparency", 0, 100);
        form->addRow(FormatPane::tr("Recolor"), recolor);
        form->addRow(FormatPane::tr("Black and white level"), threshold);
        form->addRow(FormatPane::tr("Transparency"), transparency);
        color->body()->addLayout(form);
        const QString label = FormatPane::tr("Picture Color");
        bindChoice(pane, recolor, label, [](const Item &it) -> std::optional<int> {
            const auto *p = correctable(it);
            if (!p) return nullopt;
            return p->recolor == PictureItem::Grayscale ? 1 : p->recolor == PictureItem::BlackWhite ? 2 : 0;
        }, [](Item &it, int i) {
            static_cast<PictureItem &>(it).recolor = i == 1 ? PictureItem::Grayscale : i == 2 ? PictureItem::BlackWhite : PictureItem::NoRecolor;
        });
        bindNumber(pane, threshold, label, [](const Item &it) -> std::optional<double> {
            const auto *p = correctable(it);
            return p && p->recolor == PictureItem::BlackWhite ? std::optional<double>(p->blackWhiteLevel * 100) : nullopt;
        }, [](Item &it, double v) { static_cast<PictureItem &>(it).blackWhiteLevel = v / 100; });
        bindNumber(pane, transparency, label, [](const Item &it) -> std::optional<double> { const auto *p = correctable(it); return p ? std::optional<double>(p->transparency * 100) : nullopt; },
                   [](Item &it, double v) { static_cast<PictureItem &>(it).transparency = v / 100; });
    }

    // ---- crop: where the whole picture is in the frame (what is outside the frame is cropped away)
    PaneSection *crop = page->addSection(FormatPane::tr("Crop"), false);
    {
        QWidget *w = crop->bodyWidget();
        auto *form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        auto *width = measure(w, "cropWidth", 1, 8000);
        auto *height = measure(w, "cropHeight", 1, 8000);
        auto *offsetX = measure(w, "cropOffsetX", -8000, 8000);
        auto *offsetY = measure(w, "cropOffsetY", -8000, 8000);
        form->addRow(FormatPane::tr("Picture width"), width);
        form->addRow(FormatPane::tr("Picture height"), height);
        form->addRow(FormatPane::tr("Offset X"), offsetX);
        form->addRow(FormatPane::tr("Offset Y"), offsetY);
        crop->body()->addLayout(form);
        const QString label = FormatPane::tr("Crop");
        auto imgOf = [](const Item &it) -> std::optional<QRectF> { const auto *p = correctable(it); return p ? std::optional<QRectF>(p->imgRect) : nullopt; };
        // The offsets are the picture's middle from the frame's middle.
        bindNumber(pane, width, label, [=](const Item &it) -> std::optional<double> { const auto r = imgOf(it); return r ? std::optional<double>(r->width()) : nullopt; },
                   [](Item &it, double v) {
                       auto &p = static_cast<PictureItem &>(it);
                       const QPointF middle = p.imgRect.center();
                       p.imgRect = QRectF(0, 0, v, p.imgRect.height());
                       p.imgRect.moveCenter(middle);
                   });
        bindNumber(pane, height, label, [=](const Item &it) -> std::optional<double> { const auto r = imgOf(it); return r ? std::optional<double>(r->height()) : nullopt; },
                   [](Item &it, double v) {
                       auto &p = static_cast<PictureItem &>(it);
                       const QPointF middle = p.imgRect.center();
                       p.imgRect = QRectF(0, 0, p.imgRect.width(), v);
                       p.imgRect.moveCenter(middle);
                   });
        bindNumber(pane, offsetX, label, [=](const Item &it) -> std::optional<double> {
            const auto r = imgOf(it);
            return r ? std::optional<double>(r->center().x() - it.rect.width() / 2) : nullopt;
        }, [](Item &it, double v) {
            auto &p = static_cast<PictureItem &>(it);
            p.imgRect.moveCenter(QPointF(p.rect.width() / 2 + v, p.imgRect.center().y()));
        });
        bindNumber(pane, offsetY, label, [=](const Item &it) -> std::optional<double> {
            const auto r = imgOf(it);
            return r ? std::optional<double>(r->center().y() - it.rect.height() / 2) : nullopt;
        }, [](Item &it, double v) {
            auto &p = static_cast<PictureItem &>(it);
            p.imgRect.moveCenter(QPointF(p.imgRect.center().x(), p.rect.height() / 2 + v));
        });
    }
}

} // namespace jp::format
