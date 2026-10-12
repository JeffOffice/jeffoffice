#pragma once
// Reading OpenDocument presentations (.odp, and the .otp template form: a ZIP
// of content.xml, styles.xml, and meta.xml, ODF 1.2 and 1.3) into JeffDeck's
// model (core/deck.h), the way io/pptximport.h reads the newer presentation
// files. The page size is the slide size; each master page (style:master-page)
// becomes a slide master with its drawing objects and background, and its
// placeholders (the title, the outline, the date, footer, and page number) are
// that master's prompts. The layouts under a master are the ones its slides
// use (by the placeholders each has), and the four every presentation has: a
// title slide, a title and content, a title only, and a blank one. Each
// draw:page is a slide with its objects, notes, and transition.
//
// Text takes the formatting the styles give it (the cascade of the common and
// automatic styles, the presentation styles of placeholders and their outline
// levels, and the list styles of bullets and numbers). Objects: text boxes,
// shapes (draw:custom-shape by its preset type, else by its path), the simple
// drawing shapes (rectangles, ellipses, lines, polygons, paths, connectors),
// groups, pictures with their crop, tables, and charts and other embedded
// objects as the picture the file keeps of them. Fills (a color, a gradient, a
// picture, a hatch), outlines (width, dashes, arrow ends), shadows, opacity,
// turns, and flips are kept.
//
// What isn't read is left out with a note: animations, the transitions that
// JeffDeck has no match for, embedded objects with no picture, and sounds and
// movies.

#include "core/document.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <memory>

namespace jp::deck {

// An open document presentation (or template) as a JeffDeck presentation. Null,
// with the reason in `error`, if it can't be read at all (not a package, not a
// presentation, protected with a password, no pages). Otherwise the
// presentation, and in `skipped` one note for each kind of thing that was left
// out; empty when all of it was read.
std::unique_ptr<Document> importOdp(const QByteArray &zip, QString *error = nullptr, QStringList *skipped = nullptr);

// Whether the first bytes of a file are those of an open document package
// (the ZIP's first entry is "mimetype", stored, as the standard has it, and
// holds an open document type). The package may be of another kind of open
// document than a presentation, which importOdp refuses, saying what it is.
bool looksLikeOpenDocument(const QByteArray &head);

} // namespace jp::deck
