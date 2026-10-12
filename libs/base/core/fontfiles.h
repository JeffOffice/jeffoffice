#pragma once
// The font files that ship with the program, found by the family and style a
// document names, for putting a copy in a file that others open.

#include <QString>

namespace jp {

// One bundled font file: the family of four styles it belongs to (the name a
// presentation gives, name ID 1; the typographic family, ID 16, too when it
// only groups the four styles) and which of them it is.
struct BundledFontFile {
    QString path, family;
    bool bold = false, italic = false;
    quint16 fsType = 0;
};

// The bundled file for a family in a style, found without regard to case; null
// path when no bundled font has that family or the family has no such style.
// The families are those of the files in bundledFontDirs(), read once.
BundledFontFile bundledFontFile(const QString &family, bool bold, bool italic);
// Whether the family is one of those files' (in any style).
bool isBundledFontFamily(const QString &family);

} // namespace jp
