#pragma once
// Compare for JeffDeck, with no window: what another presentation (`theirs`) changed from this one (`mine`), as a review::ChangeList, and the
// calls that take a change into the working presentation (accept) or keep this one's version of it (reject).
//
// Slides are matched by page id, then by the same title and text, then by how alike their words are (a posting list of the rarer words, so the
// work is a few hundred steps a slide however many slides there are), then by the same title. Slides left over are added or deleted; those
// matched out of order (all but a longest run that keeps its order) are moved. On matched slides, objects are matched by id, then by kind with
// the same text and place, the same text, the same role and place, and the same role. Changes found: a slide added, deleted, or moved; an
// object added, deleted, moved or resized, its text changed (the whole text, old and new; a text whose letters are the same but whose
// formatting is not is reformatted), its fill or line changed; notes changed; comments added (they come into the presentation, none is lost).
// A table whose shape (rows, columns, cells) is not the same is replaced: its old one deleted and its new one added. An object added goes after the
// object that comes before it in theirs that is in the working presentation (as slides do), so that the order of theirs comes out whatever order the
// changes are taken in. Pictures are compared by what they hold, not by the id each load gave them.
// Not found, left for later: changes of z-order among objects both have, pictures swapped, effects, replies added to a comment that was there.
//
// The working presentation starts as `mine`. Each accept or reject touches the working presentation only, and is meant to be wrapped in
// Editor::change() so that it is one undo step; Accept All and Reject All are well defined because the changes are taken in an order that
// makes the slides end up as `theirs` (or as `mine`). A change that is not accepted or rejected stays pending, in the list, which is not saved
// into any file yet.
//
// Limits (a file is hostile): at most kMaxCompareSlides slides and kMaxCompareObjects objects of each presentation are read, at most
// review::kMaxChanges changes are listed; past any of them `changes.tooMany` is set and the rest is not compared.

#include "core/document.h"
#include "text/review.h"

#include <QHash>
#include <QRectF>
#include <QSet>
#include <QString>
#include <QVector>
#include <memory>

namespace jp::deck {

constexpr int kMaxCompareSlides = 2000;
constexpr int kMaxCompareObjects = 20000;   // objects of each presentation, in all; a table's cells count as objects too
constexpr qint64 kMaxTotalText = 20000000;  // characters of text read from each presentation, in all
constexpr int kMaxSigRuns = 20000;          // paragraphs and runs of one object whose formatting is compared
constexpr qint64 kMaxTotalSigRuns = 200000; // paragraphs and runs whose formatting is compared, in all, for each presentation

// A slide of the working presentation, by its id. The first accept or reject makes every id the comparison relies on one of its own (a file may repeat
// an id or leave it out: the later ones are given new ids in the working presentation then), so an id is an address, not an ordinal.
struct CompareRef {
    QString id;
    bool valid() const { return !id.isEmpty(); }
};

// What the comparison knows of the working presentation once it has been given one (the first accept or reject). Not for callers.
struct CompareWork {
    bool bound = false;
    QVector<QString> pageStamp;                 // for each slide of mine, the id it was given when its own was repeated or empty
    int samplePage = -1;                        // one stamped slide and object, to see that the working presentation still has the stamps (an undo takes them away)
    QString samplePageId;
    int sampleSlide = -1;   // (the slide of that object)
    QString sampleId;
    QSet<QString> ids;                          // the ids of the objects of the working presentation, as far as the comparison has seen them
    QSet<QString> made;                         // the ids of objects the comparison put into the working presentation itself
    QHash<QString, int> hint;                   // an object's id: its last known place in its list
    QHash<QString, QString> theirsWork;         // "slide/path" of an object of theirs: the id of the working object that is it (matched, or taken in)
    QHash<QString, int> slideAt;                // a slide's id: its last known place in the working presentation
    bool slideFresh = false;                    // slideAt was made since the slides last changed
};

struct DeckComparison {
    review::ChangeList changes;
    // What was read, and the work done, for tests: slides and objects of each side (each capped), and the steps matching took (a look at a
    // table, or at a slide that shares a word).
    int slidesMine = 0, slidesTheirs = 0, objectsMine = 0, objectsTheirs = 0;
    qint64 work = 0;
    // The characters of text read, and the runs whose formatting was compared, from each presentation.
    qint64 textMine = 0, textTheirs = 0, sigRunsMine = 0, sigRunsTheirs = 0;
    // `mine` and `theirs` as they were when the comparison was made, kept so that a change can be taken or rejected whatever becomes of the caller's
    // presentations (the pictures and movies are shared with them, not copied).
    std::unique_ptr<Document> mine;
    std::unique_ptr<Document> theirs;
    // Slide pairing: for each slide of `theirs` the slide of `mine` it was matched with (-1 for none), and the other way round.
    QVector<int> mineOfTheirs, theirsOfMine;
    // Where each slide of either side is in the working presentation now.
    QVector<CompareRef> workOfMine, workOfTheirs;
    CompareWork state;
};

DeckComparison compareDecks(const Document &mine, const Document &theirs);

// Takes the other presentation's side of change `index` into `working`: a slide or object added comes in, one deleted goes, a moved one moves,
// a text or format changes to theirs, comments come in. It looks at the working presentation and not at the change's state, so it is safe to
// call again (after an undo). False when the change is not there, or the working presentation has nothing it can be applied to (the last slide
// is not deleted); the state is then not changed.
bool applyChange(Document &working, DeckComparison &cmp, int index);
// Keeps this presentation's side: a change still pending only becomes Rejected (nothing has changed in `working`); one accepted before is taken
// back. False as for applyChange.
bool rejectChange(Document &working, DeckComparison &cmp, int index);
// Every pending change, in the order that gives `theirs` (slides: added and moved in the order of theirs, then deleted). How many were taken.
int acceptAll(Document &working, DeckComparison &cmp);
// Every pending change (and, with `alsoAccepted`, the accepted ones too, so that the working presentation is `mine` again), in the order that
// gives `mine`. How many were rejected.
int rejectAll(Document &working, DeckComparison &cmp, bool alsoAccepted = false);

// Where change `index` shows in the working presentation, for the marks on the slides: the slide it is on (-1 when it is on none now: a slide
// added that is not taken in yet), and the object's place on the page when it is about an object (one added that is not taken in yet is where it
// is in theirs). A change not found gives slide -1 and no rectangle.
struct ChangeMark {
    int slide = -1;
    QRectF rect;
    bool hasRect = false;
};
ChangeMark markOf(const Document &working, DeckComparison &cmp, int index);
// The whole text of an object before (mine) and after (theirs) a text change, for the word-level view; false for a change that is not one.
bool textChangeOf(const DeckComparison &cmp, int index, QString *oldText, QString *newText);
// What tells a change apart across a new comparison and a new reading of the file (ids are made again each time): kind, what changed, where its slide and object are in the other presentation (kept whole), and a hash of the changed content; no words of the interface.
QString changeKey(const review::Change &c);

} // namespace jp::deck
