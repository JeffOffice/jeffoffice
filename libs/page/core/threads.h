#pragma once
// The rules of comment threads that every app shares (JeffPub and JeffDeck's comments on pages, JeffWrite's comments on ranges, JeffCrunch's threads on cells), so that a thread
// behaves the same wherever it is made. What each app keeps and how it writes it stays its own; what it may hold and how people move through it is here:
//  - a thread is its first comment and the replies to it (at most kMaxReplies); a reply to a reply goes into the same thread, after the others;
//  - resolving or reopening applies to the whole thread; deleting a reply takes that reply out, deleting the first comment takes the thread;
//  - a person may start at most kMaxThreads threads in a document and write at most kMaxChars characters in one comment;
//  - Previous and Next walk the threads in document order (a slide's order, a story's order, a workbook's sheet-row-column order) and do not wrap;
//  - the author and initials are File > Options' (the settings user/name and user/initials, jp::comments::commentAuthor()).
// What a file may hold when it is read stays with each reader (comments.h limitComments for slides, the sheet reader's own); these are what an edit may make.

#include <QString>
#include <QVector>
#include <functional>

namespace jp::threads {

constexpr int kMaxThreads = 2000;      // threads in one document
constexpr int kMaxReplies = 200;       // replies in one thread, not counting its first comment
constexpr int kMaxChars = 32767;       // characters in one comment or reply (a cell's own limit)

// The caps in force: the constants above unless a test lowers them (0 puts one back). Only tests call set.
int maxThreads();
int maxReplies();
void setCapsForTest(int threads, int replies);

enum class Refusal { None, Empty, TooLong, TooManyThreads, TooManyReplies };

// Whether a thread may be started (`threadsNow` are there), a reply added (`repliesNow` are in the thread), or a comment's words changed. `needWords` refuses text that is only
// space: the apps that ask a person for words say so.
Refusal checkNewThread(const QString &text, int threadsNow, bool needWords = false);
Refusal checkReply(const QString &text, int repliesNow, bool needWords = false);
Refusal checkText(const QString &text, bool needWords = false);
// The words to tell a person, translated ("" for None).
QString refusalText(Refusal r);

// Line ends as one "\n".
QString tidyText(const QString &text);

// Previous and Next: of `keys` (each thread's place in document order, in any order), the index of the thread just after `from` (the least key above it) or just before it (the greatest
// below it); -1 where there is none.
int stepIndex(const QVector<qint64> &keys, qint64 from, bool forward);

// The first comment of the thread `id` is in: follows `parentOf` (the id a comment replies to, empty for a first comment) up, at most `most` steps, so that a loop of replies ends.
// Returns `id` itself when it has no parent.
QString threadRoot(const QString &id, const std::function<QString(const QString &)> &parentOf, int most = 100000);

} // namespace jp::threads
