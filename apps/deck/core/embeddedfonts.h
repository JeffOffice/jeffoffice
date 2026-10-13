#pragma once
// The fonts a presentation file carries. A file may hold the fonts its text
// is set in (ppt/fonts, listed in the presentation's p:embeddedFontLst) so it
// looks the same on a computer that lacks them. Reading one makes its font
// available to that presentation while it is open, and to nothing else: the
// font is added to the font database from memory (never installed, never
// written to disk) under a private family name of its own, and the
// presentation's `fontAliases` say which family each of its runs that names
// the font is to be drawn with (LayoutEnv::familyAliases). Another
// presentation, a menu, or the program's own text never meets the font, so a
// file's embedded "Arial" can't be what every request for Arial gets, and two
// presentations that carry different fonts under one name don't clash. The
// font goes when the last presentation that carries it does. A font the
// computer already has under that name is left be (the computer's own is
// used, as the presentation program does).
//
// What a file carries that the program can't write itself (a font it has no
// copy of) is kept with the presentation, as the part's own bytes, and
// written back while the presentation uses that font.

#include "core/document.h"
#include "core/sfnt.h"

#include <QByteArray>
#include <QString>
#include <QVector>

namespace jp::deck {

// The four styles a presentation names a family in: p:regular, p:bold, p:italic, and p:boldItalic.
enum FontSlot { RegularSlot = 0, BoldSlot = 1, ItalicSlot = 2, BoldItalicSlot = 3 };
inline FontSlot fontSlot(bool bold, bool italic) { return FontSlot((bold ? 1 : 0) | (italic ? 2 : 0)); }

// One face of an embedded font as the file lists it.
struct EmbeddedFace {
    QString typeface;             // p:font's name for the family
    QString panose;               // p:font's panose, pitchFamily, and charset as written, to write them back
    int pitchFamily = 0;
    int charset = 0;
    FontSlot slot = RegularSlot;
    QByteArray part;              // the font part as the file has it (an EOT, usually compressed), kept to be written back
    QByteArray font;              // the font file inside it, when it could be read (empty after use() takes it)
    FontFileInfo info;
};

class EmbeddedFonts {
public:
    // Adds the fonts of the faces that were read, under private family names, and sets `doc`'s fontAliases (a presentation that was just read;
    // not kept alive: release it when the document goes), and keeps the faces for writing back. The faces of a family the computer has are
    // kept but not added.
    static void use(Document *doc, QVector<EmbeddedFace> faces);
    // The document has been closed or replaced: its fonts go, unless another document carries the same font.
    static void release(const Document *doc);
    // The faces a document was read with, for writing back (without the fonts' data).
    static QVector<EmbeddedFace> facesOf(const Document *doc);
    // How many fonts are added to the font database now, and the families they give.
    static int addedFontCount();
    static QStringList addedFamilies();
};

} // namespace jp::deck
