#pragma once
// Reading binary OfficeArt drawings ([MS-ODRAW]), the drawing records inside
// .ppt slides, .doc documents, and .xls sheets: shape containers with their
// property tables become placed objects (libs/draw), and the picture store's
// records become pictures. What the host format keeps in a shape's client
// records (its anchor, its text, a placeholder's identity) is the caller's,
// through the context's functions.

#include "core/items.h"

#include <QByteArray>
#include <QColor>
#include <QRectF>
#include <functional>

namespace jp::office {

// One record's header ([MS-ODRAW] 2.1.1 OfficeArtRecordHeader).
struct OdrawRecord {
    quint16 version = 0, instance = 0, type = 0;
    quint32 length = 0;
    qsizetype offset = 0;   // of the record's data, past the 8-byte header
};
// The records in `data` from `offset` up to `end` (the whole of `data` when
// -1), one level deep; a record running past the end stops the list.
QVector<OdrawRecord> odrawRecords(const QByteArray &data, qsizetype offset = 0, qsizetype end = -1);

// How much of drawings a host reads in all, shared by every drawing it reads: a file can have one drawing read again and again, or
// a drawing for each of thousands of slides.
struct OdrawBudget {
    // The shapes it still takes (a group is one, and what is in it), and the records looked at to find a background.
    qint64 left = 0;
    // Set when a drawing had more shapes than were left to read, or than one drawing is read to (100,000), so the host can say so.
    bool refused = false;
};

// What reading a drawing needs from the format around it.
struct OdrawContext {
    // Given, the budget that the drawings read with this context share: reading a drawing stops with what it has when it is spent.
    OdrawBudget *budget = nullptr;
    // The drawing group's default properties: the property records (0xF00B,
    // 0xF121, 0xF122, header included) of its container (0xF000). A shape
    // that leaves a property out has the value these give, before the one the
    // format gives it when nothing does.
    QByteArray defaults;
    // The shape's place on the page, in points, from its client anchor
    // record's data (OfficeArtClientAnchor, 0xF010), whose form each host
    // format defines (a .ppt slide's is a rectangle in master units).
    std::function<QRectF(const QByteArray &anchor)> clientAnchor;
    // A color whose fSchemeIndex bit is set: the host's color scheme entry.
    std::function<QColor(int index)> schemeColor;
    // The picture with this 1-based index in the picture store (the shape's
    // pib property), as an image id (PictureItem::imageId); "" if none.
    std::function<QString(int blipIndex)> picture;
    // The picture of a fill that has a brightness and a contrast (as a
    // PictureItem has them: percent, -100 to 100) in its pixels, a fill having
    // none of its own; the same image id if it can't be done.
    std::function<QString(const QString &imageId, double brightness, double contrast)> adjusted;
    // The shape another takes after: from a shape's client data record's
    // contents, the property records (0xF00B, 0xF121, 0xF122, header included)
    // of the shape whose properties it has for those it leaves out (a slide's
    // placeholder takes after its master's); empty for a shape that takes
    // after none.
    std::function<QByteArray(const QByteArray &clientData)> inherits;
    // The XML the shape's writer kept beside its properties for the shape
    // (property 0x03A9: a package whose shape part is a DrawingML shape, which
    // holds what the properties can't), for the host to take the shape's look
    // from. Called after the item is made, before its client records are read.
    std::function<void(Item &item, const QByteArray &package)> richLook;
    // The page's own background fill, which a shape filled the background
    // way (fillType 9) shows where it is; none when the host can't say. Given
    // the shape's frame, for a fill that keeps its place on the page (tiles).
    std::function<Fill(const QRectF &shape)> background;
    // The shape's client text box (0xF00D) and client data (0xF011)
    // records' contents: text, a placeholder's identity. Called after the
    // item is made, so it may set its story, its role, or replace its type's
    // text properties.
    std::function<void(Item &item, const QByteArray &clientTextbox, const QByteArray &clientData)> client;
};

// The shapes of a drawing (an OfficeArtDgContainer, 0xF002, or its
// OfficeArtSpgrContainer, 0xF003), in drawing order: rectangles and preset
// shapes (core/shapetypes.h numbers them), freeforms (pVertices and
// pSegmentInfo), lines and connectors, pictures, text boxes, and groups,
// with their fills (solid, gradient, picture, pattern), lines (width, dash,
// arrowheads), rotation, flips, shadows, and hyperlinks. The background shape
// (fBackground) is left out: see odrawBackground. `data` holds the
// container's record, header included.
ItemList readOdrawShapes(const QByteArray &data, const OdrawContext &ctx);
// The drawing's background shape's fill, if it has one.
Fill odrawBackground(const QByteArray &data, const OdrawContext &ctx);

// A picture store record (an OfficeArtBlip: 0xF01A EMF, 0xF01B WMF, 0xF01C
// PICT, 0xF01D JPEG, 0xF01E PNG, 0xF01F DIB, 0xF029 TIFF, 0xF02A JPEG), as
// the picture file's bytes and its format ("emf", "wmf", "png", "jpeg",
// "bmp", "tiff"); empty when it can't be read. EMF and WMF come compressed
// in the record and are inflated (at most 32 MB). `data` holds the record,
// header included.
QByteArray odrawBlip(const QByteArray &data, QString *format);
// The records of an OfficeArtBStoreContainer's file blocks (0xF007), in
// order, each as `data` and `offset` of its BLIP: in the stream itself, or
// at its foDelay offset in a separate one (a .ppt's "Pictures" stream).
struct OdrawStoreEntry { int index = 0; quint32 delayOffset = 0; QByteArray embedded; };
QVector<OdrawStoreEntry> odrawStore(const QByteArray &bstoreContainer);

} // namespace jp::office
