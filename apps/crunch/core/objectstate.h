#pragma once
// What a person can change about an object over the cells (a chart, picture, or shape), as one step of Undo holds it: where it is, how the chart is made, and whether it is there at all.
// A step keeps the state before and the state after, and Undo and Redo give the object the one or the other (core/sheetobjects.h).
//
// Where an object is and what a chart reads follow the rows and columns inserted and deleted: they are worked out from what the file has, going through those edits in order, with each place
// a person put the object (or each set of references a person gave the chart) set at the moment it was given: after that many of the edits. That is why a state keeps the list of those
// places, and not only the last: undoing an insert of rows goes back through the edits again, and finds the object where the person left it by then.

#include "office/chart.h"
#include "sheet/drawing.h"

#include <QByteArray>
#include <QString>
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

// A part an object of the saved file reached through its relationships, as the file had it: its bytes and media type, and the parts it reached in turn, each by the id its own relationships gave.
struct RevivePart {
    QString name;                     // its name in the file it was taken from
    QString relType;                  // the whole type of the relationship that reached it
    QString contentType;
    QByteArray bytes;
    QString relId;                    // the id of that relationship in the part that owns it
    bool external = false;            // the relationship leaves the package: there is no part, only `externalTarget`
    QString externalTarget;
    std::vector<RevivePart> children;
    qint64 size() const
    {
        qint64 n = qint64(sizeof(RevivePart)) + bytes.size() + (name.size() + externalTarget.size() + relType.size() + contentType.size()) * 2;
        for (const RevivePart &c : children) n += c.size();
        return n;
    }
};

// What it takes to write an object of the saved file back after the save dropped it: its anchor element (with a token where each relationship id of the drawing is, in `parts`), every part it
// reached, and where it was. Taken when the object is deleted, held by the step that deletes it (so it counts against the bytes Undo keeps, and goes when the step does).
struct ObjectRevive {
    QByteArray anchorXml;
    std::vector<std::pair<QString, RevivePart>> parts;   // (token, part)
    jp::office::sheet::DrawingAnchor anchor;
    std::shared_ptr<const jp::office::Chart> chart;   // a chart: the model the part's bytes are of (the file's); the object's own model is this one while the chart is as the file had it
    QStringList refs;                                  // what the chart read when the record was made
    bool unchanged = false;                            // the chart was as the file had it when the record was made
    qint64 bytes() const
    {
        qint64 n = qint64(sizeof(ObjectRevive)) + anchorXml.size();
        for (const auto &p : parts) n += p.first.size() * 2 + p.second.size();
        return n;
    }
};

struct ObjectState {
    std::shared_ptr<const ObjectRevive> revive;       // set on the state of a delete of an object that is in the saved file: what Undo writes back after a save
    bool madeHere = false;                            // a person made the object here (a chart, a pasted picture): it is written again from what the program holds of it
    std::vector<AnchorPlacement> placements;          // where it was put, in the order it was put there (the file's anchor, moved by the edits, is where it begins)
    std::shared_ptr<const jp::office::Chart> chart;   // a chart's model (null: whatever the object has; for a picture or shape)
    std::vector<RefPlacement> refs;                   // what the chart reads, as given by a person
    QString name;                                     // its name (empty: whatever it has)
    bool describes = false;                           // `description` is set (it may be empty: no alternative text)
    QString description;                              // its alternative text (the descr of its cNvPr)
    int prints = -1;                                  // whether it is on the printed pages (-1: whatever it has)
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

// About how many bytes a state holds that a step of Undo keeps: the numbers and words of a chart's series (a chart made from many cells is a large thing), and its references.
inline qint64 approxBytes(const ObjectState &st)
{
    qint64 n = qint64(sizeof(ObjectState)) + qint64(st.placements.size()) * qint64(sizeof(AnchorPlacement)) + (st.revive ? st.revive->bytes() : 0);
    for (const RefPlacement &r : st.refs) n += 64 + qint64(r.refs.size()) * 96;
    if (st.chart)
        for (const jp::office::ChartPlot &p : st.chart->plots)
            for (const jp::office::ChartSeries &s : p.series) {
                n += 1500 + qint64(s.values.size() + s.xValues.size() + s.categoryNumbers.size()) * 8;
                for (const QString &c : s.categories) n += 32 + qint64(c.size()) * 2;
            }
    return n;
}

} // namespace jp::crunch
