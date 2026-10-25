#pragma once
// The drawing objects of a sheet: the charts, pictures, shapes, text boxes, and groups of its drawing part (xl/drawings/drawingN.xml, which the sheet's own relationships name; a
// chart sheet has one too). Each anchor of the part holds one object and says where it is: between two cells (the usual: it moves and sizes with them), at a cell with a size of its
// own, or at a place of the sheet. This reads the anchors and finds what each holds; what a shape or picture looks like is read by libs/office/office/drawingml.h from the object's
// element, and a chart by libs/office/office/chart.h from the part it names. The element stays in the document this keeps.
//
// The part comes from a stranger: it is parsed as every part is (xml.h's limits), the anchors read are at most kMaxDrawingAnchors, and the position numbers are kept to what a
// sheet can hold, so that an anchor far outside the sheet is a place far outside it and nothing more.

#include "office/package.h"
#include "sheet/refshift.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QVector>

#include <memory>
#include <vector>

namespace jp::office::sheet {

// The anchors read from one drawing part, at most (the largest real one has 640), and the objects all the drawings of a workbook may have between them (the largest has 1,889 shapes).
constexpr int kMaxDrawingAnchors = 10000;
constexpr int kMaxWorkbookObjects = 50000;
// The most a position or size may be, in EMU (12,700 to the point): 86 million points, far beyond any object (a sheet of 1,048,576 rows of 15 points is 16 million). A larger number is kept to it.
constexpr qint64 kMaxDrawingEmu = qint64(1) << 40;
// The pixels (at 96 to the inch) in an EMU: 9,525 of them make one.
constexpr qint64 kEmuPerPixel = 9525;

enum class AnchorKind : quint8 { TwoCell, OneCell, Absolute };

// A place in a cell: the cell (from 0) and how far into it, in EMU.
struct AnchorPoint {
    quint32 col = 0, row = 0;
    qint64 colOff = 0, rowOff = 0;
};

struct DrawingAnchor {
    AnchorKind kind = AnchorKind::TwoCell;   // how the anchor is written
    AnchorKind editAs = AnchorKind::TwoCell; // what the object does when cells move or change size: moves and sizes with them (TwoCell), moves only (OneCell), or stays (Absolute)
    AnchorPoint from, to;                    // `to` for a two-cell anchor
    qint64 x = 0, y = 0;                     // an absolute anchor's place, in EMU from the sheet's top left
    qint64 cx = 0, cy = 0;                   // the size of a one-cell or absolute anchor, in EMU
};

enum class ObjectKind : quint8 { Chart, Picture, Shape, Group, Connector, Other };

struct DrawingObject {
    int index = 0;                 // the anchor's place among those read, from 0
    int ordinal = 0;               // the place of the element it is in among the children of the part's root, from 0 (saving removes an object by it)
    DrawingAnchor anchor;
    ObjectKind kind = ObjectKind::Other;
    QString name, description;
    qint64 id = 0;
    bool hidden = false;
    bool prints = true;            // the anchor does not say fPrintsWithSheet="0": the object is on the printed page
    QString chartPart;             // for a chart: the chart part's name
    QDomElement element;           // the object's element (xdr:sp, xdr:pic, xdr:grpSp, xdr:cxnSp, or the graphic frame); lives as long as the SheetDrawing's document
};

struct SheetDrawing {
    QString part;                          // the drawing part's name
    std::shared_ptr<QDomDocument> document;
    QVector<DrawingObject> objects;        // in the order of the file (which is the order they are drawn in, the first at the back)
    int anchorsInPart = 0;                 // anchors the part holds (the objects are those read, up to the limit)
    bool truncated = false;                // more anchors than kMaxDrawingAnchors: the rest were left out
    int skipped = 0;                       // anchors that hold nothing that is read (a form control, an ink stroke, a table or slicer)
    QString problem;                       // what is wrong with the part, if it could not be read
};

// The anchor after the edits that insert and delete rows and columns of its sheet (`sheet`, counted from 0), moved as saving moves the drawing part: a two-cell anchor moves and sizes
// with the cells (each end is where its cell went), unless it says editAs "oneCell" (the first end moves, the second keeps its distance from it) or "absolute" (it stays); a one-cell
// anchor moves; an absolute one stays.
DrawingAnchor movedAnchor(const DrawingAnchor &anchor, int sheet, const std::vector<StructEdit> &edits);

// The name of a sheet's drawing part, by the sheet part's relationships; empty if the sheet has none (or the part is not in the package).
QString drawingPartOf(const Package &package, const QString &sheetPart);

// Parses the bytes of a drawing part (the first half of readDrawing, which needs no package: it may be done on another thread, and the document given to readDrawing). Null, with a
// message in `error`, if they are damaged XML, have a DOCTYPE, or are past the limits of xml.h.
std::shared_ptr<QDomDocument> parseDrawing(const QByteArray &xml, QString *error = nullptr);

// Reads the drawing part (from `parsed`, if the bytes were parsed already). False, with a message in `error`, if it is not a drawing (damaged XML, a DOCTYPE, or past the limits of xml.h);
// `out` is then empty.
bool readDrawing(const Package &package, const QString &drawingPart, SheetDrawing *out, QString *error = nullptr, const std::shared_ptr<QDomDocument> &parsed = nullptr);

} // namespace jp::office::sheet
