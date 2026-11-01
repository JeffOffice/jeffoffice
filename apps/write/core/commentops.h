#pragma once
// Comments as a person works with them: listing them, adding one to the selection, replying, resolving, and deleting. A comment of a file is in four parts (the comments themselves, which
// of them are replies and which are done, their durable Ids, and the people), and in the text as three marks: where its range begins and ends, and the mark of the reference. The text keeps
// the marks (so that a step of the history takes a comment with it), and the reference mark of a comment that was made or changed here also keeps what the comment says, so that Undo
// brings it back; the parts of a file are read as they are, and are written again when a comment was made, changed, or taken out.

#include "core/writedoc.h"

#include <QDateTime>
#include <optional>

namespace jp::write {

struct Comment {
    int id = -1;
    QString author, initials;
    QDateTime date;               // none (invalid) when the file gives none
    QString text;                 // the comment's paragraphs, a line each
    QString paraId;               // the w14:paraId of its last paragraph, which the comment extensions name it by
    int parent = -1;              // the Id of the comment it replies to, -1 for none
    bool done = false;            // resolved
    int from = -1, to = -1;       // the range of the story it covers (equal for one at a point); none (-1) when the story has no mark of the range
    int reference = -1;           // where the mark of its reference is
    bool changed = false;         // made or changed in this program: the mark of its reference holds it
};

// Notes which comments the text refers to as the document is opened (a comment taken out afterwards is taken out of the file's parts when it is saved).
void rememberCommentsAtOpen(WriteDoc &doc);

// The Ids of every comment the document has: those of the file's comments part (referred to or not) and of the marks in the text.
QSet<int> allCommentIds(WriteDoc &doc);
// For tests of cost: the paragraphs looked through for the marks of comments since the program began.
qint64 commentBlocksScannedForTest();
// How many steps along a path of replies the settling of the comments took (a count for the tests of its cost).
qint64 commentReplyStepsForTest();

// The most comments looked at (a hostile file has more than anyone reads).
constexpr int kMostComments = 100000;

// The comments the text refers to, in the order of their references. A reference to a comment the file lacks is none. A reply to a comment that is not there, or that makes a loop of replies, is
// a comment of its own.
QVector<Comment> comments(WriteDoc &doc);
std::optional<Comment> commentById(WriteDoc &doc, int id);
// The comments (not their replies) whose references are in [from, to] of the text, with the places of their references only: what a page shows of them, found by looking through that stretch alone.
QVector<Comment> commentsIn(WriteDoc &doc, int from, int to);
// The comment whose range holds the place (the innermost), or whose reference is there.
std::optional<Comment> commentAt(WriteDoc &doc, int position);
// The next comment after the place (the first whose range begins after it), the previous before it; none where there is none (they do not wrap round).
std::optional<Comment> nextComment(WriteDoc &doc, int position);
std::optional<Comment> previousComment(WriteDoc &doc, int position);
// The comments that reply to one, directly, in order.
QVector<Comment> repliesTo(WriteDoc &doc, int id);

// A comment on the selection of `c` (at the caret, when nothing is selected), by the reviewer, now. Returns its Id; -1 where there can be none (not in the text, or in a paragraph that stands for a
// piece kept as XML). One step of the history.
int addComment(WriteDoc &doc, QTextCursor &c, const QString &text);
// A reply to a comment, by the reviewer, now: its range is the comment's. One step. Returns its Id, -1 for none.
int replyToComment(WriteDoc &doc, int parentId, const QString &text);
// Resolves or reopens a comment with its replies. One step. False where there is no such comment.
bool setCommentDone(WriteDoc &doc, int id, bool done);
// Changes what a comment says. One step.
bool setCommentText(WriteDoc &doc, int id, const QString &text);
// Takes a comment and its replies out (the marks of their ranges and references). One step. Returns how many were taken out.
int deleteComment(WriteDoc &doc, int id);
int deleteAllComments(WriteDoc &doc);

} // namespace jp::write
