#pragma once
// Tracked changes as a person works with them: finding them in the story, accepting and rejecting them, and the marks that new ones are made with (core/editops.cpp tracks typing,
// deleting, and formatting with them). A change is kept in the story as the file keeps it: an insertion or a deletion is a pair of invisible pieces kept as XML (w:ins, w:del, w:moveFrom,
// w:moveTo) round the text it holds, a change of a run's properties is the record in the run's kept properties (w:rPrChange), a change of a paragraph's is in the paragraph's (w:pPrChange), and
// a paragraph mark that was inserted or deleted says so in the mark's kept properties. What the lines mark (tp::Revision) is only the look of them, and follows these.

#include "core/writedoc.h"

#include <QDateTime>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <optional>

namespace jp::write {

struct Revision {
    enum Kind { Insertion, Deletion, MovedFrom, MovedTo, Format, ParagraphFormat, MarkInserted, MarkDeleted };
    Kind kind = Insertion;
    QString id, author;
    QDateTime date;               // none (invalid) when the file gives none
    int from = 0, to = 0;         // the places in the story: the two pieces that mark a change of the text are the first and last character of it; a paragraph mark is the character after its text
    int block = 0;                // the number of the paragraph it begins in

    bool isDeletion() const { return kind == Deletion || kind == MovedFrom || kind == MarkDeleted; }
    bool operator==(const Revision &o) const { return kind == o.kind && id == o.id && from == o.from && to == o.to; }
};

// The most changes that are looked for at once (a hostile file has more than anyone looks through).
constexpr int kMostRevisions = 2000000;

// The changes that touch [from, to] of the story, in order (the whole story with `to` below 0), at most `most` of them.
QVector<Revision> revisionsIn(const QTextDocument *story, int from = 0, int to = -1, int most = kMostRevisions);
// How many changes the story has.
int revisionCount(const QTextDocument *story);
// The change the place is in or touches (a deletion or an insertion it is inside of, the run it is in whose properties were changed...), the innermost; none where there is none.
std::optional<Revision> revisionAt(const QTextDocument *story, int position);
// The next change after `position` (the first that begins after it), or the previous before it (the last that begins before it); none where there is none. They do not wrap round.
std::optional<Revision> nextRevision(const QTextDocument *story, int position);
std::optional<Revision> previousRevision(const QTextDocument *story, int position);
// The words of a change, for a list of them (at most `most` characters); a change of properties has none.
QString revisionText(const QTextDocument *story, const Revision &revision, int most = 120);
// The people who made changes in the story, in the order they first appear.
QStringList revisionAuthors(const QTextDocument *story);

// Accepts or rejects changes, all as one step of the history. Accepting a deletion takes the text out, and rejecting an insertion does; the other two keep it (and rejecting a change of properties gives
// the properties it recorded back). A deleted paragraph mark joins the paragraph to the next when accepted. Returns how many were done.
int resolveRevisions(WriteDoc &doc, QTextDocument *story, const QVector<Revision> &which, bool accept);
// The changes that touch [from, to] of the body of the text (the whole story with `to` below 0).
int resolveRevisionsIn(WriteDoc &doc, QTextDocument *story, int from, int to, bool accept);
int acceptAll(WriteDoc &doc);
int rejectAll(WriteDoc &doc);
// The change at the cursor (or the selection: every change it touches), then the one after it. `c` is put where the next change is when there is one.
int resolveAtCursor(WriteDoc &doc, QTextCursor &c, bool accept);

// ---- the marks that make a change
// A format with the look of the tracked change it is in; the marks of a deletion hide the text as hidden text is.
QTextCharFormat withRevisionLook(QTextCharFormat format, bool deleted, int kind, int author, bool alsoInserted = false);
// The start tag of a w:ins, w:del, w:moveFrom, or w:moveTo, with an Id of its own, the person, and the time now.
QByteArray revisionStartTag(WriteDoc &doc, const QString &tag);
// A piece for the story that begins or ends a change (the format is the invisible piece kept as XML, in the look of `like`).
QTextCharFormat revisionPiece(const QByteArray &startTag, const QString &tag, bool open, const QTextCharFormat &like);
// The start tags of the changes that a paragraph's text is inside at `position` (outermost first), as a list of (tag, start tag).
QVector<std::pair<QString, QByteArray>> openRevisionsAt(const QTextBlock &block, int position);
// The person a start tag names, and the time.
QString revisionAuthorOf(const QByteArray &startTag);
QDateTime revisionDateOf(const QByteArray &startTag);
// Whether the text of [from, to) in a paragraph is all inside insertions by `author` (text that person typed and may take out for good).
bool isOwnInsertion(const QTextDocument *story, int from, int to, const QString &author);

// ---- making changes (Track Changes on; core/editops.cpp calls these)
// Which of [from, to) of a paragraph's text a person's deletion takes out for good (text that same person typed as a tracked insertion, and nothing else of it is then recorded) and which it only
// marks as deleted (the rest, but for what is deleted already and the pieces that mark changes). Runs, in order, each of text that is one level of the changes.
struct DeletionPlan {
    QVector<std::pair<int, int>> remove, mark;
};
DeletionPlan planDeletion(const QTextBlock &block, int from, int to, const QString &author);
// Marks the text [from, to) of one paragraph as deleted by the reviewer: it is put between the pieces of a w:del (joined to the one beside it if that is the reviewer's), and given the look of a
// deleted text. Where it is inside someone's insertion, the deletion is inside it.
void markDeleted(WriteDoc &doc, QTextDocument *story, int from, int to);
// The runs of a paragraph's text that are inside insertions by `author`, in order: (first place, end), the text itself.
QVector<std::pair<int, int>> ownInsertionRanges(const QTextBlock &block, const QString &author);
// Takes text the reviewer typed out for good, and the pairs of pieces of insertions that nothing is left in.
void removeOwnInsertion(QTextDocument *story, int from, int to);
// A paragraph's mark said to be inserted ("ins") or deleted ("del") by the reviewer; the mark of a paragraph the reviewer inserted is taken away for good by removeParagraphMark.
void markParagraphMark(WriteDoc &doc, QTextBlock block, const char *tag);
bool paragraphMarkInsertedBy(const QTextBlock &block, const QString &author);
// Takes a paragraph's mark away: the paragraph is joined to the one after it (as when a deleted mark is accepted). False where it can't be (the mark stays, without its record).
void removeParagraphMark(WriteDoc &doc, QTextDocument *story, int blockNumber);
// What the place in a paragraph is inside of, for the look of text put there: whether it is in a deletion, and the kind (1 inserted, 2 deleted, 3 moved away, 4 moved here) and the person of the
// change that marks it.
struct RevisionLook {
    bool deleted = false;
    int kind = 0;
    QString author;
};
RevisionLook revisionLookAt(const QTextBlock &block, int position);
// A format that records a change of the run's properties by the reviewer: `now` with the record that its properties were `old` (a run that is the reviewer's own insertion, or already has a record,
// is as it is). `paragraphStyle` is the id of the style of its paragraph.
QTextCharFormat withFormatRecord(WriteDoc &doc, const QString &paragraphStyle, const QTextCharFormat &old, QTextCharFormat now);
// The same for a paragraph's properties.
QTextBlockFormat withParagraphRecord(WriteDoc &doc, const QTextBlockFormat &old, QTextBlockFormat now);

// The same for a piece of XML that is kept as written (the content of a text box): its changes accepted or rejected, the XML given back (as it was when it has none or can't be read).
QByteArray resolveChangesInXml(const QByteArray &xml, bool accept);
bool xmlHasChanges(const QByteArray &xml);
// The Ids of the marks of changes in blocks about to be written: every Id is used once, and none is the Id of a comment or a bookmark (the first mark that has an Id keeps it; the others, which
// splitting a paragraph and pasting copies make, get new ones).
void makeChangeIdsUnique(WriteDoc &doc, QVector<office::wml::Block> *blocks);
// For tests of cost: the paragraphs looked through for changes, and the runs looked at to give the words of a change or to resolve a change of properties, since the program began.
qint64 revisionBlocksScannedForTest();
qint64 revisionRunsVisitedForTest();
// How many people were looked at to find the places of the people the changes name (a count for the tests of its cost).
qint64 reviewerProbesForTest();

// Makes the look of the changes in the paragraph follow the marks it has (after the pieces that mark them were taken out or put in).
void refreshRevisionLook(WriteDoc &doc, QTextBlock block);

} // namespace jp::write
