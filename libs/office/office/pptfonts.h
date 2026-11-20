#pragma once
// The fonts an old binary presentation carries. Its Environment holds a font collection (record type 2005) made of a font entity atom (4023: the face name,
// 32 UTF-16 characters in the first 64 bytes) for each family, each followed by up to four font data blobs (4024), whose record instance says the style
// (0 regular, 1 bold, 2 italic, 3 bold italic). A blob is an Embedded OpenType (eot.h), nearly always compressed.
//
// The collection is a stranger's bytes: no length is believed past the end of what holds it, at most kMaxPptFontRecords records are looked at, and the
// parts go through EmbeddedFaceIntake's limits.

#include "office/embeddedfonts.h"

namespace jp::office {

constexpr int kMaxPptFontRecords = 2048;

// The faces in the collection whose records are the bytes [begin, end) of `stream`. `notes` get what couldn't be used.
QVector<EmbeddedFace> readPptFonts(const QByteArray &stream, qint64 begin, qint64 end, QStringList *notes = nullptr);

} // namespace jp::office
