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
    enum Kind { Insertion, Deletion, MovedFrom, MovedTo, Format, ParagraphFormat, MarkInserted, MarkDeleted,
                // a paragraph's numbering that was added while changes were tracked (w:ins in w:numPr), or that was changed (w:numberingChange in w:numPr, with the old numbering as a label)
                NumberingInserted, NumberingChanged,
                // what a table records: a row or a cell inserted or deleted, cells merged, and a change of the properties of a table, a row, or a cell
                RowInserted, RowDeleted, CellInserted, CellDeleted, CellMerged, TableFormat, RowFormat, CellFormat };
    Kind kind = Insertion;
    QString id, author;
    QDateTime date;               // none (invalid) when the file gives none
    int from = 0, to = 0;         // the places in the story: the two pieces that mark a change of the text are the first and last character of it; a paragraph mark is the character after its text
    int block = 0;                // the number of the paragraph it begins in
    bool partner = false;         // set where changes are listed or resolved together: the second half of a move, and the paragraph marks that went with it (a move is shown once)
    bool moved = false;           // a paragraph mark (MarkInserted, MarkDeleted) that a move inserted or took away (w:moveTo, w:moveFrom), which goes with its move

    bool isDeletion() const { return kind == Deletion || kind == MovedFrom || kind == MarkDeleted || kind == RowDeleted || kind == CellDeleted; }
    bool inTable() const { return kind >= RowInserted; }
    // A change of how something looks (the marks beside the text call it formatting, not an insertion or a deletion).
    bool isFormatting() const { return kind == Format || kind == ParagraphFormat || kind == NumberingInserted || kind == NumberingChanged || kind == TableFormat || kind == RowFormat || kind == CellFormat; }
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
// The same by the order the changes are kept in (their place, then their kind, then their Id): the one after `after`, or before it. Changes that begin at one place (a table's records, and the text
// at its start) are each reached.
std::optional<Revision> nextRevision(const QTextDocument *story, const Revision &after);
std::optional<Revision> previousRevision(const QTextDocument *story, const Revision &before);
// The words of a change, for a list of them (at most `most` characters); a change of properties has none.
QString revisionText(const QTextDocument *story, const Revision &revision, int most = 120);
// The people who made changes in the story, in the order they first appear.
QStringList revisionAuthors(const QTextDocument *story);

// Accepts or rejects changes, all as one step of the history. Accepting a deletion takes the text out, and rejecting an insertion does; the other two keep it (and rejecting a change of properties gives
// the properties it recorded back). A deleted paragraph mark joins the paragraph to the next when accepted. Returns how many were done; `partners` (when given) gets how many of those were the second
// half of a move, or a paragraph mark that went with one (the lists show a move once, so the figure to tell a person is the return less these); `kept` (when given) gets how many changes of tables were
// settled by keeping the row, cell, or grid because taking it out (or giving the old grid back) would have broken the table: their records are gone, or stay when a grid can't be given back.
int resolveRevisions(WriteDoc &doc, QTextDocument *story, const QVector<Revision> &which, bool accept, int *kept = nullptr, int *partners = nullptr);
// The changes that touch [from, to] of the body of the text (the whole story with `to` below 0).
int resolveRevisionsIn(WriteDoc &doc, QTextDocument *story, int from, int to, bool accept, int *kept = nullptr, int *partners = nullptr);
int acceptAll(WriteDoc &doc, int *kept = nullptr, int *partners = nullptr);
int rejectAll(WriteDoc &doc, int *kept = nullptr, int *partners = nullptr);
// The change at the cursor (or the selection: every change it touches), then the one after it. `c` is put where the next change is when there is one, which may be in another
// story (a header, a footer, or a note): the cursor is the one of that story then. The cursor may be in the text, an endnote, a header, a footer, or a footnote.
int resolveAtCursor(WriteDoc &doc, QTextCursor &c, bool accept);

// ---- the changes of the whole document
// A change, and the story it is in: the text (the endnotes the pages set are in it), a footnote, a header, or a footer. `container` names the story ("body", "footnote:3",
// "header:1:0": the section, then the slot: odd, first, even; "footer:1:0"), for a list of changes to say where each is.
struct StoryRevision {
    QTextDocument *story = nullptr;
    Revision revision;
    QString container;
};
// Every change of the document in its order: the text, then the footnotes by number, then the headers and footers by section. At most `most`.
QVector<StoryRevision> documentRevisions(WriteDoc &doc, int most = kMostRevisions);
int documentRevisionCount(WriteDoc &doc);
// A number that is not the same after an edit of any story (the text, a header, a footer, or a note), for what keeps a list of the changes up to date.
qint64 reviewStamp(const WriteDoc &doc);
// How many changes are inside text boxes of the document's stories that are not listed: the words of a box in the text are a story of their own, with container "box:" and the place of its
// drawing, which the lists, Next and Previous, and Accept and Reject reach; the changes in a box that can't be read as words (a group, a chain of linked boxes, a box in a header or a note) are
// kept as the XML they are written in, so they are only counted, but Accept All and Reject All do them.
int textBoxChangeCount(WriteDoc &doc);
// Whether there is a change anywhere in the document (the text is looked through first, the other stories only when it has none).
bool documentHasRevisions(WriteDoc &doc);
// The story a change's `container` names, or none where the document has no such story now.
QTextDocument *storyOfContainer(WriteDoc &doc, const QString &container);
// The change of that story that begins at `from` and is of that kind and Id (never one only near it); none where there is none now.
std::optional<StoryRevision> findRevision(WriteDoc &doc, const QString &container, int kind, const QString &id, int from);
// The next change after `position` of `story` (or the one before it), going on into the stories after it (before it); none where there is none. Next and Previous Change use it. With `after`
// (the change last visited), the one after (before) that one in the story's order, whatever place it was at.
std::optional<StoryRevision> nextDocumentRevision(WriteDoc &doc, const QTextDocument *story, int position, bool forward, const StoryRevision *after = nullptr);
// The same from a place in the story that a container names, which may be gone now (a text box whose changes were all settled): the change after (before) the place, or, for a box that is
// gone, after (before) where it stood.
std::optional<StoryRevision> nextDocumentRevisionIn(WriteDoc &doc, const QString &container, int position, bool forward);
// The words of a text box that a person types in (a story made from the box when it is opened, with its paragraphs of deleted marks left out) have changes at places that the box's
// listed story has too, under other numbers: the change of the listed story that `r`, a change of the typed story, stands for (found by its kind and Id, and which of those it is). None where there is none.
std::optional<StoryRevision> boxRevisionLike(WriteDoc &doc, int boxPosition, const QTextDocument *typedIn, const Revision &r);
// Accepts or rejects changes of any stories, as one step of the history (the same one step as one story's). Returns how many were done.
int resolveDocumentRevisions(WriteDoc &doc, const QVector<StoryRevision> &which, bool accept, int *kept = nullptr, int *partners = nullptr);

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
// How many changes the XML holds (each element that records one: an insertion, a deletion, a move's half, a change of a run's or a paragraph's properties).
int xmlChangeCount(const QByteArray &xml);
// Whether the XML has a change of a table that xmlHasChanges does not name: of a table's, a row's, or a cell's properties, a grid, or a cell's insertion, deletion, or merge.
bool xmlHasTableChanges(const QByteArray &xml);
// The Ids of the marks of changes in blocks about to be written: every Id is used once, and none is the Id of a comment or a bookmark (the first mark that has an Id keeps it; the others, which
// splitting a paragraph and pasting copies make, get new ones).
void makeChangeIdsUnique(WriteDoc &doc, QVector<office::wml::Block> *blocks);
// For tests of cost: the paragraphs looked through for changes, and the runs looked at to give the words of a change or to resolve a change of properties, since the program began.
qint64 revisionBlocksScannedForTest();
qint64 revisionRunsVisitedForTest();
// How many people were looked at to find the places of the people the changes name (a count for the tests of its cost).
qint64 reviewerProbesForTest();
// How many times the list of the document's stories that can hold changes was made, since the program began.
qint64 siteListsBuiltForTest();

// Makes the look of the changes in the paragraph follow the marks it has (after the pieces that mark them were taken out or put in).
void refreshRevisionLook(WriteDoc &doc, QTextBlock block);

} // namespace jp::write
