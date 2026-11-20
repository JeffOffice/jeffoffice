#pragma once
// The fonts a presentation file carries (the shared reader and registry are libs/office/office/embeddedfonts.h): reading one makes its font
// available to that presentation while it is open, and `fontAliases` says which family each of its runs that names the font is drawn with.

#include "core/document.h"
#include "office/embeddedfonts.h"

#include <QVector>

namespace jp::deck {

using office::BoldItalicSlot;
using office::BoldSlot;
using office::EmbeddedFace;
using office::fontSlot;
using office::FontSlot;
using office::ItalicSlot;
using office::RegularSlot;

class EmbeddedFonts {
public:
    // Adds the fonts of the faces that were read and sets `doc`'s fontAliases (a presentation that was just read; its fonts go when the document is destroyed).
    static void use(Document *doc, QVector<EmbeddedFace> faces) { doc->fontAliases = office::EmbeddedFonts::use(doc, std::move(faces)); }
    // The document has been closed or replaced: its fonts go, unless another document carries the same font.
    static void release(const Document *doc) { office::EmbeddedFonts::release(doc); }
    // The faces a document was read with, for writing back (without the fonts' data).
    static QVector<EmbeddedFace> facesOf(const Document *doc) { return office::EmbeddedFonts::facesOf(doc); }
    // How many fonts are added to the font database now.
    static int addedFontCount() { return office::EmbeddedFonts::addedFontCount(); }
};

} // namespace jp::deck
