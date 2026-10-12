#pragma once
// Opening a presentation file of either format.

#include "core/document.h"

#include <QString>
#include <QStringList>
#include <memory>

namespace jp::deck {

// A presentation file as a JeffDeck presentation, by what the file is and not
// what it is called: one that starts with the compound file signature is an
// old binary presentation (io/pptimport.h), one whose ZIP starts with the
// open document type an open document presentation (io/odpimport.h), any
// other a package of the newer format (io/pptximport.h). Null, with the
// reason in `error`, if it can't be read; otherwise the notes of what was
// left out, as those readers give them.
std::unique_ptr<Document> importAnyPresentationFile(const QString &path, QString *error = nullptr);

// The notes that say what a reader left out of a file as unsafe or too
// large: `pictures` (Document::imagesLeftOut), and `parts`, the parts and
// links of its package that can't be used (office::Package::skippedCount).
QStringList leftOutNotes(int pictures, int parts);

} // namespace jp::deck
