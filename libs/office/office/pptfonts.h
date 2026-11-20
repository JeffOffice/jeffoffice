#pragma once
// The fonts an old binary presentation carries. Its Environment holds a font collection (record type 2005) made of a font entity atom (4023: the face name,
// 32 UTF-16 characters in the first 64 bytes) for each family, each followed by up to four font data blobs (4024), whose record instance says the style
// (0 regular, 1 bold, 2 italic, 3 bold italic). A blob is an Embedded OpenType (eot.h), nearly always compressed.
//
// A second list of the same entries (type 2006) sits in the document's ___PPT10 tag (section 2.9.11); readPptFonts takes the ranges of the lists in order and
// takes a face (family and style) that more than one list carries once, from the earliest.
//
// The collection is a stranger's bytes: no length is believed past the end of what holds it, at most kMaxPptFontRecords records are looked at, and the
// parts go through EmbeddedFaceIntake's limits.

#include "office/embeddedfonts.h"

#include <QList>
#include <QPair>
#include <QStringList>

namespace jp::office {

constexpr int kMaxPptFontRecords = 2048;

// The faces in the collection whose records are the bytes [begin, end) of `stream`. `notes` get what couldn't be used.
QVector<EmbeddedFace> readPptFonts(const QByteArray &stream, qint64 begin, qint64 end, QStringList *notes = nullptr);

// The same for several lists, one after the other; the record limit counts across them all.
QVector<EmbeddedFace> readPptFonts(const QByteArray &stream, const QList<QPair<qint64, qint64>> &lists, QStringList *notes = nullptr);

} // namespace jp::office
