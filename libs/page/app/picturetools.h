#pragma once
// The Picture Format tab's Picture Styles, Picture Border, Picture Effects, Change Picture, and Reset Picture, for the apps that edit pictures as objects of a
// page or a slide: the gallery's tiles (a sample picture drawn by the page renderer in each style), the menus, and the commands, each one step of Undo. The
// looks themselves are core/picturestyles.h's, and the effect presets formatpresets.h's.

#include "app/editor.h"
#include "app/widgets.h"
#include "core/style.h"

#include <QMenu>
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

// Crop to Shape's gallery: the closed shapes of the Shapes gallery that a file keeps by a preset name (the id is the shape's id: office::presetForShape gives its preset).
QVector<GalleryItem> cropShapeItems();
// The Crop to Shape button for a Picture Format tab: the gallery above, and `chosen` called with the shape's id.
GalleryButton *cropToShapeButton(const std::function<void(const QString &shapeId)> &chosen, const QString &objectName = QString());

} // namespace jp
