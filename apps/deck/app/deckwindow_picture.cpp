// The Picture Format tab: Crop (with the picture's handles), Crop to Shape, Aspect Ratio, Fill, Fit, and Clear Crop. The commands that change the
// frame or the picture inside it are libs/page's (app/pictureedit.h, the canvas's crop handles), the ones JeffPub's Crop uses too.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/pictureedit.h"
#include "app/removebgprogress.h"
#include "app/pictureask.h"
#include "app/filedialogs.h"
#include "app/picturecompress.h"
#include "app/picturetools.h"
#include "app/settings.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/widgets.h"
#include "canvas/canvas.h"
#include "office/drawingml_write.h"
#include "render/shapes.h"

#include <QAction>
#include <QFileInfo>
#include <QMenu>
#include <QPointer>
#include <QStandardPaths>
#include <QStatusBar>

namespace jp {

namespace {

// The pictures a crop works on: those selected that hold a picture (not an empty place for one).
QVector<PictureItem *> croppablePictures(Editor *ed)
{
    QVector<PictureItem *> out;
    for (Item *it : ed->selectedItems())
        if (auto *p = dynamic_cast<PictureItem *>(it); p && !p->imageId.isEmpty()) out << p;
    return out;
}

// A command ends the crop in progress first, keeping what was cropped (it is a step of its own).
void leaveCrop(Editor *ed)
{
    if (!ed->cropItem.isEmpty()) ed->endCrop(true);
}

} // namespace

void DeckWindow::createCropActions()
{
    Editor *ed = m_ed;
    ed->cropSessions = true;   // one step for the whole crop; Enter or a click away ends it, Esc takes it back
    mk("pic.crop", tr("Crop"), "crop", QKeySequence(), [ed] {
        const QVector<PictureItem *> pictures = croppablePictures(ed);
        if (pictures.size() != 1) return;
        ed->setCropItem(ed->cropItem == pictures[0]->id ? QString() : pictures[0]->id);
    }, true);
    mk("pic.fill", tr("Fill"), "expand", QKeySequence(), [ed] {
        leaveCrop(ed);
        ed->forEachSelected(tr("Fill"), [ed](Item *it) {
            if (auto *p = dynamic_cast<PictureItem *>(it); p && !p->imageId.isEmpty()) p->fitImage(ed->doc()->imageSize(p->imageId), true);
        });
    });
    mk("pic.fit", tr("Fit"), "shrink", QKeySequence(), [ed] {
        leaveCrop(ed);
        ed->forEachSelected(tr("Fit"), [ed](Item *it) {
            if (auto *p = dynamic_cast<PictureItem *>(it); p && !p->imageId.isEmpty()) p->fitImage(ed->doc()->imageSize(p->imageId), false);
        });
    });
    mk("pic.clearCrop", tr("Clear Crop"), "eraser", QKeySequence(), [ed] {
        leaveCrop(ed);
        ed->forEachSelected(tr("Clear Crop"), [](Item *it) {
            if (auto *p = dynamic_cast<PictureItem *>(it)) clearPictureCrop(p);
        });
    });

    // Remove Background: the mode (strokes on the picture, with the tint where the background goes), and its four commands on their own tab.
    m_removeProgress = new RemoveBackgroundProgress(ed, this);
    mk("pic.removeBackground", tr("Remove Background"), "image-minus", QKeySequence(), [this, ed] {
        leaveCrop(ed);
        const QVector<PictureItem *> pictures = croppablePictures(ed);
        QString why;
        if (pictures.size() != 1 || !ed->beginRemoveBackground(pictures[0]->id, &why)) {
            statusBar()->showMessage(why.isEmpty() ? tr("Select a picture to remove its background.") : why, 8000);
            return;
        }
        m_ribbon->setContextVisible(QStringLiteral("Background Removal"), true, true);
        refreshCrop();
    });
    mk("pic.markKeep", tr("Mark Areas to Keep"), "circle-plus", QKeySequence(), [ed] { ed->setRemoveBgTool(Editor::RemoveBgTool::Keep); }, true);
    mk("pic.markRemove", tr("Mark Areas to Remove"), "circle-minus", QKeySequence(), [ed] { ed->setRemoveBgTool(Editor::RemoveBgTool::Remove); }, true);
    mk("pic.discardRemoval", tr("Discard All Changes"), "undo-2", QKeySequence(), [ed] { ed->endRemoveBackground(false); });
    mk("pic.keepRemoval", tr("Keep Changes"), "check", QKeySequence(), [ed] { ed->endRemoveBackground(true); });
    connect(ed, &Editor::viewChanged, this, [this, ed] {
        const bool on = !ed->removeBgItem.isEmpty();
        if (m_ribbon && !on) m_ribbon->setContextVisible(QStringLiteral("Background Removal"), false);   // (it is shown, and selected, by the command that begins the mode)
        act("pic.markKeep")->setChecked(on && ed->removeBgTool == Editor::RemoveBgTool::Keep);
        act("pic.markRemove")->setChecked(on && ed->removeBgTool == Editor::RemoveBgTool::Remove);
    });

    mk("pic.change", tr("Change Picture…"), "image-up", QKeySequence(), [this] { changePictureFromFile(); });
    mk("pic.reset", tr("Reset Picture"), "rotate-ccw", QKeySequence(), [this, ed] {
        leaveCrop(ed);
        resetSelectedPictures(ed, false);
    });
    mk("pic.resetSize", tr("Reset Picture & Size"), "rotate-ccw", QKeySequence(), [this, ed] {
        leaveCrop(ed);
        resetSelectedPictures(ed, true);
    });

    mk("pic.compress", tr("Compress Pictures…"), "minimize-2", QKeySequence(), [this] {
        compressPicturesDialog(this, m_ed, tr("Apply to all pictures in the presentation"));
    });

    // Corrections, Color, and Transparency (Picture Format > Adjust): each choice is one step of Undo for the pictures selected, drawn and saved as the Format pane's boxes are.
    auto signedPercent = [](int v) { return v > 0 ? QStringLiteral("+%1%").arg(v) : QStringLiteral("%1%").arg(v); };
    auto onPictures = [ed](const QString &step, const std::function<void(PictureItem &)> &change) {
        leaveCrop(ed);
        ed->forEachSelected(step, [change](Item *it) {
            if (auto *p = dynamic_cast<PictureItem *>(it); p && p->mediaKind.isEmpty() && p->chartXml.isEmpty() && !p->imageId.isEmpty()) change(*p);
        });
    };
    for (int v : {-40, -20, 0, 20, 40}) {
        mk(QStringLiteral("pic.brightness.%1").arg(v), v == 0 ? tr("Brightness: 0% (Normal)") : tr("Brightness: %1").arg(signedPercent(v)), QString(), QKeySequence(),
           [onPictures, v] { onPictures(tr("Brightness"), [v](PictureItem &p) { p.brightness = v; }); });
        mk(QStringLiteral("pic.contrast.%1").arg(v), v == 0 ? tr("Contrast: 0% (Normal)") : tr("Contrast: %1").arg(signedPercent(v)), QString(), QKeySequence(),
           [onPictures, v] { onPictures(tr("Contrast"), [v](PictureItem &p) { p.contrast = v; }); });
    }
    for (int v : {-100, -50, 0, 50, 100}) {
        mk(QStringLiteral("pic.saturation.%1").arg(v), v == 0 ? tr("Saturation: 0% (Normal)") : v == -100 ? tr("Saturation: -100% (Gray)") : tr("Saturation: %1").arg(signedPercent(v)), QString(),
           QKeySequence(), [onPictures, v] { onPictures(tr("Saturation"), [v](PictureItem &p) { p.setSaturationMove(v); }); });
    }
    // Sharpen and soften, and color temperature: the shared picture filters (libs/page/app/picturetools).
    for (const PictureFxCommand &c : pictureSharpenCommands() + pictureTemperatureCommands())
        mk(c.id, c.text, QString(), QKeySequence(), [this, ed, c] {
            leaveCrop(ed);
            changePicturesWithFilter(ed, this, c.step, c.apply);
        });
    const struct { const char *id; PictureItem::Recolor how; QString name; } recolors[] = {{"none", PictureItem::NoRecolor, tr("No Recolor")},
                                                                                           {"gray", PictureItem::Grayscale, tr("Grayscale")},
                                                                                           {"blackwhite", PictureItem::BlackWhite, tr("Black and White")}};
    for (const auto &r : recolors) {
        const PictureItem::Recolor how = r.how;
        mk(QStringLiteral("pic.recolor.%1").arg(QLatin1String(r.id)), r.name, QString(), QKeySequence(), [onPictures, how] { onPictures(tr("Recolor"), [how](PictureItem &p) { p.recolor = how; }); });
    }
    for (int v : {0, 15, 30, 50, 65, 80, 95})
        mk(QStringLiteral("pic.transparency.%1").arg(v), v == 0 ? tr("Transparency: 0% (Normal)") : tr("Transparency: %1%").arg(v), QString(), QKeySequence(),
           [onPictures, v] { onPictures(tr("Picture Transparency"), [v](PictureItem &p) { p.transparency = v / 100.0; }); });
    // Set Transparent Color picks a color from what the window shows; the pictures selected then have that color clear.
    mk("pic.transparent", tr("Set Transparent Color"), "pipette", QKeySequence(), [this] {
        QPointer<DeckWindow> self(this);
        statusBar()->showMessage(tr("Click the color in the picture that is to be clear."), 8000);
        pickPointFromWindow(this, [self](const QPoint &at) {
            if (self) self->setTransparentColorAt(at);
        });
    });
    mk("pic.transparentNone", tr("No Transparent Color"), QString(), QKeySequence(), [onPictures] {
        onPictures(tr("Transparent Color"), [](PictureItem &p) { p.hasTransparentColor = false; });
    });

    // Aspect Ratio: the shapes slides and screens come in, wide ones and tall ones.
    const struct { int w, h; QString name; } ratios[] = {
        {1, 1, tr("Square 1:1")}, {2, 3, tr("Portrait 2:3")}, {3, 4, tr("Portrait 3:4")}, {3, 5, tr("Portrait 3:5")}, {4, 5, tr("Portrait 4:5")},
        {9, 16, tr("Portrait 9:16")}, {3, 2, tr("Landscape 3:2")}, {4, 3, tr("Landscape 4:3")}, {5, 3, tr("Landscape 5:3")}, {5, 4, tr("Landscape 5:4")},
        {16, 9, tr("Landscape 16:9")}, {16, 10, tr("Landscape 16:10")}};
    for (const auto &r : ratios) {
        const int w = r.w, h = r.h;
        mk(QStringLiteral("pic.aspect.%1x%2").arg(w).arg(h), r.name, "", QKeySequence(), [ed, w, h] {
            leaveCrop(ed);
            ed->forEachSelected(tr("Aspect Ratio"), [ed, w, h](Item *it) {
                if (auto *p = dynamic_cast<PictureItem *>(it); p && !p->imageId.isEmpty()) cropPictureToAspect(p, w, h);
            });
        });
    }

    auto sync = [this] { refreshCrop(); };
    connect(m_ed, &Editor::selectionChanged, this, sync);
    connect(m_ed, &Editor::changed, this, sync);
    connect(m_ed, &Editor::viewChanged, this, sync);
    connect(m_ed, &Editor::pageChanged, this, sync);
    connect(m_ed, &Editor::documentReplaced, this, sync);
    refreshCrop();
}

void DeckWindow::cropPictureToShape(const QString &shape)
{
    leaveCrop(m_ed);
    m_ed->forEachSelected(tr("Crop to Shape"), [shape](Item *it) {
        if (auto *p = dynamic_cast<PictureItem *>(it); p && !p->imageId.isEmpty()) p->maskShape = shape;
    });
}

bool DeckWindow::setTransparentColorAt(const QPoint &windowPoint)
{
    const QPointF page = m_canvas->toPage(QPointF(m_canvas->viewport()->mapFrom(this, windowPoint)));
    const QString id = m_canvas->itemAt(page);
    const auto *pic = dynamic_cast<const PictureItem *>(m_ed->doc()->item(id));
    auto refuse = [this] {
        statusBar()->showMessage(tr("Click a color in a picture that is selected."), 6000);
        return false;
    };
    if (!pic || !m_ed->selection().contains(id) || pic->imageId.isEmpty() || !pic->mediaKind.isEmpty() || !pic->chartXml.isEmpty()) return refuse();
    // The point in the picture's own pixels: back through the object's turn and flip, then across the picture's frame (a crop moves it) to the pixel.
    const QImage source = m_ed->doc()->image(pic->imageId);
    const QPointF local = pic->transform().inverted().map(page);
    if (source.isNull() || pic->imgRect.width() <= 0 || pic->imgRect.height() <= 0) return refuse();
    const int x = int(std::floor((local.x() - pic->imgRect.x()) / pic->imgRect.width() * source.width()));
    const int y = int(std::floor((local.y() - pic->imgRect.y()) / pic->imgRect.height() * source.height()));
    if (x < 0 || y < 0 || x >= source.width() || y >= source.height()) return refuse();
    const QColor color = source.pixelColor(x, y);
    if (color.alpha() == 0) return refuse();
    setPictureTransparentColor(color);
    return true;
}

void DeckWindow::setPictureTransparentColor(const QColor &color)
{
    if (!color.isValid()) return;
    leaveCrop(m_ed);
    m_ed->forEachSelected(tr("Transparent Color"), [color](Item *it) {
        if (auto *p = dynamic_cast<PictureItem *>(it); p && p->mediaKind.isEmpty() && p->chartXml.isEmpty() && !p->imageId.isEmpty()) {
            p->hasTransparentColor = true;
            p->transparentColor = QColor(color.red(), color.green(), color.blue());
        }
    });
}

bool DeckWindow::changePictureFromFile(const QString &chosen)
{
    auto *pic = dynamic_cast<PictureItem *>(m_ed->single());
    if (!pic || pic->imageId.isEmpty() || !pic->mediaKind.isEmpty() || !pic->chartXml.isEmpty()) return false;
    QString path = chosen;
    if (path.isEmpty()) {
        const QString dir = Settings::get().value(QStringLiteral("dirs/pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
        const QString filter = tr("All Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.webp *.svg *.wmf *.emf *.ico *.pdf)") + QStringLiteral(";;") + tr("All Files (*)");
        const QStringList paths = askPicturePaths(this, tr("Change Picture"), dir, filter, false, nullptr);
        if (paths.isEmpty()) return false;
        Settings::get().setValue(QStringLiteral("dirs/pictures"), QFileInfo(paths.first()).absolutePath());
        path = paths.first();
    }
    QByteArray bytes = readPictureBytes(path);
    QString format;
    QSize pixels;
    bool asksNotToBeCopied = false;
    if (bytes.isEmpty() || !readPictureFileAsking(this, path, 0, &bytes, &format, &pixels, nullptr, &asksNotToBeCopied)) {
        statusBar()->showMessage(tr("%1 can't be read as a picture.").arg(QFileInfo(path).fileName()), 8000);
        return false;
    }
    leaveCrop(m_ed);
    const QString imageId = m_ed->doc()->addImage(bytes, format, path);
    if (imageId.isEmpty()) return false;
    return changePictureTo(m_ed, pic->id, imageId);
}

// The Picture Format tab shows while one picture is selected (a movie or sound has its own tab, Playback).
void DeckWindow::refreshCrop()
{
    const QVector<PictureItem *> pictures = croppablePictures(m_ed);
    const bool marking = !m_ed->removeBgItem.isEmpty();
    const bool shows = !marking && m_ed->selection().size() == 1 && pictures.size() == 1 && pictures[0]->mediaKind.isEmpty() && pictures[0]->chartXml.isEmpty();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Picture Tools"), shows);
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("pic.")) && a->objectName() != QLatin1String("pic.crop")) a->setEnabled(!pictures.isEmpty());
    // Change Picture and Reset Picture work on a picture of a file (not a movie, a sound, or a chart, which have no picture to change).
    bool plain = false;
    for (const PictureItem *p : pictures) plain = plain || (p->mediaKind.isEmpty() && p->chartXml.isEmpty());
    act("pic.change")->setEnabled(shows);
    act("pic.removeBackground")->setEnabled(shows && plain && m_ed->removeBgItem.isEmpty() && !m_ed->readOnly());   // (not in a presentation open for reading only)
    act("pic.reset")->setEnabled(plain);
    act("pic.resetSize")->setEnabled(plain);
    act("pic.crop")->setEnabled(shows);
    for (const char *id : {"pic.markKeep", "pic.markRemove", "pic.discardRemoval", "pic.keepRemoval"}) act(QString::fromLatin1(id))->setEnabled(marking);
    act("pic.crop")->setChecked(shows && m_ed->cropItem == pictures[0]->id);
}

void DeckWindow::addCropParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("pictureFormat")] = QColor(0x2F, 0x7D, 0x4F);
    parts.colors[QStringLiteral("table")] = QColor(0x8E, 0x2A, 0x5E);   // Table Design and Layout share it
    // Picture Styles: the gallery of looks, then Picture Border and Picture Effects, which are made again each time they open (the colors are the presentation's now).
    parts.widgets[QStringLiteral("pictureStyleGallery")] = [this]() -> QWidget * {
        auto *gal = new Gallery(QSize(60, 46), 6);
        gal->setObjectName(QStringLiteral("pictureStyleGallery"));
        gal->setTitle(tr("Picture Styles"));
        gal->setItemsProvider([this] { return pictureStyleItems(m_ed->doc()->colors); });
        connect(gal, &Gallery::activated, this, [this](const QString &id) {
            leaveCrop(m_ed);
            applyPictureStyleToSelection(m_ed, id.toInt());
            m_canvas->setFocus();
        });
        return gal;
    };
    parts.widgets[QStringLiteral("artisticEffectsButton")] = [this]() -> QWidget * {
        return artisticEffectsButton(
            [this] {
                const auto *p = dynamic_cast<const PictureItem *>(m_ed->single());
                return p && !p->imageId.isEmpty() ? m_ed->doc()->image(p->imageId) : QImage();
            },
            [this](int style) {
                leaveCrop(m_ed);
                applyArtisticEffect(m_ed, this, style);
            },
            QStringLiteral("artisticEffectsButton"));
    };
    parts.menus[QStringLiteral("pictureBorderMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, m, [this, m] { fillPictureBorderMenu(m, m_ed, this, [this] { formatSelection(FormatPane::Tab::FillLine); }); });
        return m;
    };
    parts.menus[QStringLiteral("pictureEffectsMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, m, [this, m] { fillPictureEffectsMenu(m, m_ed, [this] { formatSelection(FormatPane::Tab::Effects); }); });
        return m;
    };
    parts.launchers[QStringLiteral("pictureStyles")] = [this] { formatSelection(FormatPane::Tab::Picture); };
    // Crop to Shape: the closed shapes, as the Shapes gallery lists them, that a presentation file has by name (it saves the picture's shape as one of them).
    parts.widgets[QStringLiteral("cropToShapeButton")] = [this]() -> QWidget * { return cropToShapeButton([this](const QString &id) { cropPictureToShape(id); }, QStringLiteral("cropToShapeButton")); };
}

} // namespace jp
