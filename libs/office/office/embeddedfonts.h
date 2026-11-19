#pragma once
// The fonts a document file carries, for every program (ECMA-376: a presentation's ppt/fonts, a word processing document's word/fonts). A file may hold
// the fonts its text is set in so it looks the same on a computer that lacks them. Reading one makes its font
// available to that document while it is open, and to nothing else: the
// font is added to the font database from memory (never installed, never
// written to disk) under a private family name of its own, and the
// document's aliases say which family each of its runs that names
// the font is to be drawn with (LayoutEnv::familyAliases). Another
// document, a menu, or the program's own text never meets the font, so a
// file's embedded "Arial" can't be what every request for Arial gets, and two
// documents that carry different fonts under one name don't clash. The
// font goes when the last document that carries it does, and with the program
// at the latest: it is never in a file of the computer's, so nothing is left behind
// by a crash. A font the computer already has under that name is left be (the computer's own is
// used).
//
// What a file carries that the program can't write itself (a font it has no
// copy of) is kept with the document, as the part's own bytes, and
// written back while the document uses that font.

#include "core/sfnt.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

namespace jp::office {

// The four styles a file names a family in: regular, bold, italic, and bold italic.
enum FontSlot { RegularSlot = 0, BoldSlot = 1, ItalicSlot = 2, BoldItalicSlot = 3 };
inline FontSlot fontSlot(bool bold, bool italic) { return FontSlot((bold ? 1 : 0) | (italic ? 2 : 0)); }

// A font in a file is shown while the document is open unless its license is the restricted one (embedding any copy of it is not allowed):
// preview and print, editable, and installable all let it be drawn.
inline bool fontMayBeShown(const FontFileInfo &info) { return (info.fsType & 0x000E) != 0x0002; }

// One face of an embedded font as the file lists it.
struct EmbeddedFace {
    QString typeface;             // the file's name for the family
    QString panose;               // a presentation's panose, pitchFamily, and charset as written, to write them back
    int pitchFamily = 0;
    int charset = 0;
    FontSlot slot = RegularSlot;
    QByteArray part;              // the font part as the file has it (an EOT, usually compressed), kept to be written back
    QByteArray font;              // the font file inside it, when it could be read (empty after use() takes it)
    FontFileInfo info;
};

// The fonts added for the documents that are open, by document (any address that is the document's own and stays while it is open).
class EmbeddedFonts {
public:
    // Adds the fonts of the faces that were read, under private family names, and keeps the faces for writing back. Gives the aliases for the document: each family
    // (lower-cased) the faces are for and the private family its font was added under. The faces of a family the computer has are kept but not added.
    static QHash<QString, QString> use(const void *owner, QVector<EmbeddedFace> faces);
    // The document has been closed or replaced: its fonts go, unless another document carries the same font.
    static void release(const void *owner);
    // The faces a document was read with, for writing back (without the fonts' data).
    static QVector<EmbeddedFace> facesOf(const void *owner);
    // How many fonts are added to the font database now.
    static int addedFontCount();
};

} // namespace jp::office
