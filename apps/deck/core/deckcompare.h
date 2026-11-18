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
// Not found, left for later: changes of z-order, pictures swapped, effects, replies added to a comment that was there.
//
// The working presentation starts as `mine`. Each accept or reject touches the working presentation only, and is meant to be wrapped in
// Editor::change() so that it is one undo step; Accept All and Reject All are well defined because the changes are taken in an order that
// makes the slides end up as `theirs` (or as `mine`). A change that is not accepted or rejected stays pending, in the list, which is not saved
// into any file yet.
//
// Limits (a file is hostile): at most kMaxCompareSlides slides and kMaxCompareObjects objects of each presentation are read, at most
// review::kMaxChanges changes are listed; past any of them `changes.tooMany` is set and the rest is not compared.

#include "core/document.h"
#include "core/review.h"

#include <QString>
#include <QVector>
#include <memory>

namespace jp::deck {

constexpr int kMaxCompareSlides = 2000;
constexpr int kMaxCompareObjects = 20000;

// A slide or object of the working presentation as its id and which one of those with that id (a file may repeat an id).
struct CompareRef {
    QString id;
    int ord = -1;   // -1: none
    bool valid() const { return ord >= 0; }
};

struct DeckComparison {
    review::ChangeList changes;
    // What was read, and the work done, for tests: slides and objects of each side (each capped), and the steps matching took (a look at a
    // table, or at a slide that shares a word).
    int slidesMine = 0, slidesTheirs = 0, objectsMine = 0, objectsTheirs = 0;
    qint64 work = 0;
    // `mine` as it was when the comparison was made, kept so that a rejected change can bring its version back; `theirs` belongs to the caller
    // and must stay while the changes are used.
    std::unique_ptr<Document> mine;
    const Document *theirs = nullptr;
    // Slide pairing: for each slide of `theirs` the slide of `mine` it was matched with (-1 for none), and the other way round.
    QVector<int> mineOfTheirs, theirsOfMine;
    // Where each slide of either side is in the working presentation now.
    QVector<CompareRef> workOfMine, workOfTheirs;
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

} // namespace jp::deck
