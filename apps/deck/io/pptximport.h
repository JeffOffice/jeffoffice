#pragma once
// Reading presentation files (.pptx, and the .potx template and .ppsx show
// forms of it: PresentationML in an OPC package) into JeffDeck's model
// (core/deck.h). Slide masters and slide layouts become master pages (props
// "kind" and "type", a layout's baseId its master), each slide a page over
// its layout, each placeholder a text box whose role says what it is and
// whose position, shape, and text formatting come from the layout and master
// it stands on.
//
// What the model has no place for is kept to be written back: the theme part
// (appData "themeXml", base64), a slide's transition and timing (page props
// "transitionXml" and "timingXml", the XML as written), and the frames that
// can't be read yet, such as charts and diagrams (page props "unread", an
// array of {"xml", "x", "y", "w", "h"} with the frame's place in points); the
// XML kept holds the namespace declarations it needs. A frame that holds a
// table becomes a table (TableItem).

#include "core/document.h"

#include <QByteArray>
#include <QString>
#include <memory>

namespace jp::deck {

// A presentation file as a JeffDeck presentation. Null, with the reason in
// `error`, if it can't be read at all (not a package, no presentation in it).
// Otherwise the presentation, and in `error`, one note a line for each slide,
// layout, or master that was left out because it couldn't be read; empty when
// all of it was.
std::unique_ptr<Document> importPresentation(const QByteArray &bytes, QString *error = nullptr);
std::unique_ptr<Document> importPresentationFile(const QString &path, QString *error = nullptr);

// Counted since the program began on this thread, for tests of cost: the placeholders of layouts and masters looked at to match the placeholders of slides to them (each once
// to be indexed, and a few lookups for each placeholder asked about; a list gone through for each would count its length each time).
struct PptxImportWork {
    qint64 placeholdersExamined = 0;
};
PptxImportWork pptxImportWorkForTest();

} // namespace jp::deck
