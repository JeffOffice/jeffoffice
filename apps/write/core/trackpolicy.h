#pragma once
// What each command of the window does while Track Changes is on. A command whose change is not recorded as a tracked change yet works as it does with Track Changes off, and the status bar
// says that the change was not recorded; every command is named here, and one that is not named is Untracked. The probe test in tests/write/tst_write_review.cpp runs every command that
// is not Neutral with tracking on and checks that it does what it does with tracking off, makes no mark, and says so.

#include "core/writedoc.h"

#include <QString>

namespace jp::write {

enum class TrackPolicy {
    Neutral,        // does not change the document (it looks, finds, saves, opens, or only changes how changes are shown)
    Tracked,        // changes the document and the change is recorded (the caret may be anywhere the text is)
    TrackedInText,  // as Tracked, but in a table's cell the change is not recorded yet (it is made, and said so)
    Untracked       // changes the document and the change is not recorded yet: it is made as with Track Changes off, and the status bar says it was not recorded
};

// The policy of a command by its id. `known` (if given) says whether the id is named: a command that is not named is Untracked.
TrackPolicy trackPolicyOf(const QString &commandId, bool *known = nullptr);

// Whether the person can accept or reject a tracked change themselves: not where the document's protection locks tracking on or holds it for reading only (Accept and Reject are off there).
bool trackedMarksCanBeResolved(const WriteDoc &doc);

// The notice for a change that is made and not recorded as a tracked change.
QString untrackedNoticeText();

// The message for a change that is not made because it would take part of a tracked row.
QString trackedRowsText();

} // namespace jp::write
