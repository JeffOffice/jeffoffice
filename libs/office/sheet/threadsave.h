#pragma once
// Writing the threads of comments of a worksheet into the file it was read from: the thread part (each comment, with its place, time, person, and reply-to) and the workbook's list of people.
// A change names the cells it is to (the same NoteOp as the notes): the comments a cell has in the part are dropped, and those the change carries are put at the end. The part's other
// comments are left exactly as the file has them. The older note that stands for each thread is written by notesave.h.

#include "sheet/notesave.h"
#include "sheet/threads.h"

#include <QByteArray>
#include <QString>

#include <vector>

namespace jp::office::sheet {

struct ThreadPerson {
    QString id, name, userId, providerId;
};

// The people the threads of the changes name (each once, by id).
std::vector<ThreadPerson> threadPersons(const std::vector<NoteOp> &ops);

// The thread part with the changes put in; `changed` says whether the bytes differ. False, with a message, if the part is not XML this can change.
bool applyThreadOpsToPart(const QByteArray &xml, const std::vector<NoteOp> &ops, QByteArray *out, bool *changed, QString *why = nullptr);
// A thread part for a sheet that had none.
QByteArray newThreadsPart(const std::vector<NoteOp> &ops);
// Whether the changes put any thread.
bool opsHaveThreads(const std::vector<NoteOp> &ops);

// The list of people with the ones it lacks added.
bool applyPersonsToPart(const QByteArray &xml, const std::vector<ThreadPerson> &persons, QByteArray *out, bool *changed, QString *why = nullptr);
QByteArray newPersonsPart(const std::vector<ThreadPerson> &persons);

} // namespace jp::office::sheet
