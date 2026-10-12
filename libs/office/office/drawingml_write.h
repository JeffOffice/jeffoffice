#pragma once
// Writing DrawingML (the counterpart of drawingml.h): placed objects become
// shape tree elements and stories become text bodies, as the presentation
// program writes them, so it opens the file without repairs and shows the
// same thing. Elements are written with the "p:", "a:", and "r:" prefixes; the
// part being written declares those namespaces on its root element.

#include "core/color.h"
#include "core/items.h"
#include "office/package.h"
#include "office/theme.h"

#include <QHash>
#include <QSet>
#include <QXmlStreamWriter>
#include <functional>

class QTextDocument;

namespace jp::office {

// What writing a drawing needs from around it.
struct DrawingWriteContext {
    PackageWriter *package = nullptr;
    QString part;                     // the part being written: pictures and links get its relationships
    Theme theme;
    // A picture's file bytes and format ("png", "jpeg", "emf"...) by image id.
    std::function<QByteArray(const QString &imageId, QString *format)> imageBytes;
    std::function<const QTextDocument *(const QString &storyId)> storyDoc;
    // The next shape id in the part (p:cNvPr id): unique within the part.
    int nextShapeId = 2;
    // Pictures already written into the package, by image id, so each is
    // stored once (the part name, "/ppt/media/image3.png").
    QHash<QString, QString> *mediaParts = nullptr;
    // What the scheme colors of a fill, line, or text stand for.
    ColorScheme colors;
    // Set while the text of a placeholder is written: a paragraph with no
    // list then says so (a:buNone), as the layout and master may give theirs.
    bool placeholderText = false;
    // The shape ids written in the part, so none is given twice. A shape that
    // came from a file keeps its number (Item::sourceId) when it is free;
    // `reservedIds` are those the part's items will ask for, which a new
    // shape's number leaves alone.
    QSet<int> usedIds, reservedIds;
    // The id each item (Item::id) was written with, for the connectors that
    // are attached to it.
    QHash<QString, int> shapeIds;
    // How many connection points the standard gives the shape of each item
    // written (4 or 8), for the ones that have them: where a connector joins.
    QHash<QString, int> connectionSites;
    // The relationships made from the part so far, by type and target, so
    // that a picture or an address used twice has one.
    QHash<QString, QString> relations;
};

// Gives each object of a shape tree its number before any is written, in the
// order they will be (a group's own, then those inside it), so that a
// connector can be attached to a shape that comes after it. What can't be
// written (a table, a picture that has no bytes) gets none.
void assignShapeIds(const ItemList &items, DrawingWriteContext &ctx);
// One placed object: p:sp (shapes and text boxes), p:pic, p:cxnSp, or
// p:grpSp, with its transform, geometry (a:prstGeom for presets, a:custGeom
// for freeforms), fill, line, effects, hyperlink, text body, and, for an item
// whose role is "ph:<type>[:<idx>]", its p:nvPr/p:ph. Nothing for an item it
// can't write yet (it says so in `skipped`).
void writeShape(QXmlStreamWriter &w, const Item &item, DrawingWriteContext &ctx, QStringList *skipped = nullptr);
// A story as a:p paragraphs with their a:pPr and runs with their a:rPr
// (inside a p:txBody or a:txBody the caller starts, after its a:bodyPr).
void writeParagraphs(QXmlStreamWriter &w, const QTextDocument *doc, DrawingWriteContext &ctx);
void writeFill(QXmlStreamWriter &w, const Fill &fill, DrawingWriteContext &ctx);
void writeLine(QXmlStreamWriter &w, const Stroke &stroke, DrawingWriteContext &ctx);
// The preset geometry name for a shape id of the shape library, the
// counterpart of shapeForPreset; "" when the shape has none (written as a
// freeform).
QString presetForShape(const QString &shapeId);

} // namespace jp::office
