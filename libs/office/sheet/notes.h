#pragma once
// The notes (the comments a person writes on a cell) that a worksheet's part has: by cell, the note's text, who wrote it, and whether the file shows it all the time. The notes of a
// file are read here; writing them is notesave.h. A note that belongs to a thread of comments is given its text from the thread and marked, so that it can be shown and kept but not
// changed. A damaged part gives what was read before the damage.

#include "office/package.h"
#include "sheet/sheetreader.h"

#include <QString>

#include <map>
#include <vector>

namespace jp::office::sheet {

// The most notes and characters read from one sheet, and the largest comments part looked at. (A sheet read for editing has at most kMaxModelNotes of them: what is past it is in the file, and
// stays as it is, but is not followed through edits to rows and columns, so a sheet that has more can't have its notes changed.)
constexpr qint64 kMaxNotes = 100000;
constexpr qint64 kMaxModelNotes = 20000;
constexpr qint64 kMaxNoteChars = qint64(8) << 20;
constexpr qint64 kMaxNotesPartBytes = qint64(64) << 20;
// The most of a note's drawing (the file's shapes for its notes) and of a thread part that is read.
constexpr qint64 kMaxNotesDrawingBytes = qint64(32) << 20;
// The most characters of notes, authors, and names of people read in a whole workbook, and of the names of people (and the most of them).
constexpr qint64 kMaxNoteCharsPerBook = qint64(32) << 20;
constexpr int kMaxPersons = 10000;
constexpr int kMaxPersonChars = 200;

// What reading the notes of a workbook's sheets one after another shares: a comments part that more than one sheet names is read once, the people of the threads are read once, and the
// characters read are counted for the whole workbook.
struct NoteInfo;
struct NoteReadContext;

struct NoteInfo {
    QString author;
    QString text;
    bool shown = false;      // the file shows the note all the time (not only when the pointer is on its cell)
    bool threaded = false;   // the text is that of a thread of comments: shown, kept, and not changed
    bool operator==(const NoteInfo &o) const { return author == o.author && text == o.text && shown == o.shown && threaded == o.threaded; }
};

struct NoteReadContext {
    std::map<QString, std::map<quint64, NoteInfo>> parts;   // by the comments part's name
    std::map<QString, QString> persons;
    bool personsRead = false;
    qint64 chars = 0;
    bool exhausted = false;                                  // the budget of characters is spent: nothing more is read
};

// The key a note is kept by: `(row << 32) | col`.
inline quint64 noteKey(quint32 row, quint32 col) { return (quint64(row) << 32) | col; }

// The notes of the sheet (by its place in the workbook) as the file has them. Empty for a sheet with none, a sheet that was added, and a workbook that has no package of XML parts.
// `truncated`, if given, is told whether a limit stopped the read.
// With a `context`, what an earlier call read is not read again, and the budget of characters is the workbook's.
std::map<quint64, NoteInfo> readSheetNoteInfos(const Workbook &book, int sheet, bool *truncated = nullptr, qint64 most = kMaxNotes, NoteReadContext *context = nullptr);

// How many times a sheet's notes have been read from a file since the program began (a test counts them to see that a search does not read them again and again).
qint64 noteReads();
// Just the texts (what Find looks in).
std::map<quint64, QString> readSheetNotes(const Workbook &book, int sheet, bool *truncated = nullptr);

// The part of the package that holds the shapes of a sheet's notes (the sheet's drawing that has a shape of the kind a note has), or empty if the sheet has none. Of a sheet's
// drawings of this kind the one for the header and footer pictures is not it. `bytes`, if given, gets the part (not larger than kMaxNotesDrawingBytes, else empty is returned).
QString notesDrawingPart(const Package &package, const QString &sheetPart, QByteArray *bytes = nullptr);

// The notes' shapes in a drawing's bytes: by cell, whether the shape is shown all the time. Shapes that are not a note's are not in it. At most kMaxNotes.
std::map<quint64, bool> noteShapes(const QByteArray &vml);

// The shapes of a drawing (its `shape` elements that are not inside another), each from its start tag to its end, in order; at most `most`.
struct DrawingShape {
    qsizetype start = 0, end = 0;
};
std::vector<DrawingShape> drawingShapes(const QByteArray &vml, qint64 most = kMaxNotes * 4);
// What a shape's bytes say of a note: the cell (from 0) its `ClientData` of the kind Note names, and whether it is shown all the time. False for a shape that is not a note's.
bool shapeNote(const QByteArray &vml, const DrawingShape &shape, quint32 *row, quint32 *col, bool *shown);

} // namespace jp::office::sheet
