#pragma once
// Fonts in a presentation file. The presentation program keeps a font it
// embeds in a part of its own ("ppt/fonts/font1.fntdata", content type
// application/x-fontdata) as Embedded OpenType (EOT), the format the web fonts
// of the late 1990s used: a header that describes the font (its names, its
// PANOSE class, which characters it has, whether it may be embedded), then the
// font file. The font may be compressed (flag 0x4, MicroType Express, which
// the program does for every font it embeds) and scrambled (flag 0x10000000:
// every byte XOR 0x50). Other programs write plain TrueType data in the same
// part, or an EOT whose font is not compressed, which is what makeEot writes.

#include "core/sfnt.h"

#include <QByteArray>
#include <QString>

namespace jp::office {

// An EOT, version 2.2 as the presentation program writes it, around `font`
// (the whole TrueType or OpenType file, not compressed, not scrambled). Empty
// if the font can't be described (no name table, no OS/2 table).
QByteArray makeEot(const QByteArray &font);

// The font file in a part of embedded font data: the data itself when it is a
// font file, else the font inside an EOT, uncompressed and unscrambled. The
// font is inspected (inspectFont) before it is given back, so what comes out
// is sound; `info` gets what it says about itself. False, with the reason in
// `why`, if there is no usable font in the data: damaged, larger than the
// limits allow, or compressed in a way that is not read.
bool readEmbeddedFont(const QByteArray &data, QByteArray *font, FontFileInfo *info = nullptr, QString *why = nullptr);

} // namespace jp::office
