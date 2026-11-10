#pragma once
// Writing presentation files (the counterpart of pptximport.h): a JeffDeck
// presentation as ECMA-376 PresentationML in an OPC package, laid out as the
// presentation program lays out its own files (part names, relationship types,
// content types, and the order of every element), so that program opens the
// file without a repair prompt and shows the same slides.
//
// The package holds the presentation part, its properties parts, one theme and
// one part for each slide master, a part for each slide layout and slide, a
// notes master and a notes slide for each slide that has notes, the pictures,
// and a thumbnail of the first slide. Shapes and text come from libs/office's
// DrawingML writer; what the reader kept to write back (the theme part, a
// slide's transition and timing, the frames it can't read yet) is written as
// it was kept, except what points at parts the reader did not keep (a chart's
// data, a sound) or at shapes whose numbers the model does not keep (an
// animation's targets): that is left out, with a note. The frames come after
// the page's objects, as the reader does not keep where among them they stood.

#include "core/document.h"
#include "core/embeddedfonts.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::deck {

// The three forms of the file: they differ in one content type, and in the
// ending of the file's name (.pptx, .potx, .ppsx).
enum class PresentationKind { Presentation, Template, Show };

// The kind a file name's ending asks for (.potx a template, .ppsx a show,
// anything else a presentation).
PresentationKind kindOfFileName(const QString &path);

// The presentation as a file's bytes. Empty, with the reason in `error`, if
// it can't be written. `skipped` gets one entry for each thing left out:
// the id of an object the drawing writer has no form for yet, or a line
// saying which slide's transition, animation, or frame was left out and why.
// With `embedFonts` the file carries the program's own fonts that the text
// uses (the styles it uses, each with all its characters: "Embed fonts in the
// file"), as io/pptxfonts.h says; the fonts the file this came from carried
// are written back whatever it is, while the presentation uses them.
QByteArray exportPresentation(const Document &doc, QString *error = nullptr, QStringList *skipped = nullptr,
                              PresentationKind kind = PresentationKind::Presentation, bool embedFonts = false);
// What a copy of the presentation needs to be made on another thread while the window goes on being edited: taken (snapshotForExport) on the thread that owns the presentation, made into
// the file's bytes (exportSnapshot) anywhere. The same bytes exportPresentation gives for the presentation as it was when the snapshot was taken.
// Everything that the text layout or the renderer must do (the picture of the first slide) is done when the snapshot is taken: neither may be used from another thread.
struct ExportSnapshot {
    Document::Snapshot doc;
    QByteArray thumbnail;                  // the first slide's picture, as the file carries it
    QVector<EmbeddedFace> keptFonts;       // the fonts the file this came from carried
};
ExportSnapshot snapshotForExport(const Document &doc);
QByteArray exportSnapshot(const ExportSnapshot &snapshot, QString *error = nullptr, PresentationKind kind = PresentationKind::Presentation, bool embedFonts = false);

// Writes the file at `path`, as its ending asks (kindOfFileName). The old
// file stays whole until the new one is complete: a full disk, a folder that
// can't be written, or a path that isn't one leave it as it was.
// With a `password` the package is protected by it (office/officecrypt.h), as a presentation program does.
bool savePresentationFile(const Document &doc, const QString &path, QString *error = nullptr, QStringList *skipped = nullptr, bool embedFonts = false, const QString &password = QString());

} // namespace jp::deck
