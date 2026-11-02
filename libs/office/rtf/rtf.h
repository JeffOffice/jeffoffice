#pragma once
// Reading Rich Text Format files (.rtf) into the document the .docx reader makes (libs/office/wml): the file is turned into the XML parts of a .docx
// package, which the .docx reader then reads, so everything after it (layout, editing, saving as .docx) is the same code. This is the way the old binary
// format (libs/office/doc) is read too. Only reading is done; a document is saved as .docx.
//
// These files come from strangers. The nesting of groups, the size of the file, the count of every kind of thing a file can ask for (fonts, colors, styles, lists,
// notes, headers, tables, pictures), and the XML made from it are held to the limits below; what goes past a limit is left out, the rest is read, and the reader's
// notes say what was left out. Nothing here loops without a bound, reads past the bytes, or builds more than the limits allow.

#include "wml/wml.h"

namespace jp::office::rtf {

// The file, the groups that nest in one another (real files: up to about 40), and the characters of text all together.
constexpr qint64 kMaxFileBytes = qint64(1) << 29;
constexpr int kMaxGroupDepth = 256;
constexpr qint64 kMaxChars = 40000000;
// Fonts, colors, styles, lists (and their instances), and the levels of a list.
constexpr int kMaxFonts = wml::kMaxFonts;
constexpr int kMaxColors = 4096;
constexpr int kMaxStyles = 4095;
constexpr int kMaxLists = 16000;
constexpr int kMaxListInstances = 65000;
// Tab stops of one paragraph, and cells in a row.
constexpr int kMaxTabs = 64;
constexpr int kMaxRowCells = wml::kMaxTableColumns;
// Tables: as deep, as many rows, and as many cells (all the tables together) as the .docx reader lays out.
constexpr int kMaxTableDepth = wml::kMaxTableDepth;
constexpr int kMaxTableRows = wml::kMaxTableRows;
constexpr int kMaxTableCellsAll = wml::kMaxTableCells;
// Notes (the .docx reader's own limit), header and footer stories, bookmarks, and fields that nest in one another.
constexpr int kMaxNotes = wml::kMaxNotes;
constexpr int kMaxHeaderStories = 12000;
constexpr int kMaxBookmarks = 20000;
constexpr int kMaxShapes = 5000;
constexpr int kMaxFieldDepth = 20;
constexpr int kMaxFieldInstructionChars = 4096;
// Paragraphs of all the stories together (the .docx reader takes 500,000 in a body), and sections.
constexpr int kMaxParagraphs = 450000;
constexpr int kMaxSections = 10000;
// Pictures: one picture's bytes, all together, and how many.
constexpr qint64 kMaxPictureBytes = qint64(32) << 20;
constexpr qint64 kMaxPictureTotal = qint64(64) << 20;
constexpr int kMaxPictures = 20000;
// The XML made from the file, all parts together: its bytes. (Past it the text stops where it is.)
constexpr qint64 kMaxOutputBytes = qint64(96) << 20;

// True when the bytes begin as a Rich Text Format file does ("{\rtf", after any spaces or a byte order mark).
bool looksLikeRtf(const QByteArray &bytes);

// The .docx package that the document comes to; empty, with the reason in `error`, if the file is not one this reads. `notes` gets one line for each kind of
// thing left out.
QByteArray toDocxPackage(const QByteArray &bytes, QString *error = nullptr, wml::ReadNotes *notes = nullptr);

// For tests of cost: the most the last file read on this thread kept of what a file can make many of (override levels of a list instance, the text of a list level's characters).
struct ReadWork {
    int maxOverrides = 0, maxLevelRaw = 0;
};
ReadWork lastWorkForTest();

// For tests of cost: the XML the last file read on this thread came to, counted against kMaxOutputBytes.
qint64 lastOutputBytesForTest();

// The document: the package above read as a .docx is. False, with the reason, if the file can't be read.
bool readRtf(const QByteArray &bytes, wml::Document *doc, QString *error = nullptr, wml::ReadNotes *notes = nullptr);

} // namespace jp::office::rtf
