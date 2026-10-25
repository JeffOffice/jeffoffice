#pragma once
// Writing the notes of a worksheet into the file it was read from: the comments part (the text and author of each note) and the drawing that has a shape for each (where it sits and
// whether it is shown all the time). A change names the cells it is to: a note put (new, or replacing the one there) or taken away. The notes the change does not name are left exactly as
// the file has them (their formatted text, their shapes, their sizes), and so are a thread's comments, which are not changed here.

#include <QByteArray>
#include <QString>

#include <vector>

namespace jp::office::sheet {

struct NoteOp {
    quint32 row = 0, col = 0;
    bool remove = false;      // the note on the cell is taken away (else it is put: made, or replaced)
    QString author, text;
    bool shown = false;       // shown all the time
};

// The most operations one sheet's change may hold, and the most bytes of a note's text.
constexpr qint64 kMaxNoteOps = 100000;

// The comments part with the changes put in (sorted by row, then column; none twice). False, with a message, if the part is not XML this can change.
bool applyNoteOpsToComments(const QByteArray &xml, const std::vector<NoteOp> &ops, QByteArray *out, QString *why = nullptr);
// A comments part for a sheet that had none, with the notes that are put (the ones taken away are not in it).
QByteArray newCommentsPart(const std::vector<NoteOp> &ops);
// The notes' drawing with the changes put in: a shape made for a note that is put where there is none, the shape of one taken away dropped, and whether a note is shown set.
bool applyNoteOpsToDrawing(const QByteArray &vml, const std::vector<NoteOp> &ops, QByteArray *out, QString *why = nullptr);
// A drawing for a sheet that had none.
QByteArray newNotesDrawing(const std::vector<NoteOp> &ops);

} // namespace jp::office::sheet
