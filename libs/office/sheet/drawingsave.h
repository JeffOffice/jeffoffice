#pragma once
// What saving does to the drawing and chart parts of a workbook beyond moving them with the cells (structsave.h): taking away the objects a person deleted from a drawing part, and
// making again the caches of the charts whose cells changed (the numbers and texts a chart stores for the cells it reads, which a program that does not calculate shows). Both work on
// the part's bytes with the scanner of xmlscan.h, so everything else in the part stays as it was.

#include "sheet/chartdata.h"
#include "sheet/drawing.h"

#include <QByteArray>
#include <QString>

#include <map>
#include <vector>

namespace jp::office::sheet {

// The drawing part without the children of its root (anchors, or what wraps them) at the given places (from 0, as DrawingObject::ordinal counts them). `changed` says whether the output
// is not the input; false, with nothing read, if the part has no root element. `relIds`, if given, is told the relationship ids (the r:embed, r:link, r:id, and r:pict attributes) of what
// was taken out.
bool removeDrawingObjects(const QByteArray &xml, const std::vector<int> &ordinals, QByteArray *out, bool *changed, QStringList *relIds = nullptr);

// The relationship ids (the values of r:embed, r:link, r:id, and r:pict attributes) a part names.
QStringList namedRelationships(const QByteArray &xml);

// The drawing part with the children of its root in another order: the child at place k of the new part is the one `order[k]` of the part (a permutation of all the children, counted as
// DrawingObject::ordinal counts them); what lies between the children (white space) stays where it is. `changed` says whether the output is not the input; false if the part has no root, or
// the order is not a permutation of its children.
bool reorderDrawingChildren(const QByteArray &xml, const std::vector<int> &order, QByteArray *out, bool *changed);

// The two-cell anchors at the given places (as DrawingObject::ordinal counts them; the first anchor of a wrapper) given the places `from` and `to` say, as the part writes them; every other byte
// stays. A place that is not of a two-cell anchor is left as it is.
bool setDrawingAnchors(const QByteArray &xml, const std::map<int, DrawingAnchor> &anchors, QByteArray *out, bool *changed);

// The name of the objects at the given places (as DrawingObject::ordinal counts them) written as given: the name attribute of the first cNvPr of each (its own, not a member's of a group). Every
// other byte stays.
bool setDrawingObjectNames(const QByteArray &xml, const std::map<int, QString> &names, QByteArray *out, bool *changed);

// An object's element (or several) with the name of its first cNvPr set to `name`.
QByteArray withFirstObjectName(const QByteArray &element, const QString &name);

// The references a chart part reads (the text of each c:f), in order, as written (unescaped).
QStringList chartPartRefs(const QByteArray &xml);

// What a chart part stores for the references it reads, by the reference as the part writes it (its c:f): the cells' contents now.
using ChartCaches = std::map<QString, ChartRefData>;

// The chart part with the cache of each c:numRef and c:strRef that is in `caches` made from the cells' contents (a number cache keeps the format code it has; a reference that has no cache is
// given one). A cache that already holds what the cells do (the same points, numbers equal to 15 digits) is left as it was, so a part is changed only for what changed. A reference of several
// levels (c:multiLvlStrRef) is not touched. `changed` says whether the output is not the input.
bool refreshChartCaches(const QByteArray &xml, const ChartCaches &caches, QByteArray *out, bool *changed);

} // namespace jp::office::sheet
