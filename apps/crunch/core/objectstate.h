#pragma once
// What a person can change about an object over the cells (a chart, picture, or shape), as one step of Undo holds it: where it is, how the chart is made, and whether it is there at all.
// A step keeps the state before and the state after, and Undo and Redo give the object the one or the other (core/sheetobjects.h).
//
// Where an object is and what a chart reads follow the rows and columns inserted and deleted: they are worked out from what the file has, going through those edits in order, with each place
// a person put the object (or each set of references a person gave the chart) set at the moment it was given: after that many of the edits. That is why a state keeps the list of those
// places, and not only the last: undoing an insert of rows goes back through the edits again, and finds the object where the person left it by then.

#include "office/chart.h"
#include "sheet/drawing.h"

#include <QStringList>

#include <memory>
#include <vector>

namespace jp::crunch {

// An anchor a person gave an object, after `at` of the edits to rows and columns made since the file was read or saved.
struct AnchorPlacement {
    int at = 0;
    jp::office::sheet::DrawingAnchor anchor;
};

// The references a person gave a chart (by the names the file has for sheets), the same way.
struct RefPlacement {
    int at = 0;
    QStringList refs;
};

struct ObjectState {
    std::vector<AnchorPlacement> placements;          // where it was put, in the order it was put there (the file's anchor, moved by the edits, is where it begins)
    std::shared_ptr<const jp::office::Chart> chart;   // a chart's model (null: whatever the object has; for a picture or shape)
    std::vector<RefPlacement> refs;                   // what the chart reads, as given by a person
    bool deleted = false;                             // taken away (or not made yet)
    int order = 0;                                    // its place among the sheet's objects, from the back
    int epoch = 0;                                    // which stretch between saves the places are of: after a save that followed edits to rows and columns, what was kept of the one before is no use
};

// A change to one object of a sheet: the state it had and the state it has.
struct ObjectEdit {
    int sheet = 0;
    int id = 0;
    ObjectState before, after;
    bool deletes() const { return after.deleted && !before.deleted; }
    bool adds() const { return before.deleted && !after.deleted; }
};

} // namespace jp::crunch
