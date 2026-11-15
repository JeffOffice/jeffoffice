// The Picture Format tab: Crop (with the picture's handles), Crop to Shape, Aspect Ratio, Fill, Fit, and Clear Crop. The commands that change the
// frame or the picture inside it are libs/page's (app/pictureedit.h, the canvas's crop handles), the ones JeffPub's Crop uses too.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/pictureedit.h"
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
    const bool shows = m_ed->selection().size() == 1 && pictures.size() == 1 && pictures[0]->mediaKind.isEmpty() && pictures[0]->chartXml.isEmpty();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Picture Tools"), shows);
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("pic.")) && a->objectName() != QLatin1String("pic.crop")) a->setEnabled(!pictures.isEmpty());
    // Change Picture and Reset Picture work on a picture of a file (not a movie, a sound, or a chart, which have no picture to change).
    bool plain = false;
    for (const PictureItem *p : pictures) plain = plain || (p->mediaKind.isEmpty() && p->chartXml.isEmpty());
    act("pic.change")->setEnabled(shows);
    act("pic.reset")->setEnabled(plain);
    act("pic.resetSize")->setEnabled(plain);
    act("pic.crop")->setEnabled(shows);
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
    parts.widgets[QStringLiteral("cropToShapeButton")] = [this]() -> QWidget * {
        auto *b = new GalleryButton(icon("shapes"), tr("Crop to Shape"), QSize(24, 24), 12, true);
        b->setObjectName(QStringLiteral("cropToShapeButton"));
        b->setItemsProvider([] {
            QVector<GalleryItem> v;
            for (const auto &s : shapeLibrary())
                if (!s.open && !office::presetForShape(s.id).isEmpty()) v << GalleryItem{s.id, s.name, shapeIcon(s.id), s.category};
            return v;
        });
        connect(b, &GalleryButton::activated, this, [this](const QString &id) { cropPictureToShape(id); });
        return b;
    };
}

} // namespace jp
