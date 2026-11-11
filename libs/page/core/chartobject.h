#pragma once
// A chart on a page or a slide: a picture (drawn by the shared chart drawing, as SVG, so it is vector in a PDF and on paper) that also holds the chart part it was drawn from
// (PictureItem::chartXml), so the chart can be changed (its kind, titles, legend, axes, style, and data) and saved as a chart. Making one, changing one, and drawing it again when its
// frame has been sized anew are here; the presentation and publishing programs call them, and the window's controls are the editor's (app/chartedit.h).

#include "core/document.h"
#include "core/items.h"
#include "office/chart.h"
#include "office/theme.h"
#include "sheet/chartops.h"

#include <QByteArray>
#include <QRectF>
#include <QSizeF>
#include <functional>

namespace jp {

// The most series and points a chart is kept as a chart with (more is kept as the picture it was drawn as): the limits of the charts a file is read with.
constexpr int kMostChartObjectSeries = 255;
constexpr qint64 kMostChartObjectPoints = 400000;

// The theme a chart is colored by in this document: the document's colors in the slots they stand for.
office::Theme chartThemeOf(const Document &doc);
// The look a chart read from a file is read with (PictureItem::chartLook): the theme's colors and fonts and the color map of the master it is on.
QJsonObject chartLookOf(const office::Theme &theme, const office::ColorMap &colors);
// The theme a picture's chart is read and made with: its own look when a file gave one, else the document's colors.
office::Theme chartThemeOfPicture(const Document &doc, const PictureItem &pic);
// Whether the chart is within what a chart object is kept as (series and points).
bool chartIsWithinLimits(const office::Chart &chart);
// The chart a picture holds; false for a picture that is no chart or whose part can't be read or is past the limits.
bool readChartOf(const Document &doc, const PictureItem &pic, office::Chart *chart);
// Draws the chart at `size` as the picture's image (its frame and cropping are set to it), and keeps its part. False, the picture as it was, if it can't be drawn.
bool setChartOf(Document &doc, PictureItem &pic, const office::Chart &chart, const QSizeF &size);
// A new chart object of the kind at `rect` (points), with the data; null if the data can't be a chart.
std::shared_ptr<PictureItem> makeChartObject(Document &doc, office::sheet::ChartForm form, const office::sheet::ChartData &data, const QRectF &rect);
// What the chart of a picture says of itself (not valid for no chart).
office::sheet::ChartInfo chartInfoOfPicture(const Document &doc, const PictureItem &pic);
// How many times a chart was read to say what it is (not found among the ones already said), on this thread since the program began: a count for tests of cost.
qint64 chartInfosMadeForTest();
// Changes the chart of the picture: `edit` changes the model and says whether it did; the picture is drawn again and keeps the new part. False if nothing changed.
bool changeChartOf(Document &doc, PictureItem &pic, const std::function<bool(office::Chart &)> &edit);
// Draws again the charts (on every page, master, and the scratch area) whose frames are another size than they were drawn at. True if any was.
bool redrawResizedCharts(Document &doc);
// Takes the chart part off every picture past what a file holds (the picture stays): the most charts, and the most bytes of one and of all. For a document just read.
// Returns how many were taken off.
int limitChartsRead(Document &doc);
// How many pictures of the document hold a chart.
int chartObjectsIn(const Document &doc);

} // namespace jp
