#pragma once
// The threads of comments on a worksheet's cells (the newer kind of comment, with replies): what a thread holds, how it reads as words, and how it is told to older programs, which see
// only a note that stands for it. Reading a file's threads is notes.h; writing them is threadsave.h.

#include <QString>

#include <memory>
#include <vector>

namespace jp::office::sheet {

// One comment of a thread: the first, or a reply. `time` is as the file writes it. `extra` is what the file has after the words (who is mentioned, and extensions), kept as it was.
struct ThreadComment {
    QString id, parentId, personId, time;
    QString author, userId, providerId;   // the person: the name shown, and the ids the file's list of people has for them
    QString text;
    QString extra;
    bool done = false;                    // resolved (the first comment of a thread carries it)
    bool operator==(const ThreadComment &o) const
    {
        return id == o.id && parentId == o.parentId && personId == o.personId && time == o.time && author == o.author && userId == o.userId && providerId == o.providerId && text == o.text && extra == o.extra
               && done == o.done;
    }
};
using ThreadList = std::vector<ThreadComment>;
using ThreadPtr = std::shared_ptr<const ThreadList>;

// The most comments one thread holds (a file with more has the first of them, shown and kept, and not changed), the most in a workbook, and the most characters of one comment's extra.
constexpr int kMaxThreadComments = 500;
constexpr qint64 kMaxThreadCommentsPerBook = 400000;
constexpr int kMaxThreadExtraChars = 4000;
// The most a comment's words and a new comment's words may hold.
constexpr int kMaxThreadCommentChars = 32767;
// The most characters of the older note that stands for a thread (a cell holds 32,767), and of a thread's words as a note shows them.
constexpr int kMaxLegacyNoteChars = 32767;
constexpr qint64 kMaxThreadWordsChars = qint64(8) << 20;

// Whether two threads are the same (null is a thread with no comments).
bool sameThread(const ThreadPtr &a, const ThreadPtr &b);
// The thread's words, the way a note stands for it: each comment under the other, with the name of who wrote it.
QString threadWords(const ThreadList &thread);
// What an older program that cannot show threads shows in the thread's place.
QString legacyThreadText(const ThreadList &thread);
// The name the file's older note for a thread carries as its author: "tc=" and the id of the thread's first comment.
QString legacyThreadAuthor(const ThreadList &thread);
// Whether the thread is resolved.
bool threadResolved(const ThreadList &thread);
// How many replies deep comment `index` is (0 for the first; the walk up stops at 32, so a loop of parents ends).
int threadDepth(const ThreadList &thread, size_t index);
// The depth of every comment of the thread, worked out together (the same numbers, in time that grows with the length of the thread, not with its square).
std::vector<int> threadDepths(const ThreadList &thread);
// A new id for a comment or a person: "{" and 36 characters of a random id in capitals and "}".
QString newThreadId();
// The time now as a comment writes it.
QString threadTimeNow();

} // namespace jp::office::sheet
