#pragma once
// Reading the old binary word processing format (.doc, .dot, the documents of 1997 to 2003) into the document the .docx reader makes
// (libs/office/wml): the file is a compound file holding the text, the tables of its pieces, the properties of its characters and
// paragraphs, the styles, the lists, and the pictures, and is turned into the XML parts of a .docx package that the .docx reader then
// reads, so everything after it (layout, editing, saving as .docx) is the same code. Only reading is done; a document is saved as .docx.
//
// These files come from strangers. Every count, length, and offset in them is held to the limits below; a structure that is damaged or
// goes past a limit is left out, the rest is read, and the reader's notes say what was left out. Nothing here reads outside the file's
// bytes, loops without a bound, or builds more than the limits allow.

#include "wml/wml.h"

namespace jp::office::doc {

// The file: the largest accepted, and what its text (all the stories together) may come to. Real files: 5 MB at the most of 1,469 public
// documents, and some tens of thousands of characters; a book of a thousand pages is a few million.
constexpr qint64 kMaxFileBytes = qint64(1) << 30;
constexpr qint64 kMaxChars = 40000000;
// The pieces of the text (a piece is a run of text stored in one place and one coding), and the runs of properties in the bins.
constexpr int kMaxPieces = 500000;
constexpr qint64 kMaxBinRuns = 8000000;
// A list of property changes (a sprm group) is read to this many changes; a style chain, to this many styles deep.
constexpr int kMaxSprms = 4096;
constexpr int kMaxStyleDepth = 40;
constexpr int kMaxStyles = 4095;
// Lists and their instances.
constexpr int kMaxLists = 16000;
constexpr int kMaxListInstances = 65000;
// Tables: as deep as the .docx reader reads, and as many columns as a row's definition can hold.
constexpr int kMaxTableDepth = wml::kMaxTableDepth;
constexpr int kMaxCells = 64;
// Fields that nest in one another (an unclosed field is closed at the end of its paragraph).
constexpr int kMaxFieldDepth = 20;
// Sections, header and footer stories, notes (the .docx reader's own limit), and fonts.
constexpr int kMaxSections = 10000;
constexpr int kMaxHeaderStories = 6 * 2000;
constexpr int kMaxNotes = wml::kMaxNotes;
constexpr int kMaxFonts = wml::kMaxFonts;
// Paragraphs of all the stories together (the .docx reader takes 500,000 in a body).
constexpr int kMaxParagraphs = 450000;
// Pictures: one picture's bytes (a metafile is inflated to at most this), all the pictures together, and how many.
constexpr qint64 kMaxPictureBytes = qint64(48) << 20;
constexpr qint64 kMaxPictureTotal = qint64(384) << 20;
constexpr int kMaxPictures = 20000;
// The XML made from the file, all parts together.
constexpr qint64 kMaxOutputBytes = qint64(768) << 20;

// True when the bytes begin as a compound file does, the container of the old binary formats.
bool looksLikeCompoundFile(const QByteArray &bytes);

// The .docx package that the old binary document comes to; empty, with the reason in `error`, if the file is not one this reads (not a
// compound file, no document in it, from before 1997, or protected with a password). `notes` gets one line for each kind of thing left out.
QByteArray toDocxPackage(const QByteArray &bytes, QString *error = nullptr, wml::ReadNotes *notes = nullptr);

// The document: the package above read as a .docx is (compatibility mode 11, the mode of the 2003 programs, since the file says no other).
// False, with the reason, if the file can't be read.
bool readDoc(const QByteArray &bytes, wml::Document *doc, QString *error = nullptr, wml::ReadNotes *notes = nullptr);

} // namespace jp::office::doc
