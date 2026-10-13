#pragma once
// Writing open document presentations (the counterpart of odpimport.h): a
// JeffDeck presentation as ODF 1.3, laid out the way LibreOffice Impress lays
// out its own files, so that program opens it and shows the same slides, and
// JeffDeck reads it back as it was.
//
// The package is a ZIP whose first entry is the stored "mimetype", then
// META-INF/manifest.xml listing every part with its media type, content.xml
// (the automatic styles and a draw:page for each slide, with its objects,
// speaker notes, transition, animations, and comments), styles.xml (the
// page size, the default and named styles, and a style:master-page for each
// slide master and each layout that has graphics or a background of its own,
// the others standing on their master's page), meta.xml (the generator
// "JeffDeck/<version>", title, author, and dates), and the pictures under
// Pictures/. A template (.otp) differs in one line: its type.
//
// Slide masters and layouts: a slide's placeholders carry their own places and
// text formatting, so a layout that only holds placeholders needs no page of
// its own; a layout with graphics, a background, or hidden master graphics
// gets one, with its master's graphics under its own. Placeholders are
// written with their presentation:class (title, subtitle, outline, notes,
// date-time, footer, page-number), and the date, footer, and slide number of
// a slide are the page style's flags and the presentation's declarations.
//
// Text is written with every property explicit (paragraph and character
// styles, a list style for each run of bullets or numbers), so the cascade of
// a master's text styles never changes what a slide shows; the master's own
// are written too, for text typed later.
//
// What ODF has no form for is left out, and `skipped` says so: SmartArt
// diagrams, charts, and equations are written as the pictures JeffDeck shows
// (they are pictures to it too), frames that were never read, effects of
// light and depth (reflection, bevel, turned-in-space), sections, and the
// animation effects with no ODF form.

#include "core/document.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::deck {

// The presentation as the bytes of an .odp file (an .otp when `asTemplate`).
// Empty, with the reason in `error`, if it can't be written. `skipped` gets
// one line for each kind of thing that was left out.
QByteArray exportOdp(const Document &doc, bool asTemplate = false, QString *error = nullptr, QStringList *skipped = nullptr);

// Writes the file at `path`, an .otp when its ending is that. The old file
// stays whole until the new one is complete.
bool saveOdpFile(const Document &doc, const QString &path, QString *error = nullptr, QStringList *skipped = nullptr);

} // namespace jp::deck
