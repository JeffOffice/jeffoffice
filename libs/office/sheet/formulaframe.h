#pragma once
// Formula texts in the id frame (docs/jeffcrunch-rowids.md, 4.4). A formula that a person types names rows and columns by their places; the text a workbook holds for it names them by their ids,
// which an insert or a delete of rows or columns does not change, so an edit has no text to write again. The two are the same numbers until an edit of the sheet's rows or columns (an id is its
// place when the sheet was read), and a row or column that was inserted has an id past the grid, so the held text may have rows to 2^24 and columns to 65,536 (calc::ParseLimits::ids()).
//
// What it takes of the workbook is the order of each sheet's rows and columns and the names of the sheets; FrameSheets says so. A reference through several sheets ("Sheet1:Sheet3!A5") names
// the same cells on each of them, which have each an order of their own, so a formula that has one is not held in ids: its text stays in places (and is written again by an edit, as it was).
// So does a text in which a defined name could be read as a column past XFD (a name "ABCD1": the wide limits would make it a cell). No GUI.

#include "calc/formula.h"

#include <QString>

#include <functional>

namespace jp::office::sheet {

class CellStore;

// The sheets of a workbook as a conversion needs them.
class FrameSheets {
public:
    virtual ~FrameSheets() = default;
    virtual int frameSheetCount() const = 0;
    virtual int frameSheetIndex(const QString &name) const = 0;   // -1 for none (a sheet of another workbook is none)
    virtual const CellStore &frameCells(int sheet) const = 0;
    // Whether no sheet has had its rows or columns edited: every id is its place.
    bool allPlain() const;
};

// What a mapper says of a reference it was given: it may have changed the numbers of `*ref` (Changed), left it alone (Same), turned out to name what is gone (Gone: the text for it is #REF!), or
// found that the formula cannot be mapped (Fail).
enum class MapResult : quint8 { Same, Changed, Gone, Fail };
using RefMapper = std::function<MapResult(int sheet, jp::calc::RefLit *ref)>;

// Calls `map` for each reference of cells in the text (not for a reference of another workbook or of a sheet the workbook has not), with the place of the sheet it names, and writes the text again with
// the references it changed. The corners of a reference it changed are put in order. False if the text can't be read, or a mapper failed; `threeD` is set if a reference through several sheets
// was met (it is left as it is). `limits` say how large the numbers in the text may be.
bool mapReferences(const FrameSheets &sheets, int ownSheet, const QString &text, const jp::calc::ParseLimits &limits, const RefMapper &map, QString *out, bool *threeD = nullptr);

// A text a person typed (in places, as the grid's limits say) as the workbook holds it (in ids). False if it can't be held so: it has a reference through several sheets, a name that could be
// read as a column in the wide limits, or it can't be read. `wide` is set if the text has numbers past the grid.
bool placesToIds(const FrameSheets &sheets, int ownSheet, const QString &typed, QString *held, bool *wide);
// The text as a person reads it, from the held one (for the cell the text is written for). A reference to a row or column that is deleted is #REF!.
QString idsToPlaces(const FrameSheets &sheets, int ownSheet, const QString &held, const jp::calc::ParseLimits &limits);

// Whether a name could be read as a cell when the limits are the wide ones (and is not one when they are the grid's): "ABCD1".
bool nameLooksWide(const QString &name);

} // namespace jp::office::sheet
