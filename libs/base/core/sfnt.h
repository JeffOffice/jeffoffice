#pragma once
// TrueType and OpenType font files (the "sfnt" wrapper: a table directory,
// then the tables), read as far as a program needs to embed a font in a file,
// to take one out of a file, and to be sure the one it was given is sound
// before a font engine sees it. A font taken from a file is a stranger's:
// inspectFont holds it to limits and checks that every table the directory
// lists lies inside the file and holds what the tables that tell the engine
// how many glyphs there are, where they are, and how wide, say it needs.

#include <QByteArray>
#include <QString>

namespace jp {

// The most bytes one font file taken from a document may be (the largest East
// Asian fonts are about 25 MB), and the most fonts and bytes one document may
// carry together.
constexpr qint64 kMaxEmbeddedFontBytes = 32LL * 1024 * 1024;
constexpr int kMaxEmbeddedFonts = 64;
constexpr qint64 kMaxEmbeddedFontsTotal = 256LL * 1024 * 1024;
// The most tables a font's directory may list (real fonts have 10 to 40).
constexpr int kMaxFontTables = 100;

// What the name, head, OS/2, and post tables say about a font.
struct FontFileInfo {
    QString family, subfamily;     // name IDs 1 and 2: the family of four styles (regular, bold, italic, bold italic) a presentation names
    QString typographicFamily;     // ID 16 when the font has one, else empty
    QString fullName, versionName; // IDs 4 and 5
    quint16 weight = 400;          // OS/2 usWeightClass
    quint16 fsType = 0;            // OS/2 embedding permissions (0 when the font has no OS/2 table)
    bool bold = false, italic = false;
    bool outlinesAreCff = false;   // an OpenType font with PostScript outlines
    bool fixedPitch = false;
    quint8 panose[10] = {};
    quint32 unicodeRange[4] = {};
    quint32 codePageRange[2] = {};
    quint32 checkSumAdjustment = 0;
    int glyphs = 0;
    // How a presentation file describes the font (a:latin's pitchFamily and charset, and the same in an embedded font's header).
    quint8 pitchFamily() const;
    quint8 charset() const;
};

// Whether `font` is a TrueType or OpenType file that is safe to hand to the
// font engine. False, with the reason in `why`, if it is not: not a font,
// larger than kMaxEmbeddedFontBytes, a collection, a variable font, tables
// that share bytes or run past the end of the file, a table too short for what
// the font says it holds, a glyph that is not inside the glyph data or whose
// parts are not (a composite glyph naming a glyph the font lacks, inside
// itself, or nested more than 16 levels or than maxp says), a character map
// that names a glyph the font lacks. `info` gets what the font says about itself.
bool inspectFont(const QByteArray &font, FontFileInfo *info = nullptr, QString *why = nullptr);
// The same for font data in memory that is not a QByteArray (a mapped file).
bool inspectFont(const uchar *data, qint64 size, FontFileInfo *info = nullptr, QString *why = nullptr);

// Whether a font's embedding permissions (OS/2 fsType) let its file travel in
// a document that others open and edit: installable (0) and editable (8)
// do. Restricted licenses (2), preview-and-print (4) when nothing freer is
// also set, and bitmap-only embedding (0x200) do not. When more than one of
// the three levels is set the freest applies.
bool fontAllowsEmbedding(quint16 fsType);

} // namespace jp
