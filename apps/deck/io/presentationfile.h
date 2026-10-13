#pragma once
// Opening a presentation file of either format.

#include "core/document.h"
#include "io/pptxexport.h"

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

// The two formats JeffDeck writes: the newer presentation files (.pptx, .potx,
// .ppsx) and the open document ones (.odp, .otp). The ending of a file's name
// says which; any other ending is a .pptx.
enum class FileFormat { Pptx, Odp };
FileFormat formatOfFileName(const QString &path);
// The kind a file name's ending asks for: a template (.potx, .pot, .otp), a
// show (.ppsx, .pps), or a presentation (anything else, .odp included).
PresentationKind presentationKindOf(const QString &path);
// Whether JeffDeck writes files with that ending itself (.pptx, .potx,
// .ppsx, .odp, .otp); an old binary file (.ppt, .pps, .pot) or another ending
// is read, never written over.
bool isWrittenEnding(const QString &path);
// Writes the presentation at `path`, in the format its ending asks for
// (formatOfFileName). The old file stays whole until the new one is complete.
// `skipped` gets a line for each thing the format has no form for.
bool saveAnyPresentationFile(const Document &doc, const QString &path, QString *error = nullptr, QStringList *skipped = nullptr);

// The notes that say what a reader left out of a file as unsafe or too
// large: `pictures` (Document::imagesLeftOut), and `parts`, the parts and
// links of its package that can't be used (office::Package::skippedCount).
QStringList leftOutNotes(int pictures, int parts);

} // namespace jp::deck
