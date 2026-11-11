#pragma once
// The drawing objects of a sheet in an OpenDocument spreadsheet: pictures, shapes, text boxes, Text Art (as the fontwork shape the standard has), groups, and lines. They are kept in the
// workbook as the XML form keeps them (the anchors of a drawing part, sheet/drawing.h, each object an element the shared drawing code of office/drawingml.h reads and writes), so this
// converts between that and the standard's elements: a frame or custom shape in the sheet's `table:shapes` (or in a cell), with its place (the cell it is anchored to and how far into it, or a
// place of the sheet), size, turn, order, name, alternative text (svg:title and svg:desc), fill, outline, text, and picture bytes with the crop.
//
// Both ways the objects come from a stranger's file or a model that may be damaged: the objects are at most kMaxDrawingAnchors to a sheet and kMaxWorkbookObjects to a workbook, a picture
// at most kMaxPictureBytes and all of them kMaxPictureTotal, the text of an object at most kMaxTextChars, and the nesting of groups and of the elements read are bounded. What is past
// a limit, and what is of a kind that is not read, is left out and named.
//
// Not here: charts (an embedded chart object is neither read nor written; the reader says so, as it did before).

#include "sheet/drawing.h"
#include "sheet/cellref.h"
#include "sheet/drawingadd.h"
#include "sheet/sheetmodel.h"

#include <QByteArray>
#include <QHash>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>

#include <atomic>
#include <functional>
#include <memory>

class QXmlStreamReader;

namespace jp::office {
class Package;
}

namespace jp::office::sheet::odsdraw {

constexpr qint64 kMaxPictureBytes = qint64(32) << 20;
constexpr qint64 kMaxPictureTotal = kMaxAddedBytes;
constexpr int kMaxTextChars = 100000;
constexpr int kMaxParagraphs = 5000;
constexpr int kMaxGroupDepth = 16;
constexpr int kMaxNodes = 400000;            // the elements one sheet's shapes may be made of
constexpr qint64 kMaxNodeChars = 90000000;   // and the characters in them (the text of the objects, and a picture given inside the document as base64)

// What one workbook's reading has taken so far (the sheets are read apart, by any thread).
struct Budget {
    std::atomic<qint64> objects{0}, pictureBytes{0};
};

// ---- the grid: where cells are, in EMU from the sheet's top left
class Grid {
public:
    // A stretch of columns (or rows) and the size of each, in points (0 for hidden); those that are not listed have the default size.
    struct Run {
        quint32 first = 0, last = 0;
        double points = 0;
    };
    Grid(const QVector<Run> &columns, double defaultColPt, const QVector<Run> &rows, double defaultRowPt);
    qint64 columnStart(quint32 col) const { return m_cols.start(col); }   // EMU
    qint64 rowStart(quint32 row) const { return m_rows.start(row); }
    // The cell a distance falls in, and how far into it (EMU).
    void columnAt(qint64 x, quint32 *col, qint64 *offset) const { m_cols.at(x, kMaxColumns, col, offset); }
    void rowAt(qint64 y, quint32 *row, qint64 *offset) const { m_rows.at(y, kMaxRows, row, offset); }
    // An anchor's box, from the sheet's top left.
    void boxOf(const DrawingAnchor &a, qint64 *x, qint64 *y, qint64 *cx, qint64 *cy) const;

private:
    struct Axis {
        QVector<Run> runs;            // sorted, not overlapping
        QVector<qint64> sizeBefore;   // how much more than the default size all the cells before each run have in all
        qint64 defaultEmu = 0;
        void build(QVector<Run> list, double defaultPt);
        qint64 start(quint32 index) const;
        qint64 size(quint32 index) const;
        void at(qint64 distance, quint32 count, quint32 *index, qint64 *offset) const;
    };
    Axis m_cols, m_rows;
};

// ---- reading
// An element of the document and what is in it, kept whole so that it is read once the styles are known. Names are the standard's own prefixes ("draw:frame", "text:p", "svg:title");
// an attribute of the xlink space is "xlink:href".
struct Node {
    QString name;
    QHash<QString, QString> attrs;
    QVector<std::shared_ptr<Node>> children;
    // The characters that come before each child, and after the last (`texts.size() == children.size() + 1`).
    QStringList texts;
};
using NodePtr = std::shared_ptr<Node>;

struct Counters {
    int nodes = 0;
    qint64 chars = 0;
    bool over = false;   // more was in the element than the limits let be kept
};

// Reads the element `xml` has just begun (its start is the current token) to its end, with the elements in it. False if the XML is damaged or nests deeper than `depthLeft`. What is past the
// limits of `counters` is read and not kept (`over`).
bool readNode(QXmlStreamReader &xml, int depthLeft, Counters *counters, NodePtr *out);

// A shape of the document, and where it was.
struct SheetShape {
    NodePtr node;
    bool inCell = false;
    quint32 row = 0, col = 0;   // the cell it is in
};

// The properties of a graphic or text or paragraph style by name, keyed as the styles are ("graphic:draw:fill", "text:fo:font-weight", "para:fo:text-align").
using StyleLookup = std::function<QHash<QString, QString>(const QString &family, const QString &name)>;

// Makes the drawing of the sheet from its shapes. `parts` gives the bytes of a part of the package by its name ("Pictures/a.png"; empty for one that is not there). The drawing is empty,
// and `notRead` says what was left out, if nothing can be made.
void shapesToDrawing(const QVector<SheetShape> &shapes, const Counters &counters, const Grid &grid, const StyleLookup &styles, const std::function<QByteArray(const QString &)> &parts, Budget *budget,
                     ImportedDrawing *out);

// ---- writing
// What a sheet's drawing is as the standard's elements.
struct Written {
    QString shapes;                                  // the elements of table:shapes
    QString styles;                                  // the automatic styles they use (for content.xml)
    QString commonStyles;                            // the styles they use that go in styles.xml (dashes)
    QVector<QPair<QString, QByteArray>> pictures;    // the file's name in the package ("Pictures/a.png") and its bytes
    int count = 0;
    QStringList notWritten;                          // what was left out, in words
};
// `package` holds the workbook whose sheet has the drawing part `drawingPart`; `sheetNumber` makes the names of the styles different from those of other sheets; `pictureBytes` is what the
// workbook's earlier sheets have taken of the room for pictures.
// `sheetRef` is the sheet's name as an address in the file writes it (quoted if it needs); `objects` counts the objects the workbook's earlier sheets have written.
void drawingToShapes(const Package &package, const QString &mainPart, const QString &drawingPart, const Grid &grid, const QString &sheetRef, int sheetNumber, Written *out, qint64 *pictureBytes,
                     qint64 *objects);

// Lengths: a length in EMU as the standard writes it ("1.2345678in"), and one read (false if it is none); a number that is too large is kept to the most a position may be.
QString lengthText(qint64 emu);
bool lengthEmu(const QString &text, qint64 *emu);

} // namespace jp::office::sheet::odsdraw
