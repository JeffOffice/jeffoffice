#pragma once
// What a person can change about an object over the cells (a chart, picture, or shape), as one step of Undo holds it: where it is, how the chart is made, and whether it is there at all.
// A step keeps the state before and the state after, and Undo and Redo give the object the one or the other (core/sheetobjects.h).

#include "office/chart.h"
#include "sheet/drawing.h"

#include <QStringList>

#include <memory>

namespace jp::crunch {

struct ObjectState {
    jp::office::sheet::DrawingAnchor anchor;          // where it is, as the rows and columns were after `anchorAt` of the edits to rows and columns made since the file was read or saved
    int anchorAt = 0;
    std::shared_ptr<const jp::office::Chart> chart;   // a chart's model (null: whatever the object has; for a picture or shape)
    QStringList refs;                                 // the chart's references, by the names the file has for sheets, as they were after `refsAt` of those edits
    int refsAt = 0;
    bool deleted = false;                             // taken away (or not made yet)
    int order = 0;                                    // its place among the sheet's objects, from the back
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
