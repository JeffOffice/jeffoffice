#pragma once
// What saving does to the drawing and chart parts of a workbook beyond moving them with the cells (structsave.h): taking away the objects a person deleted from a drawing part, and
// making again the caches of the charts whose cells changed (the numbers and texts a chart stores for the cells it reads, which a program that does not calculate shows). Both work on
// the part's bytes with the scanner of xmlscan.h, so everything else in the part stays as it was.

#include "sheet/chartdata.h"
#include "sheet/drawing.h"

#include <QByteArray>
#include <QDomElement>
#include <QString>

#include <map>
#include <vector>

namespace jp::office::sheet {

// The drawing part without the children of its root (anchors, or what wraps them) at the given places (from 0, as DrawingObject::ordinal counts them). `changed` says whether the output
// is not the input; false, with nothing read, if the part has no root element. `relIds`, if given, is told the relationship ids (the r:embed, r:link, r:id, and r:pict attributes) of what
// was taken out.
bool removeDrawingObjects(const QByteArray &xml, const std::vector<int> &ordinals, QByteArray *out, bool *changed, QStringList *relIds = nullptr);

// The most tags hiddenObjectsOf looks at in one drawing part (the rest of the part is not looked at, and the answer says so).
constexpr int kMaxHiddenScanTags = 4000000;

// The objects a drawing part hides (their name's element says hidden), found by reading its tags, not by building a tree, so that no object is skipped for its kind (a shape, a picture, a
// group, a line, a chart, a table, a slicer, or anything else with a name), or for being wrapped in an alternate-content element, or for being past the limit of objects that are read.
// An object that is a child of the root's child (or alone in the wrapper) is taken out whole with that child: its place is in `whole`. A hidden object inside a group that is not hidden is
// taken out alone: its place is in `nested`, with the child of the root it is in, and its bytes. `complete` is false when the part had more tags than `maxTags`: what comes after is not looked at.
struct HiddenInDrawing {
    struct Nested {
        int ordinal = 0;
        qsizetype begin = 0, end = 0;
    };
    std::vector<int> whole;
    std::vector<Nested> nested;
    bool complete = true;
};
HiddenInDrawing hiddenObjectsOf(const QByteArray &xml, int maxTags = 0);
// What `maxTags` is when it is 0 (for a test: a small number, then back to kMaxHiddenScanTags with 0).
void setHiddenScanLimit(int tags);

// The drawing part without the hidden objects that are inside groups that stay (the whole ones are left to removeDrawingObjects). `removed` is how many; `relIds` is told the relationship ids
// of what was taken out. `changed` says whether the output is not the input.
bool removeNestedHiddenObjects(const QByteArray &xml, QByteArray *out, bool *changed, QStringList *relIds = nullptr, int *removed = nullptr);

// The shapes of an old-style drawing part (the form that held pictures, text boxes, and controls before the drawing part above), not counting those that are a note's box: this program does not
// read them, so it can't say whether one is hidden. (At most `maxTags` tags are looked at; 0 is the limit above.)
int legacyDrawingShapes(const QByteArray &vml, int maxTags = 0);

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

// The same for the alternative text (the descr of the first cNvPr; left out when empty), and for the objects at the given places of a drawing.
QByteArray withFirstObjectDescription(const QByteArray &element, const QString &text);
bool setDrawingObjectDescriptions(const QByteArray &xml, const std::map<int, QString> &texts, QByteArray *out, bool *changed);

// A barcode's marker (office/barcodemarker.h) set in the first cNvPr of an object's element, or of the objects at the given places of a drawing: `extXml` is the marker's a:ext. Every other byte stays.
QByteArray withFirstObjectBarcodeMarker(const QByteArray &element, const QByteArray &extXml);
bool setDrawingObjectBarcodeMarkers(const QByteArray &xml, const std::map<int, QByteArray> &markers, QByteArray *out, bool *changed);

// A shape or text box that shows a cell's text: the textlink attribute of its xdr:sp holds the reference (one cell, or a name), and its text body the text it had when the file was saved.
constexpr int kMaxShapeLinkChars = 255;     // a reference longer than this is not a link
constexpr int kMaxShapeLinkText = 32767;    // the text shown is cut to what a cell holds
// Whether `ref` can be a link: not empty, at most kMaxShapeLinkChars, no control characters, and one reference (no list and no range). Whether it names a cell is for the workbook to say.
bool validShapeLink(const QString &ref);
// The text as a shape holds it: line breaks as \n, cut to kMaxShapeLinkText, and without the characters XML cannot hold (as the cell writer has them).
QString normalizedShapeLinkText(const QString &text);
// The shape element's text body made to hold `text` (cut to kMaxShapeLinkText): one paragraph for each line, in the first paragraph's and first run's look; the body is made if the shape has
// none. `changed` says whether the words were other than they were (a body that already holds the words is not touched).
void setShapeLinkText(QDomElement sp, const QString &text, bool *changed = nullptr);
// The words of a shape element's text body (paragraphs joined by a line break).
QString shapeLinkStoredText(const QDomElement &sp);
// What is written for a linked shape at a place of a drawing: the reference (empty: no link), and the text if `withText`.
struct TextLinkWrite {
    QString ref, text;
    bool withText = false;
};
// An object's element with its first shape's textlink set (left out when `ref` is empty) and, if `withText`, its text made `text`. The element is returned as it was if it has no shape.
QByteArray withFirstObjectTextLink(const QByteArray &element, const QString &ref, const QString &text, bool withText);
// The same for the objects at the given places of a drawing; an object whose shape already says so is left byte for byte.
bool setDrawingObjectTextLinks(const QByteArray &xml, const std::map<int, TextLinkWrite> &links, QByteArray *out, bool *changed);

// Whether the objects at the given places print: the fPrintsWithSheet of the first clientData of each written as "0" (does not print) or left out (prints). Every other byte stays.
bool setDrawingObjectPrints(const QByteArray &xml, const std::map<int, bool> &prints, QByteArray *out, bool *changed);
// An object's element (or several) with its first clientData set so.
QByteArray withFirstObjectPrints(const QByteArray &element, bool prints);
// An object's element (or several) with the number of its first cNvPr set to `id`.
QByteArray withFirstObjectId(const QByteArray &element, qint64 id);

// The child of the drawing's root at `ordinal` (as DrawingObject::ordinal counts), whole, with the namespace declarations of the root it does not have itself added to its first tag, so that
// it can be put in another drawing. Empty if there is no such child.
QByteArray drawingChildXml(const QByteArray &xml, int ordinal);

// The references a chart part reads (the text of each c:f), in order, as written (unescaped).
QStringList chartPartRefs(const QByteArray &xml);

// What a chart part stores for the references it reads, by the reference as the part writes it (its c:f): the cells' contents now.
using ChartCaches = std::map<QString, ChartRefData>;

// The chart part with the cache of each c:numRef and c:strRef that is in `caches` made from the cells' contents (a number cache keeps the format code it has; a reference that has no cache is
// given one). A cache that already holds what the cells do (the same points, numbers equal to 15 digits) is left as it was, so a part is changed only for what changed. A reference of several
// levels (c:multiLvlStrRef) is not touched. `changed` says whether the output is not the input.
bool refreshChartCaches(const QByteArray &xml, const ChartCaches &caches, QByteArray *out, bool *changed);

} // namespace jp::office::sheet
