#pragma once
// What each command of the window does while Track Changes is on. A command that changes the document and is not recorded as a change would change it with no mark, which is worse than
// turning it down, so every command is named here and a command that is not named is turned down (as with the commands a read-only document allows). The probe test in
// tests/write/tst_write_review.cpp runs every command that is not Neutral with tracking on and checks that its change, if it makes one, is marked.

#include "core/writedoc.h"

#include <QString>

namespace jp::write {

enum class TrackPolicy {
    Neutral,        // does not change the document (it looks, finds, saves, opens, or only changes how changes are shown)
    Tracked,        // changes the document and the change is recorded (the caret may be anywhere the text is)
    TrackedInText,  // as Tracked, but in a table's cell the change is not recorded yet, so it is turned down there
    Refused         // changes the document and the change is not recorded yet: turned down with a message while Track Changes is on
};

// The policy of a command by its id. `known` (if given) says whether the id is named: a command that is not named is Refused.
TrackPolicy trackPolicyOf(const QString &commandId, bool *known = nullptr);

// The message for a change that is turned down because it is not recorded yet.
QString trackRefusalText();

// For the commands of the program's own code: with Track Changes on, says that the change about to be made is not recorded yet. Sets `doc.trackNotice` to the message and returns true (the caller
// does nothing); false (nothing set) when changes are not tracked.
bool refusedWhileTracking(WriteDoc &doc);

} // namespace jp::write
