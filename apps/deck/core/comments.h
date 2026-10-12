#pragma once
// A slide's comments, kept in its page props "comments" (so undo covers them):
//
//   [{"id", "author", "initials", "date" (ISO 8601, UTC), "text",
//     "x", "y" (points on the slide, where the comment's marker stands),
//     "replies": [{"id", "author", "initials", "date", "text"}],
//     "done": true once someone resolved it}]
//
// Ids are made for each comment and reply and are only meant to be told
// apart within a slide. The list is in the order the comments were made.

#include "core/document.h"

#include <QDateTime>
#include <QPointF>
#include <QString>
#include <QVector>
#include <optional>

namespace jp::deck {

struct CommentReply {
    QString id, author, initials;
    QDateTime date;   // UTC; invalid when the file gave none
    QString text;
};

struct Comment {
    QString id, author, initials;
    QDateTime date;
    QString text;
    QPointF at{10, 10};
    QVector<CommentReply> replies;
    bool done = false;
};

// The person at the keyboard, as File > Options keeps them (the settings
// user/name and user/initials). `name` is empty until someone says.
struct CommentAuthor {
    QString name, initials;
    bool known() const { return !name.trimmed().isEmpty(); }
};
CommentAuthor commentAuthor();
// Keeps the name and initials in the settings; initials left empty are made from the name.
void setCommentAuthor(const CommentAuthor &who);
// "Pat Doe" -> "PD": the first letter of up to three words, upper case.
QString initialsFor(const QString &name);

QVector<Comment> commentsOf(const Page &slide);
// Stores the list, or takes the key out of the props when it is empty.
void setComments(Page &slide, const QVector<Comment> &list);
int commentCount(const Page &slide);
// The comment with that id (not a reply's); empty when there is none.
std::optional<Comment> findComment(const Page &slide, const QString &id);

// A new comment, marker at `at`; returns its id. Text is kept as typed apart
// from the line ends, which become "\n".
QString addComment(Page &slide, const CommentAuthor &who, const QString &text, const QPointF &at, const QDateTime &when = QDateTime::currentDateTimeUtc());
// An answer to comment `commentId`; returns its id, empty when the slide has no such comment.
QString addReply(Page &slide, const QString &commentId, const CommentAuthor &who, const QString &text, const QDateTime &when = QDateTime::currentDateTimeUtc());
// New text for a comment or a reply (by its id). False when there is none.
bool editComment(Page &slide, const QString &id, const QString &text);
bool resolveComment(Page &slide, const QString &id, bool done = true);
bool moveComment(Page &slide, const QString &id, const QPointF &to);
// Takes away a comment with its replies, or one reply (by its id).
bool deleteComment(Page &slide, const QString &id);
// All of a slide's comments, or all of the presentation's; how many went.
int deleteComments(Page &slide);
int deleteAllComments(Document &d);

// Where the comments of the presentation stand, slide by slide in the order
// they were made, for Previous and Next.
struct CommentRef {
    int slide = -1;
    QString id;
    bool operator==(const CommentRef &o) const { return slide == o.slide && id == o.id; }
};
QVector<CommentRef> allComments(const Document &d);

// Fits a text to what a comment may hold and tidies its line ends.
QString commentText(const QString &text);
constexpr int kMaxCommentText = 32768;

} // namespace jp::deck
