#pragma once
// MicroType Express (MTX), the compression the presentation program applies to
// the fonts it embeds (an EOT whose flags say so, see eot.h). Reading it is
// the whole job here: a font in a file is a stranger's, so every number the
// data gives is held to a limit before it is used, and no work grows faster
// than the data does.
//
// The data is a 10-byte header (version 3, the copy limit, and where the second
// and third blocks start) and three blocks, each compressed by LZCOMP, an LZ77
// variant whose symbols, copy lengths, and copy distances are coded with three
// adaptive Huffman trees. Block one is a TrueType file's table directory and
// tables, but for the tables that are translated (CTF, the Compact Table
// Format): the glyph outlines (glyf, each glyph as a count of contours, flag
// bytes, and coordinates coded as triplets of 2 to 5 bytes, with the glyph
// programs split off), the control values (cvt, as differences), and the glyph
// index (loca, left out and made again). Block two holds the values the glyph
// programs start by pushing, block three the rest of their instructions.
// The result is the TrueType file the font was; the device metrics tables
// (hdmx, VDMX) are left out of it, as a program draws without them.

#include <QByteArray>
#include <QString>

namespace jp::office {

// The font in MTX data, as an ordinary TrueType file. False, with the reason
// in `why`, if the data is damaged, holds more than a font may (kMaxEmbeddedFontBytes
// of result, in any of its blocks), or is in a form this does not read.
bool mtxToTrueType(const QByteArray &mtx, QByteArray *font, QString *why = nullptr);

// How many bytes the reader has moved on this thread since it began: the bytes a block came to when decompressed, the bytes of tables copied out of it, and the bytes of the
// file made. A count for tests of cost, in which a tiny file is refused before anything is copied, and no byte is moved more often than the data allows.
qint64 mtxBytesMovedForTest();

} // namespace jp::office
