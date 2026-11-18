#pragma once
// The drawing objects of a sheet: the charts, pictures, shapes, text boxes, and groups of its drawing part (xl/drawings/drawingN.xml, which the sheet's own relationships name; a
// chart sheet has one too). Each anchor of the part holds one object and says where it is: between two cells (the usual: it moves and sizes with them), at a cell with a size of its
// own, or at a place of the sheet. This reads the anchors and finds what each holds; what a shape or picture looks like is read by libs/office/office/drawingml.h from the object's
// element, and a chart by libs/office/office/chart.h from the part it names. The element stays in the document this keeps.
//
// The part comes from a stranger: it is parsed as every part is (xml.h's limits), the anchors read are at most kMaxDrawingAnchors, and the position numbers are kept to what a
// sheet can hold, so that an anchor far outside the sheet is a place far outside it and nothing more.

#include "ole/embedobj.h"
#include "sheet/refshift.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QVector>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace jp::office { class Package; }

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

enum class ObjectKind : quint8 { Chart, Picture, Shape, Group, Connector, Diagram, Other };

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
    QString diagramData;           // for a diagram: the id of the drawing part's link to the diagram's data part, which names the drawing the file keeps of it (libs/office/office/diagram.h)
    qint64 diagramCx = 0, diagramCy = 0;   // for a diagram: the size its frame says (the frame of a spreadsheet's diagram says 0 by 0: the anchor gives the size)
    QDomElement element;           // the object's element (xdr:sp, xdr:pic, xdr:grpSp, xdr:cxnSp, or the graphic frame); lives as long as the SheetDrawing's document
};

struct SheetDrawing {
    QString part;                          // the drawing part's name
    std::shared_ptr<QDomDocument> document;
    QVector<DrawingObject> objects;        // in the order of the file (which is the order they are drawn in, the first at the back)
    int anchorsInPart = 0;                 // anchors the part holds (the objects are those read, up to the limit)
    int rootChildren = 0;                  // elements the part's root holds (an object's `ordinal` counts among them; one added at the end has the next)
    bool truncated = false;                // more anchors than kMaxDrawingAnchors: the rest were left out
    int skipped = 0;                       // anchors that hold nothing that is read (a form control, an ink stroke, a table or slicer)
    QString problem;                       // what is wrong with the part, if it could not be read
};

// The anchor after the edits that insert and delete rows and columns of its sheet (`sheet`, counted from 0), moved as saving moves the drawing part: a two-cell anchor moves and sizes
// with the cells (each end is where its cell went), unless it says editAs "oneCell" (the first end moves, the second keeps its distance from it) or "absolute" (it stays); a one-cell
// anchor moves; an absolute one stays.
DrawingAnchor movedAnchor(const DrawingAnchor &anchor, int sheet, const std::vector<StructEdit> &edits);

// Whether the anchor's first cell is still there after the edits (a row or column that holds it was not deleted); an absolute anchor always is.
bool anchorSurvives(const DrawingAnchor &anchor, int sheet, const std::vector<StructEdit> &edits);

// An equation of a drawing: an object whose text is math that can be laid out (a text box with a14:m, the form a spreadsheet keeps equations in).
struct SheetEquation {
    DrawingAnchor anchor;
    QString name, description;
    QByteArray markup;   // m:oMathPara or m:oMath, declaring its namespace
};
// The equations of a drawing part, in the order they are drawn (at most kMaxDrawingEquations); `objects`, if given, is the number of objects the drawing holds in all.
constexpr int kMaxDrawingEquations = 500;
QVector<SheetEquation> readEquations(const Package &package, const QString &drawingPart, int *objects = nullptr);

// ---- embedded objects (a sheet's `oleObjects`: each `oleObject` names the program, the package or other bytes the object holds by a relationship of the sheet, and in `objectPr` its
// picture by another and its place by an `anchor` of the drawing's kind of cell point). They are not in the drawing part: the sheet's own XML has them. An object of ours is a safe
// package of one of our apps (ole::EmbeddedObject::isOurs) and may be put in and changed; any other is kept as the file has it, element and parts, byte for byte.
//
// The file is a stranger's: only a relationship of the kind an object has (oleObject or package) to a part under /xl/embeddings/ names an object; a picture is read only through an image
// relationship, and only if it is a PNG within the caps; the size of a part is asked of the package before it is inflated, and a part many objects name is inflated once for all the sheets
// (OleReadState). The objects of the sheets of a workbook are counted by one ole::ObjectBudget, and no more than kMaxOleChildren elements of one sheet are looked at.
constexpr int kMaxOleChildren = 10000;

// One child of a sheet's `oleObjects`, as the file has it (an `oleObject`, or the `AlternateContent` that holds one): where it is in the sheet's bytes, and what it names.
struct OleChild {
    qsizetype begin = 0, end = 0;   // its bytes in the sheet's XML
    QString progId;
    QString objectRel, pictureRel;  // the ids of the relationships of the sheet that name the object's bytes and its picture
    qint64 shapeId = 0;
    bool hasAnchor = false;
    DrawingAnchor anchor;           // from the `anchor` of its `objectPr` (a two-cell anchor)
};
// The children of the sheet's `oleObjects` in order (at most kMaxOleChildren), from the bytes that follow the sheet's cells (the elements of the worksheet itself, from the end of `sheetData`
// on); their places are in those bytes. `truncated` says there were more.
std::vector<OleChild> oleChildrenOf(const QByteArray &sheetXml, bool *truncated = nullptr);
// The same for a sheet's part: it is read as a stream and only what follows its cells is kept (a sheet of hundreds of megabytes is not held), so the places are in that tail, not in the part.
// False if the part is not there, is not read to its end, or its tail is past 64 MB.
bool readSheetOleChildren(const Package &package, const QString &sheetPart, std::vector<OleChild> *out);

struct SheetOleObject {
    ole::EmbeddedObject object;     // `origin` is the part the object's bytes are in
    DrawingAnchor anchor;
    int fileIndex = -1;             // the child's place among the sheet's `oleObjects` in the file (-1: one that is not in the file yet)
    qint64 shapeId = 0;
};

// What reading the objects of the sheets of a workbook keeps between sheets: the count and bytes so far, the parts already inflated, and the parts refused (too large, or over the caps).
struct OleReadState {
    ole::ObjectBudget budget;
    std::map<QString, QByteArray> parts;
    std::set<QString> refused;
    int pastLimit = 0;              // objects that name a part that is there but were not kept (over a cap): they stay in the file, as the file has them
    // The size an anchor of two cells gives an object, in EMU (the default: a column of 609,600 and a row of 190,500, which a caller that knows the sheet's sizes replaces).
    std::function<void(const DrawingAnchor &, qint64 *, qint64 *)> measure;
};

// The objects of a sheet that can be kept (the others stay in the file as they are). False if the sheet's part is not there or is not XML.
bool readSheetOleObjects(const Package &package, const QString &sheetPart, OleReadState *state, std::vector<SheetOleObject> *out);

// The objects that fit what is left of a budget, from copies of a sheet's objects: the later ones are checked after one that does not fit.
std::vector<SheetOleObject> oleObjectsThatFit(const std::vector<SheetOleObject> &objects, ole::ObjectBudget *budget);

// What a person changed in a sheet's objects, by the place of the children in the file: those taken away, those given new bytes and a new picture (their place and size stay), and new ones.
struct OleObjectEdit {
    struct Add {
        ole::EmbeddedObject object;
        DrawingAnchor anchor;
    };
    std::set<int> remove;
    std::map<int, ole::EmbeddedObject> replace;
    std::vector<Add> add;
    bool empty() const { return remove.empty() && replace.empty() && add.empty(); }
};

// What saving gives the patch of a sheet's XML for its `oleObjects`: the children to take out, the replaced ones' new relationships (the object's and the picture's), and the new elements.
struct OleTailEdit {
    struct Add {
        QString progId, objectRel, pictureRel;
        qint64 shapeId = 0;
        DrawingAnchor anchor;
    };
    std::set<int> remove;
    std::map<int, std::pair<QString, QString>> retarget;
    std::vector<Add> add;
};

// The name of a sheet's drawing part, by the sheet part's relationships; empty if the sheet has none (or the part is not in the package).
QString drawingPartOf(const Package &package, const QString &sheetPart);

// Parses the bytes of a drawing part (the first half of readDrawing, which needs no package: it may be done on another thread, and the document given to readDrawing). Null, with a
// message in `error`, if they are damaged XML, have a DOCTYPE, or are past the limits of xml.h.
std::shared_ptr<QDomDocument> parseDrawing(const QByteArray &xml, QString *error = nullptr);

// Reads the drawing part (from `parsed`, if the bytes were parsed already). False, with a message in `error`, if it is not a drawing (damaged XML, a DOCTYPE, or past the limits of xml.h);
// `out` is then empty.
bool readDrawing(const Package &package, const QString &drawingPart, SheetDrawing *out, QString *error = nullptr, const std::shared_ptr<QDomDocument> &parsed = nullptr);

} // namespace jp::office::sheet
