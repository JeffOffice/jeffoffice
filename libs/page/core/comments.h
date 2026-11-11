#pragma once
// A page's (a slide's) comments, kept in its page props "comments" (so undo covers them):
//
//   [{"id", "author", "initials", "date" (ISO 8601, UTC), "text",
//     "x", "y" (points on the page, where the comment's marker stands),
//     "item": the id of the object the comment is attached to (left out for a
//       comment on a place of the page), whose top right corner the marker
//       follows while the object is there; x and y are where it was,
//     "replies": [{"id", "author", "initials", "date", "text"}],
//     "done": true once someone resolved it}]
//
// Ids are made for each comment and reply and are only meant to be told
// apart within a page. The list is in the order the comments were made.

#include "core/document.h"

#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QPointF>
#include <QString>
#include <QVector>
#include <optional>

namespace jp::comments {

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
    QString itemId;   // the object the comment is attached to, if any
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

// A new comment, marker at `at` (attached to object `itemId` when that is given); returns its id. Text is kept as typed apart
// from the line ends, which become "\n".
QString addComment(Page &slide, const CommentAuthor &who, const QString &text, const QPointF &at, const QDateTime &when = QDateTime::currentDateTimeUtc(), const QString &itemId = QString());
// Where the comment's marker stands on its page: at the top right corner of the object it is attached to (looked for on that page only), else where it was put.
QPointF commentPlace(const Page &page, const Comment &c);

// What drawing and clicking a page's markers need of each comment, with the place resolved: made once for a page and kept until its comments or objects change.
struct CommentMark {
    QString id, author, initials;
    QPointF place;
    bool done = false, attached = false;   // attached: the marker stands on an object that is on the page
};
// One scan of the page's objects at most, however many comments are attached to objects (an object that is not there is found out once, in a table).
QVector<CommentMark> commentMarks(const Page &page);
// For tests: how many lists of marks, and how many scans of a page's objects, have been made.
int commentMarkBuilds();
int commentItemScans();
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
// What a document read from a file may hold: comments in all, replies to one, and (kMaxCommentText) characters in one.
constexpr int kMaxComments = 2000;
constexpr int kMaxReplies = 200;
// ... and all the replies of the document, and all the characters of the texts of its comments and replies.
constexpr int kMaxTotalReplies = 20000;
constexpr int kMaxTotalText = 8 * 1024 * 1024;
// How far outside the page a marker may stand.
constexpr double kPlaceMargin = 100;
// Holds the comments of a document from a file (a .jpub, a .pptx) to those limits (the order is kept, and the first ones stay) and tidies them: what is not a comment goes; a place
// that is not a number is the default one and any place is brought to the page and its margin; an id that is empty or used again is made anew; an object that is not on the page
// is let go (the comment keeps its place). Returns how many comments and replies went.
int limitComments(Document &d);

// While it lives, the authors' names and initials of every comment and reply of the document are empty (when `on`), and come back when it ends: for writing a file with no names in
// it (Remove personal information when saving). The document is not changed as far as a person can tell: no step of the history, and it is not marked as changed.
class NamesOutScope {
public:
    NamesOutScope(Document &d, bool on);
    ~NamesOutScope();
    NamesOutScope(const NamesOutScope &) = delete;
    NamesOutScope &operator=(const NamesOutScope &) = delete;

private:
    Document &m_doc;
    bool m_on;
    QVector<QJsonArray> m_kept;   // each page's comments as they were
};

// Change Template: the comments of each page of `from` go to the page of `to` with the same number (the last page, when there are fewer). An object the comments were attached to
// is a new copy in the new design: the comment lets go of it and keeps its place.
void carryComments(const Document &from, Document &to);
// Duplicate Page: the comments of `from` are copied to `to` with ids of their own, each attached to the copy of its object (`copies` maps an object's id to its copy's).
void copyComments(const Page &from, Page &to, const QHash<QString, QString> &copies);

} // namespace jp::comments
