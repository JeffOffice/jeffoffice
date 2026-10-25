#pragma once
// The notes (the comments a person writes on a cell) that a worksheet's part has, read for Find: by cell, the note's text. They are only read, never written (a note is kept in the
// file as it is). A damaged part gives what was read before the damage.

#include "office/package.h"
#include "sheet/sheetreader.h"

#include <QString>

#include <map>

namespace jp::office::sheet {

// The most notes and characters read from one sheet, and the largest comments part looked at.
constexpr qint64 kMaxNotes = 100000;
constexpr qint64 kMaxNoteChars = qint64(8) << 20;
constexpr qint64 kMaxNotesPartBytes = qint64(64) << 20;

// The notes of the sheet (by its place in the workbook) as the file has them, by `(row << 32) | col`. Empty for a sheet with none, a sheet that was added, and a workbook that
// has no package of XML parts. `truncated`, if given, is told whether a limit stopped the read.
// How many times a sheet's notes have been read from a file since the program began (a test counts them to see that a search does not read them again and again).
qint64 noteReads();
std::map<quint64, QString> readSheetNotes(const Workbook &book, int sheet, bool *truncated = nullptr);

} // namespace jp::office::sheet
