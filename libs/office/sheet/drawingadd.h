#pragma once
// Putting new objects on a sheet in the file: the charts, pictures, and shapes a person made or pasted. A sheet's drawing part (xl/drawings/drawingN.xml) gets their anchors at its end (so
// they are drawn over what is there), the parts they relate to (a chart part, a picture) are added to the package with the relationships and content types that name them, and a sheet
// that had no drawing is given one, with the link from the sheet to it. Everything else in the file stays as it was. The bytes are changed with the scanner of xmlscan.h; saving
// (xlsxsave.cpp) puts the pieces together.

#include "sheet/drawing.h"

#include <QByteArray>
#include <QString>
#include <QSet>
#include <QStringList>
#include <functional>
#include <vector>

namespace jp::office::sheet {

// A part a new object relates to.
struct AddedPart {
    QString part;            // its name in the package ("/xl/charts/chart3.xml")
    QString relType;         // the last word of the relationship's type ("chart", "image")
    QString contentType;     // for a part the package does not have: its media type (empty: the package has the part, and only the relationship is made)
    QByteArray bytes;        // for a part the package does not have
    std::function<QByteArray()> produce;   // or the way to make them, run by the thread that writes the file (a chart is written there, not where the save began)
    QString token;           // what the object's XML has where the relationship's id goes; the id is put in its place
};

struct AddedObject {
    QByteArray xml;                   // the anchor element, whole (it declares the namespaces it uses)
    std::vector<AddedPart> parts;
};

// The new objects of one sheet, in the order they are drawn.
struct AddedDrawing {
    QString part;                     // the drawing part: the sheet's own if it has one, else the name the new one is made with
    std::vector<AddedObject> objects;
};

// The most that is added to one workbook by one save (a hostile paste is bounded before it gets here): objects, and the bytes of all the parts.
constexpr int kMaxAddedObjects = 1000;
constexpr qint64 kMaxAddedBytes = qint64(256) << 20;

// ---- the anchor of a chart (the element a drawing holds for a chart's frame): the places are `anchor`'s (always written as two cells, with what it does when cells are resized in editAs);
// `shapeId` is its number in the drawing, and the chart is the relationship `token` stands for.
QByteArray chartAnchorXml(const DrawingAnchor &anchor, int shapeId, const QString &name, const QString &token);

// The anchor of a picture, shape, group, or connector: the element (as bytes, with the pictures it names by the tokens that are put in its place) in a two-cell anchor, with the namespace
// declarations (`xmlns:...` attributes as written) it needs.
QByteArray objectAnchorXml(const DrawingAnchor &anchor, const QByteArray &namespaces, const QByteArray &elementXml);

// ---- the bytes of the parts
// The drawing part with the anchors (elements, as bytes) put at the end of its root; empty if the part has no root to put them in.
QByteArray drawingWithAnchors(const QByteArray &drawingXml, const QByteArray &anchors);
// A drawing part holding the anchors.
QByteArray newDrawingXml(const QByteArray &anchors);
// The ids a relationships part names.
QStringList relationshipIds(const QByteArray &relsXml);
// The first "rIdN" not among the ids.
QString freeRelationshipId(const QStringList &used);
// One Relationship element, as the part writes it.
QByteArray relationshipXml(const QString &id, const QString &type, const QString &target, bool external = false);
// The relationships part with one more relationship (made, if `relsXml` is empty). `type` is the last word of the type, or a whole type; `target` is as the part writes it.
bool addRelationship(const QByteArray &relsXml, const QString &id, const QString &type, const QString &target, QByteArray *out, bool external = false);
// The content types part with an override for the part, or a default for the extension (unless it has one); false if it is not a content types part.
bool addContentOverride(const QByteArray &typesXml, const QString &part, const QString &contentType, QByteArray *out);
bool addContentDefault(const QByteArray &typesXml, const QString &extension, const QString &contentType, QByteArray *out);
bool hasContentDefault(const QByteArray &typesXml, const QString &extension);
// The target of a relationship from `from` to `to` (both names in the package): "../charts/chart1.xml".
QString relativeTarget(const QString &from, const QString &to);
// A part name not among `taken` (compared without regard to case): "/xl/charts/chart" + N + ".xml", the first N from `first` up.
QString freePartName(const QString &prefix, const QString &suffix, const QSet<QString> &taken, int first = 1);

} // namespace jp::office::sheet
