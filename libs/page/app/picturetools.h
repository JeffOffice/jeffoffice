#pragma once
// The Picture Format tab's Picture Styles, Picture Border, Picture Effects, Change Picture, and Reset Picture, for the apps that edit pictures as objects of a
// page or a slide: the gallery's tiles (a sample picture drawn by the page renderer in each style), the menus, and the commands, each one step of Undo. The
// looks themselves are core/picturestyles.h's, and the effect presets formatpresets.h's.

#include "app/editor.h"
#include "app/infopage.h"
#include "app/widgets.h"
#include "core/style.h"

#include <QMenu>
#include <QImage>
#include <atomic>
#include <functional>

namespace jp {

// The gallery's tiles, one for each style (the id is the style's number, as text) with the style's name for a tip; drawn in the colors of `scheme`.
// With `sample` (not null), the tiles show that picture in each style, fitted to the sample picture's own shape, in place of the gallery's landscape.
QVector<GalleryItem> pictureStyleItems(const ColorScheme &scheme, const QSize &tile = QSize(60, 46), const QImage &sample = QImage());
// One style's tile: the sample picture of the gallery in that style (-1 for the picture as it comes), or `sample` where it is given.
QPixmap pictureStylePixmap(int k, const ColorScheme &scheme, const QSize &tile, const QImage &sample = QImage());

// The selected pictures (those that hold a picture, not a movie or a chart) take the style, as one step.
void applyPictureStyleToSelection(Editor *ed, int k);
// Reset Picture (and, with `andSize`, Reset Picture & Size) on the selected pictures, as one step; false when none is selected.
bool resetSelectedPictures(Editor *ed, bool andSize);
// Change Picture: the picture `pictureId` shows the picture of `imageId` (already in the document), keeping its size, place, crop, and formatting; one step.
bool changePictureTo(Editor *ed, const QString &pictureId, const QString &imageId);

// Picture Border: colors, No Outline, more colors, Weight, and Dashes. `options` (if set) is called by More Border Options.
void fillPictureBorderMenu(QMenu *menu, Editor *ed, QWidget *dialogParent, const std::function<void()> &options = {});
// Picture Effects: Shadow, Reflection, Glow, Soft Edges, Bevel, and 3-D Rotation, each with its presets and a "No" entry first. `options` (if set) is
// called by the Options entry at the bottom.
void fillPictureEffectsMenu(QMenu *menu, Editor *ed, const std::function<void()> &options = {});
// The same menu for an app that keeps its pictures its own way: each entry calls `apply` with the command's name and what it does to the picture's effects.
using EffectChange = std::function<void(Effects &)>;
void fillPictureEffectsMenu(QMenu *menu, const std::function<void(const QString &label, const EffectChange &change)> &apply, const std::function<void()> &options = {});

// Sharpen and soften, color temperature, and the artistic effects (core/picturefx.h): the same commands, names, and steps in every app that edits pictures as objects.
// Each command changes the selected pictures through `apply`, as one step called `step`.
struct PictureFxCommand {
    QString id;      // "pic.sharpen.-50", "pic.temperature.50"
    QString text;    // the menu's entry
    QString step;    // the Undo step
    std::function<void(PictureItem &)> apply;
};
QVector<PictureFxCommand> pictureSharpenCommands();
QVector<PictureFxCommand> pictureTemperatureCommands();
// The artistic effects of the gallery, each with a thumbnail of `sample` (a gray sample picture when it is null) in that effect; the first is "none", whose id is "-1" (the others' ids are the style's number).
QVector<GalleryItem> artisticEffectItems(const QImage &sample, const QSize &tile = QSize(48, 36));
// The Artistic Effects button for a Picture Format tab: a gallery of the effects, shown on the picture selected (`sample` gives it, as it is now), and `chosen` called with the effect's id.
GalleryButton *artisticEffectsButton(const std::function<QImage()> &sample, const std::function<void(int style)> &chosen, const QString &objectName = QString());
// The effect of `style` (-1 for none) on the selected pictures, as one step.
void applyArtisticEffect(Editor *ed, QWidget *window, int style);
// Changes the selected pictures that hold a picture (not a movie or a chart) with `change` as one step `step`. A filter that would take longer than half a second is worked out first on a
// thread of its own, with a box that can cancel it (nothing changes then), so the window is not held; the picture is drawn from what was worked out, never worked out again.
void changePicturesWithFilter(Editor *ed, QWidget *window, const QString &step, const std::function<void(PictureItem &)> &change);
// How many times changePicturesWithFilter worked on a thread of its own (for tests).
int pictureFxThreadedRuns();
// Called at the start of the work on its thread, with the flag the Cancel button sets (for tests, as Editor::removeBgWorkerHook is).
extern std::function<void(const std::atomic<bool> &)> pictureFxWorkerHook;

// What Inspect finds beside the finished look of pictures (every page, master, and the scratch area): the original that stays in the file under a picture effect, the whole picture
// under a removed background, and the parts cropped out of sight. The rows count them and take them out; a picture then keeps only what shows, as a picture of its own (Reset Picture
// and Remove Background can't bring the rest back). One step of Undo for each.
enum class PictureKept { FilterOriginals, RemovedBackgrounds, CroppedAreas };
int picturesKeeping(const Document &doc, PictureKept kind);
int removeKeptPictureParts(Editor *ed, PictureKept kind);
// How many pictures have a crop that the cropped-parts row leaves as it is: one drawn smaller than it is (it would be drawn another size after a cut), or one that holds a picture with another.
int picturesCroppedAndLeft(const Document &doc);
// How many drawings and linked pictures have what the row is about (an effect, a removed background, a crop) and cannot be changed by it.
int picturesNotChanged(const Document &doc, PictureKept kind);
// How many pictures have filters left out (more than can be worked out on the picture: only a file can have them).
int picturesWithEffectsLeftOut(const Document &doc);
QVector<InspectItem> pictureInspectItems(Editor *ed);

// Crop to Shape's gallery: the closed shapes of the Shapes gallery that a file keeps by a preset name (the id is the shape's id: office::presetForShape gives its preset).
QVector<GalleryItem> cropShapeItems();
// The Crop to Shape button for a Picture Format tab: the gallery above, and `chosen` called with the shape's id.
GalleryButton *cropToShapeButton(const std::function<void(const QString &shapeId)> &chosen, const QString &objectName = QString());

} // namespace jp
