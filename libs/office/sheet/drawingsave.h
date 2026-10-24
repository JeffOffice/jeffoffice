#pragma once
// What saving does to the drawing and chart parts of a workbook beyond moving them with the cells (structsave.h): taking away the objects a person deleted from a drawing part, and
// making again the caches of the charts whose cells changed (the numbers and texts a chart stores for the cells it reads, which a program that does not calculate shows). Both work on
// the part's bytes with the scanner of xmlscan.h, so everything else in the part stays as it was.

#include "sheet/chartdata.h"

#include <QByteArray>
#include <QString>

#include <map>
#include <vector>

namespace jp::office::sheet {

// The drawing part without the children of its root (anchors, or what wraps them) at the given places (from 0, as DrawingObject::ordinal counts them). `changed` says whether the output
// is not the input; false, with nothing read, if the part has no root element.
bool removeDrawingObjects(const QByteArray &xml, const std::vector<int> &ordinals, QByteArray *out, bool *changed);

// What a chart part stores for the references it reads, by the reference as the part writes it (its c:f): the cells' contents now.
using ChartCaches = std::map<QString, ChartRefData>;

// The chart part with the cache of each c:numRef and c:strRef that is in `caches` made from the cells' contents (a number cache keeps the format code it has; a reference that has no cache is
// given one). A cache that already holds what the cells do (the same points, numbers equal to 15 digits) is left as it was, so a part is changed only for what changed. A reference of several
// levels (c:multiLvlStrRef) is not touched. `changed` says whether the output is not the input.
bool refreshChartCaches(const QByteArray &xml, const ChartCaches &caches, QByteArray *out, bool *changed);

} // namespace jp::office::sheet
