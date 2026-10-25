#pragma once
// Reading the old binary presentation files ([MS-PPT]: .ppt, and the .pps
// show and .pot template forms) into JeffDeck's model (core/deck.h), the way
// io/pptximport.h reads the newer ones.

#include "core/document.h"

#include <QByteArray>
#include <QString>
#include <memory>

namespace jp::deck {

// An old binary presentation (.ppt, .pps, .pot) as a JeffDeck presentation.
// Null, with the reason in `error`, if it can't be read at all (not a
// compound file, no presentation in it, protected with a password, edits that
// go round in a circle, cut short). Otherwise the presentation, and in
// `error`, one note a line for each slide or master that was left out because
// it couldn't be read; empty when all of it was.
std::unique_ptr<Document> importBinaryPresentation(const QByteArray &bytes, QString *error = nullptr);

// The work the last presentation read on this thread took, counted for tests of cost: the shapes of drawings taken, the persist entries listed, and the items or entries that
// the lookups of pictures, notes, and master placeholders went through (each is passed once to make an index and not again for each lookup).
struct PptImportWork {
    qint64 shapesTaken = 0, persistRead = 0, storeEntriesVisited = 0, notesVisited = 0, placeholderItemsVisited = 0, writersXmlBytes = 0, pp9EntriesRead = 0;
};
PptImportWork lastPptImportWorkForTest();

} // namespace jp::deck
