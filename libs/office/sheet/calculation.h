#pragma once
// The calculation of a workbook: its formulas worked out from its constants (not from the results the file stored), kept up to date as cells are edited. Every formula cell
// is a node; the dependency graph (calc/graph.h) says which nodes read a cell, and an edit marks those nodes, and the nodes that read them, dirty. A recalculation works
// the dirty nodes out in the order they depend on each other, with an explicit stack: a formula that reads a cell not yet worked out waits for it, so a running balance of
// a hundred thousand rows deep needs no deeper a call stack than one row. A formula that is found to read itself, directly or round a circle, is circular, and its result is
// 0 (what a calculation gives it); a workbook that asks for iterative calculation is not worked out that way yet, and with keepApart the circles of any workbook are kept apart,
// since what a file stores for them is what the last calculation left. A recalculation reads the clock
// once, can be done in slices of a given work and stopped, and a formula that reads where its values say (INDIRECT, OFFSET, a name) tells the graph what it read each time it
// is worked out. Formulas that change by themselves (NOW, RAND, OFFSET, INDIRECT) are worked out in every recalculation.
//
// With `keepApart` (what the yardstick of docs/jeffcrunch.md uses) the formulas the engine cannot work out keep the result the file stored, and every formula that reads
// one, directly or through others, is marked tainted: its value is of the engine's working, but not of the file's whole.

#include "calc/eval.h"
#include "calc/graph.h"
#include "calc/linemap.h"
#include "calc/refs.h"
#include "sheet/recalc.h"
#include "sheet/refshift.h"

#include <QHash>

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class QThread;

namespace jp::office::sheet {

// What one recalculation may take, in steps (all its slices together), and what the formulas of a workbook may hold of texts and arrays, in bytes: past them the formulas still
// to be worked out, or whose values would not fit, have the error #VALUE! (kept apart, they keep what the file stored and are of the class L, past the bounds).
constexpr qint64 kMaxRecalcWork = 1000000000;
// A cell of an array that a formula builds takes the time of a few steps (the cells alive at once, at the peak of an evaluation, are counted this many steps each, besides the steps).
constexpr qint64 kArrayCellSteps = 5;
// An array formula's range larger than this is not made members of (no real one is).
constexpr qint64 kMaxArrayCellsMarked = 1000000;
constexpr qint64 kMaxHeldBytes = qint64(512) << 20;

struct CalcOptions {
    bool keepApart = false;           // formulas the engine cannot work out keep the stored result (and mark what reads them): see the class
    bool workOutTables = false;       // with keepApart: a formula that names the columns of a table is worked out (the yardstick keeps it apart, as it keeps what it cannot work out)
    bool goRoundCircles = false;      // a workbook that asks for iterative calculation (or is told to: setIteration) has its circles worked out by going round them, as a window does; else (as the yardstick does)
                                      // they are given 0, or, with keepApart, kept apart as the file has them
    bool workOutWhatHasNoResult = false;   // with keepApart: a formula the file stored no result for is worked out (the yardstick keeps it apart, for want of a result to compare with; a window has nothing else to show)
    qint64 workLimit = 0;             // steps for one formula; 0: the engine's own
    qint64 recalcWorkLimit = 0;       // the most steps a recalculation takes in all its slices; 0: kMaxRecalcWork
    qint64 maxHeldBytes = 0;          // the most bytes the texts and arrays that formulas gave may take together; 0: kMaxHeldBytes
    qint64 maxGraphBytes = jp::calc::DependencyGraph::kDefaultBytes;   // what the graph may take; the formulas past it are not tracked and are worked out in every recalculation
    double now = 0;                   // 0: the clock is read at each recalculation; else the day number NOW and TODAY give
    qint64 workbookBytes = kMaxWorkbookBytes;
    // If set, called with the result the file stored for a formula and the one worked out; false marks the formula a mismatch, and every formula that reads one, directly or
    // through others, a cascade (isCascaded): the yardstick tells the formulas that are wrong of their own from those that follow a wrong one.
    std::function<bool(const jp::calc::Value &stored, const jp::calc::Value &ours)> verify;
};

class Calculation : public WorkbookSource {
public:
    explicit Calculation(const Workbook &book, const CalcOptions &options = CalcOptions());
    ~Calculation() override;

    // Reads every sheet and registers every formula: all of them are then to be worked out. False if a sheet could not be read whole (what was read stays).
    bool load();

    struct Progress {
        bool finished = false;      // nothing is left to work out
        bool cancelled = false;     // it was stopped, and what is left is still to be worked out
        qint64 evaluated = 0;       // formulas worked out in this call (one that waited for another and was worked out again is counted again)
        qint64 recalculated = 0;    // formulas that have their value in this call
        qint64 work = 0;            // steps taken
        qint64 circular = 0;        // formulas found circular
        qint64 deepest = 0;         // the most formulas waiting for others at once
        bool overBudget = false;    // the recalculation took the steps it may; the formulas it had not worked out have the error #VALUE! (or, kept apart, keep what the file stored)
        qint64 refused = 0;         // formulas that were given that error for want of steps, or because the values held would pass the bound
        qint64 jobItems = 0;        // things done by the steps that go a chunk at a time (formulas of a group given back, marked, refused, put right after a stop) in this call: what a test counts
                                    // to see that a slice does a bounded amount of work, whatever the machine's speed
        qint64 circleWork = 0;      // steps taken to place what a formula waits for, and the formulas of the groups that wait for each other (counted: a test can see that a circle is not found
                                    // by going over the stack again and again)
    };
    // Works out what is to be worked out (and the formulas that change by themselves): all of it, or about `workBudget` steps of it, then stops with `finished` false and the
    // rest to be done by the next call. If `cancel` is set it stops at once, with `cancelled` true.
    // `timeBudgetMicros` ends a slice by the clock as well: it stops, after a formula that has been worked out (at least one in every call, so that a call always gets on), once
    // that many microseconds have passed (0 for no limit). The work of one formula is bounded (CalcOptions::workLimit), so a slice is that long and one formula more.
    Progress recalculate(qint64 workBudget = 0, const std::atomic<bool> *cancel = nullptr, qint64 timeBudgetMicros = 0);
    // For tests: called as a formula reads a block of cells (in the middle of working it out), so that one can act at a place that is certainly inside a formula.
    void setVisitHook(std::function<void()> hook) { m_visitHook = std::move(hook); }
    // A flag that stops the calculation wherever it is: a load that is registering formulas, a recalculation between formulas, and a formula that is being worked out (the
    // evaluator looks at it as it takes its steps). The flag is only read; it must stay where it is as long as the calculation is used.
    void setCancel(const std::atomic<bool> *cancel) { m_cancel = cancel; }
    // Called with each result a recalculation gives a cell: the value of a formula (and of each cell of an array it made), where the grid can show it. Not with the result of a formula
    // that is kept apart (the file's value is what is shown then). Called on the thread that recalculates.
    // Told each value as it is worked out (on the worker): the sheet and the ids of the cell's row and column (see CellStore::rowIdAt), which are the cell's wherever an edit of rows or columns has put it.
    using ResultSink = std::function<void(int sheet, quint32 rowId, quint32 colId, const jp::calc::Value &value)>;
    void setResultSink(ResultSink sink) { m_sink = std::move(sink); }
    // ---- results in flight. Every edit of cells, and every edit of whole rows or columns that is not rigid for some formula (one that asks something of the worker: see `questions` below), makes the
    // sheet's structural epoch one more; a value told carries the epoch the sheet had when it was worked out. A result worked out before a rigid edit is good after it, and is not discarded: its ids name the
    // same cells and nothing it read changed. A formula that the worker's questions mark to be worked out again is stamped with the epoch of the edit (while a value is in flight), and the window
    // lets go of a value for it that was worked out under an older epoch; that is the worker's to do, so the stamps are kept under a lock. A formula that an
    // edit of cells moved one by one is stamped with the epoch of the edit (by the ids of its cell, at the place it was and the place it is), and the window lets go of a value for it that was worked
    // out under an older epoch (it names the cell as the edit found it), and has the formula worked out again (`redirtyCell`). A value is told by ids, so a value in flight needs no stamp unless
    // such an edit is made while one is: **stamps are made only when some value has been told and not yet taken by the window** (`resultsInFlight`), which is not so for the edit of a sheet the
    // worker has caught up with. The ids of a sheet that was renumbered (compactLines) name other cells than before: a value worked out under an epoch before that is let go of whatever its cell.
    // Window thread only, and with the worker held back for the stamps' making.
    quint32 structEpoch(int sheet) const { return sheet >= 0 && sheet < int(m_state.size()) ? m_state[size_t(sheet)].epoch : 0; }
    struct Target {
        quint32 rowId = 0, colId = 0;
        bool valid = false;
    };
    bool staleResult(int sheet, quint32 rowId, quint32 colId, quint32 epoch, Target *redo = nullptr) const;
    // The formula whose cell has these ids is worked out again (called with the worker held back).
    void redirtyCell(int sheet, quint32 rowId, quint32 colId);
    // The stamps that every value told up to the given epochs (by sheet) is past are let go of (a last batch of a calculation says nothing older is on its way).
    void pruneStamps(const std::vector<quint32> &epochs);
    qint64 stampCount() const { return qint64(m_stampCount.load(std::memory_order_relaxed)); }
    // How many values have been told (by the worker) and how many the window has taken (it says so with `valuesTaken`, for every item of a batch, those it lets go of too).
    void valuesTaken(qint64 n) const { m_valuesTaken.fetch_add(n, std::memory_order_relaxed); }
    bool resultsInFlight() const { return m_valuesTold.load(std::memory_order_relaxed) != m_valuesTaken.load(std::memory_order_relaxed); }
    // Counted, for tests: the formulas worked out, and the places of cells looked up for a formula or a value told (a scalar told by a node needs none: it names its cell by ids).
    qint64 evaluations() const { return m_evaluations.load(std::memory_order_relaxed); }
    qint64 placeLookups() const { return m_placeLookups.load(std::memory_order_relaxed); }

    // The day number NOW and TODAY give from the next recalculation (0: the clock is read at each, as it is when the calculation is made without a time).
    void setNow(double serial) { m_options.now = serial; }
    // The tables changed (made, taken out, renamed, moved, resized, or a column renamed): the formulas that name a table's columns are worked out again. Call with the worker held back.
    void tablesChanged();
    // The rows hidden on a sheet changed: the formulas that call SUBTOTAL are worked out again (they leave out hidden rows). Call with the worker held back.
    void hiddenRowsAndSubtotals(int sheet);
    // Marks every formula to be worked out again (the next recalculation is a whole one).
    void invalidateAll();
    // The same for the formulas of one sheet, and only those: what they read that is out of date is worked out as they need it, and what is waiting on other sheets (formulas never worked out,
    // and what reads cells that were changed) stays waiting, until the next recalculation, which is a whole one.
    void invalidateSheet(int sheet);
    // Circular references. A formula that reads itself, directly or round a circle, has the result 0 and is circular (isCircular), unless the workbook is to be worked out by going round
    // circles: then each formula of a circle is worked out again and again, reading the latest values of the others (those they had before, at first), until no value changes by as much as
    // `delta` or `count` rounds are done. A change of these works every formula out again. Only with CalcOptions::goRoundCircles (the yardstick keeps the circles of a file apart: what the file
    // stored is what its own program's last round left).
    static constexpr int kMaxIterations = 32767;
    void setIteration(bool on, int count, double delta);
    bool iterating() const { return m_iterate; }
    int iterationCount() const { return m_iterCount; }
    double iterationDelta() const { return m_iterDelta; }
    // The formulas found circular (not worked out by going round): how many, and the first `cap` of them in the order of the sheets and cells (`out` may be null). Called with the worker held back.
    struct CircularCell {
        int sheet;
        quint32 row, col;
    };
    qint64 circularCells(std::vector<CircularCell> *out, qint64 cap) const;
    // For tests: the formulas looked at to answer circularCells (it looks at those that were found in circles, not at every formula), and the formulas a change of a name read again.
    qint64 circleWork() const { return m_circleWork; }
    qint64 nameRebuilds() const { return m_nameRebuilds; }
    // Works out the formulas typed or edited since the last recalculation, and what they need that is not worked out; what reads them is left as it is. For a workbook worked out by hand.
    Progress recalculateFresh(qint64 workBudget = 0, const std::atomic<bool> *cancel = nullptr, qint64 timeBudgetMicros = 0);
    // Formulas that have yet to be worked out.
    bool isDirty() const { return m_notDone > 0 || m_running || m_invalidate.active; }
    // The formulas still to be worked out (while all of them are being marked, the ones not marked yet are counted, those that cannot be worked out among them).
    qint64 dirtyCount() const { return m_notDone + (m_invalidate.active ? qint64(m_nodes.size()) - qint64(m_invalidate.at) : 0); }

    // What the cells hold now: a formula's value is its last result, a constant what was put or read.
    jp::calc::Value cell(int sheet, quint32 row, quint32 col) const override;
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const jp::calc::Value &)> &f) const override;

    // What a cell shows: its value; a formula that has never been worked out shows the result the file stored for it (nothing if it stored none), and one that waits to be
    // worked out again the value it had.
    jp::calc::Value shown(int sheet, quint32 row, quint32 col) const;

    // Edits. Each marks what depends on the cell, and the values change at the next recalculation. False, with nothing changed, if the cell is off the sheet, is a cell of an array or a data
    // table (which is not changed alone), or the text is no formula. With Marking::Later the marking is left to the recalculation (the formulas that read the cell are looked for, a chunk
    // at a time, by its first slices): the edit itself then costs what moving the cell in its block of rows does, whatever number of formulas read it. A window that edits on its own thread
    // and calculates on another takes Later.
    enum class Marking { Now, Later };
    bool setValue(int sheet, quint32 row, quint32 col, const jp::calc::Value &value, Marking marking = Marking::Now, qint32 sharedString = -1);   // an empty value empties the cell
    // `sharedString`: for a text, its place in the workbook's shared strings if it is there (the cell then refers to it, and no text is kept in the sheet).
    // `stored`, if given, is the result the cell has until the formula is worked out (for a formula the engine keeps apart, which it does not work out).
    bool setFormula(int sheet, quint32 row, quint32 col, const QString &formula, Marking marking = Marking::Now, const jp::calc::Value *stored = nullptr);   // as the file writes it, with no "=" in front
    bool clearCell(int sheet, quint32 row, quint32 col) { return setValue(sheet, row, col, jp::calc::Value()); }
    // The format of the cell (the place of a cell format): a cell that holds nothing and has the first format is taken away. No formula reads a cell's format, so nothing is marked.
    bool setStyle(int sheet, quint32 row, quint32 col, quint32 style);
    // What a cell holds, whole, so that an edit can be undone: see put(). A cell that is not there is `exists` false.
    struct CellContent {
        bool exists = false;
        bool editable = true;          // false for a cell of an array or a data table
        bool isFormula = false;
        QString formula;               // as the file writes it (for a shared formula, as it reads in this cell)
        jp::calc::Value value;         // a constant's value (empty for a formula)
        qint32 sharedString = -1;      // a text in the shared strings: its place
        quint32 style = 0;
        // A formula the engine keeps apart (a function it lacks): the result it shows, which is the one the file stored until something it reads changes. Put back with the formula, so
        // that an edit undone does not turn the result into #NAME?.
        bool hasStored = false;
        jp::calc::Value stored;
        // The first cell of an array formula: the cells it covers (valid), else not valid. A data table's first cell is marked with `dataTable`.
        CellRange arrayRange;
        bool dataTable = false;
        bool operator==(const CellContent &o) const
        {
            return exists == o.exists && isFormula == o.isFormula && formula == o.formula && value == o.value && sharedString == o.sharedString && style == o.style;
        }
    };
    CellContent content(int sheet, quint32 row, quint32 col) const;
    // Puts the content in the cell (the cell taken away if it does not exist): its constant or formula, then its format. False if the cell cannot be changed, or the formula is no formula.
    bool put(int sheet, quint32 row, quint32 col, const CellContent &content, Marking marking = Marking::Now);
    // ---- structure: rows, columns, and cells inserted and deleted (see sheet/refshift.h). The cells move, every formula of every sheet that reads cells that moved is written again with
    // its references following them (and the defined names too), a reference to deleted cells is #REF!, and what must be worked out again is marked. Not for a Move (a cut and paste,
    // which is done cell by cell).
    struct EditRecord {
        StructEdit edit;
        int sheet = -1;
        // cells deleted, as they were, at the places they were
        struct GoneCell {
            int sheet;
            quint32 row, col;
            CellContent content;
            quint32 kind = 0;   // the cell's own flags that make it an array formula or a data table (put back as one, not as a formula)
            qint32 range = -1;  // and the index of its range in the sheet's list
            quint32 rowId = 0, colId = 0;   // the ids of its row and column when it was taken (the window's values and marks are by them)
        };
        std::vector<GoneCell> gone;
        // formula cells whose text changed: where each was before the edit, and what it read
        struct FormulaWas {
            int sheet;
            quint32 row, col;
            QString text;
            bool hasStored;
            jp::calc::Value stored;
        };
        std::vector<FormulaWas> was;
        // formulas of a shared family that were written again alike (one text for all of them, kept shared): the text they had (for the first cell the family was written for), and where
        // each was before the edit
        struct RewrittenGroup {
            int sheet;
            QString oldText;
            std::vector<std::pair<quint32, quint32>> cells;
            std::vector<std::pair<qint32, qint32>> offsets;   // and how far (rows, columns) each was from the cell the text was written for
        };
        std::vector<RewrittenGroup> groups;
        // the sheet's other structures, as they were
        QVector<CellRange> ranges, merges;
        AutoFilter autoFilter;
        qint64 mergeCount = 0;
        QVector<ColumnSpan> columns;
        QVector<RowSpec> rowSpecs;
        quint32 extentRows = 0, extentCols = 0, rowsSeen = 0, colsSeen = 0;
        // defined names whose text changed, with the text they had (by the key the source keeps them by)
        std::vector<std::pair<QString, QString>> names;
        qint64 cellsMoved = 0, formulasRewritten = 0;
        // the rules of validation of the sheets they changed in, as they were
        std::vector<std::pair<int, QVector<Validation>>> validations;
        // and the rules of conditional formatting of the sheets they changed in
        std::vector<std::pair<int, QVector<CondRule>>> conditionalFormats;
        // a Move: the merged ranges of the sheet it went to, as they were (the first sheet's are in `merges`)
        QVector<CellRange> toMerges;
        qint64 toMergeCount = 0;
        // where the formulas that were written again are now (after the edit), for what saving must write whole
        struct Place {
            int sheet;
            quint32 row, col;       // where the cell is (after the edit: Undo goes by places, which come back as they were)
            quint32 rowId, colId;   // and the ids of its row and column then (what a mark of the session is by; only for use at once, for a rebase of the ids renumbers them)
            bool dataTable;   // the first cell of a data table (the file's own formula is moved with it)
        };
        std::vector<Place> rewritten;
        // cells shifted or moved one by one (not whole rows or columns, which move nothing): the formulas among them, by the ids of the cell they were in and the cell they are in (the values the window
        // holds for them follow), and for a Move the places they went to on the sheet `toSheet`, for Undo
        struct ShiftedFormula {
            quint32 fromRowId, fromColId, toRowId, toColId;
        };
        std::vector<ShiftedFormula> shifted;
        std::vector<std::pair<quint32, quint32>> landed;
        // rows or columns inserted or deleted: what the edit did to the order of the sheet's rows or columns (Undo and Redo go by it, so the rows and columns come back with their cells)
        mutable CellStore::LineEdits lines;
        // a Move that carries whole arrays: the arrays it took apart (the place each had in the sheet's list of ranges, and the range), and the arrays it made where they went (and whether
        // the place in the list was added, not one a taken array left free)
        struct ArraySlot {
            int sheet;
            qint32 slot;
            CellRange range;
            bool added = false;
        };
        std::vector<ArraySlot> arraysTaken, arraysMade;
    };
    // Whether the edit can be made; if not, why, in a sentence for the person (it would cut through an array formula or a data table, push cells off the sheet, or the sheet is protected).
    bool canApply(const StructEdit &edit, QString *why) const;
    // About how many bytes the record that undoes the edit would hold (the cells it deletes or writes over, whole, and the formulas it writes again), worked out without doing it: it
    // stops counting past `stopAt`, and says that much.
    qint64 estimateUndoBytes(const StructEdit &edit, qint64 stopAt) const;
    // Makes the edit and says what is needed to undo it. False, with nothing changed, if it can't be made. `moved` is the number of cells shifted and `rewritten` the formulas written again
    // (in the record).
    bool applyEdit(const StructEdit &edit, EditRecord *record);
    // ---- the order of a sheet's rows and columns (see CellStore): an edit of whole rows or columns takes ids, which a long session could use up. An edit that is done again after Undo, and Undo itself, take none.
    // Whether the order of the rows (columns) the edit is on is near what it may hold, so that it should be compacted (compactLines) first, with every record of an edit of it let go of (forgetLines).
    bool linesNeedCompaction(const StructEdit &edit) const;
    // Puts the order of the sheet's rows (columns) back to one run, renumbering the cells and letting go of the dead ones. Every record of an edit of that order made before is stale after it: Undo of it
    // is refused, so the caller lets go of those first. False if it can't be done.
    // `table`, if given, is how the ids were renumbered: the nodes of the sheet's formulas are renumbered with the cells, and the formulas are worked out again (a value told before is let go of; the window renumbers the
    // values and marks it holds by the table).
    bool compactLines(int sheet, bool rows, std::vector<jp::calc::LineMapCore::Renumber> *table = nullptr);
    // The record's edit of rows or columns will never be undone or done again: the cells that only it could bring back are let go of. The record's lines are empty after.
    void forgetLines(EditRecord *record);
    // The most runs and ids the order of a sheet's rows and columns may have (for a test of what happens at the caps; those of a sheet are jp::calc::LineLimits'). Only for a sheet whose order has no edit.
    void setLineLimitsForTest(int sheet, const jp::calc::LineLimits &rows, const jp::calc::LineLimits &cols);
    // Puts back what the edit changed (the record is the last applyEdit's).
    bool undoEdit(const EditRecord &record, EditRecord *done = nullptr);   // `done`, if given, is told what undoing moved and removed (as applyEdit's record is)

    // ---- the widths of columns and the heights of rows (the sheet's own, which the file gives and a person changes). Called with the worker held back.
    // The runs of columns [first, last] has widths or hidden or formats for, cut to that range, in order; the rows in it that are not as the default, in order.
    std::vector<ColumnSpan> columnRuns(int sheet, quint32 first, quint32 last) const;
    std::vector<RowSpec> rowSpecsIn(int sheet, quint32 first, quint32 last) const;
    // All the rows' specs of a sheet, in order of row (shared, not copied), and putting a whole list back (it must be in order of row).
    QVector<RowSpec> rowSpecsOf(int sheet) const { return sheet >= 0 && sheet < int(m_sheets.size()) ? m_sheets[size_t(sheet)].rowSpecs : QVector<RowSpec>(); }
    void setRowSpecsOf(int sheet, const QVector<RowSpec> &specs)
    {
        if (sheet >= 0 && sheet < int(m_sheets.size())) {
            m_sheets[size_t(sheet)].rowSpecs = specs;
            hiddenRowsChanged(sheet);
        }
    }
    // The merged ranges of a sheet (and how many it has, which may be more than are kept), and putting them back.
    QVector<CellRange> mergesOf(int sheet) const { return sheet >= 0 && sheet < int(m_sheets.size()) ? m_sheets[size_t(sheet)].merges : QVector<CellRange>(); }
    qint64 mergeCountOf(int sheet) const { return sheet >= 0 && sheet < int(m_sheets.size()) ? m_sheets[size_t(sheet)].mergeCount : 0; }
    void setMerges(int sheet, const QVector<CellRange> &merges, qint64 count)
    {
        if (sheet < 0 || sheet >= int(m_sheets.size())) return;
        m_sheets[size_t(sheet)].merges = merges;
        m_sheets[size_t(sheet)].mergeCount = count;
    }
    // The sheet's filter (not valid when it has none), and putting one in its place (a filter that is not valid takes it away).
    const AutoFilter &autoFilterOf(int sheet) const
    {
        static const AutoFilter none;
        return sheet >= 0 && sheet < int(m_sheets.size()) ? m_sheets[size_t(sheet)].autoFilter : none;
    }
    void setAutoFilter(int sheet, const AutoFilter &filter)
    {
        if (sheet >= 0 && sheet < int(m_sheets.size())) m_sheets[size_t(sheet)].autoFilter = filter;
    }
    // The sheet's protection (the worker does not read it: the window changes it).
    void setProtection(int sheet, const SheetProtection &protection)
    {
        if (sheet < 0 || sheet >= int(m_sheets.size())) return;
        m_sheets[size_t(sheet)].protection = protection;
        m_sheets[size_t(sheet)].isProtected = protection.enabled;
    }
    // The sheet's own default sizes (the default row height and column width its file gives), and putting them back.
    SheetFormat sheetFormatOf(int sheet) const { return sheet >= 0 && sheet < int(m_sheets.size()) ? m_sheets[size_t(sheet)].format : SheetFormat(); }
    void setSheetFormat(int sheet, const SheetFormat &format)
    {
        if (sheet >= 0 && sheet < int(m_sheets.size())) m_sheets[size_t(sheet)].format = format;
    }
    // Puts these in their place (what the range had is let go of first): for making a change and for undoing it.
    void setColumnRuns(int sheet, quint32 first, quint32 last, const std::vector<ColumnSpan> &runs);
    void setRowSpecs(int sheet, quint32 first, quint32 last, const std::vector<RowSpec> &specs);

    // ---- the rules of validation (see sheet/sheetmodel.h): the rule for a cell (the first that names it), whether an entry is allowed by it (`typed` is the text as typed; the rule's
    // formulas are worked out with the entry in the cell), and the choices a list offers.
    // (Window thread only: the index it finds the rule with is made when first asked and held without a lock; a debug build checks that one thread asks.)
    const Validation *validationAt(int sheet, quint32 row, quint32 col) const;
    bool entryAllowed(int sheet, quint32 row, quint32 col, const Validation &rule, const jp::calc::Value &entry, const QString &typed) const;
    QStringList listChoices(int sheet, quint32 row, quint32 col, const Validation &rule) const;
    // How many times an entry was worked out against a rule of validation (for tests: what is not typed is not held to a rule).
    // Many cells changed one after another (Undo of a step, a paste, a cut): what is waiting to be worked out is put right once, before the first, not at each.
    class BulkEdit {
    public:
        explicit BulkEdit(Calculation *c) : m_c(c)
        {
            m_c->abandonNow();
            ++m_c->m_bulk;
        }
        ~BulkEdit() { --m_c->m_bulk; }
        BulkEdit(const BulkEdit &) = delete;
        BulkEdit &operator=(const BulkEdit &) = delete;

    private:
        Calculation *m_c;
    };
    // For tests: how many entries of the lists of what waits were put right (it grows with the edits, not with the edits and what waits).
    qint64 abandonWork() const { return m_abandonWork; }
    qint64 ruleEvaluations() const { return m_ruleEvaluations; }
    // For tests: the texts and formulas of a sheet that are in use, and the programs worked out for them (what edits made and let go of must not grow what is held).
    qint64 formulaTextsInUse(int sheet) const;
    qint64 formulaEntriesInUse(int sheet) const;
    qint64 programsHeld(int sheet) const;
    qint64 threeDFormulasHeld() const { return qint64(m_threeD.size()); }   // (the formulas over a block of several sheets that are kept apart: it is those that there are)
    // And how many ranges were looked at to find the rules of cells (for tests: it does not grow with the number of rules).
    qint64 ruleLookupWork() const { return m_ruleLookupWork; }

    // A column of a table renamed: the defined names that name it follow. Gives the texts the names had (by the key the calculation keeps them by), for putting back with restoreNames.
    std::vector<std::pair<QString, QString>> renameColumnInNames(const QString &table, const QString &oldName, const QString &newName);
    void restoreNames(const std::vector<std::pair<QString, QString>> &names);
    // The defined names whose texts name a table (a table renamed, or made plain cells): each text that `rewrite` gives another text for is changed. Gives the texts the names had, as above.
    std::vector<std::pair<QString, QString>> rewriteNames(const std::function<QString(const QString &)> &rewrite);

    // ---- the sheets (a person adds, renames, deletes, and copies them: see apps/crunch/core/editsession.h). A sheet is never renumbered: one added is put after the others and keeps its
    // number, one deleted keeps its number and its cells (nothing reads them any more: every reference to it is #REF!), and Undo puts it all back. Called with the worker held back.
    // Appends a sheet, empty or a copy of the cells, formats, sizes, and rules of another (its formulas are registered and are to be worked out); its number. `name` is what formulas call it.
    int addSheet(const QString &name, int copyOf = -1);
    // Takes the last sheet away (Undo of adding one).
    void removeLastSheet();
    // What formulas call the sheet from now on: `oldName` stops naming it and `newName` begins to (either may be empty: a sheet deleted, a sheet brought back).
    void setSheetNames(int sheet, const QString &oldName, const QString &newName);
    // What a rename or a delete of a sheet did to the texts that name it, for putting back.
    struct SheetRewrite {
        struct Text {
            int sheet;
            qint32 slot;
            QString before;
        };
        std::vector<Text> texts;                                              // formula texts of the cells
        std::vector<EditRecord::Place> rewritten;                             // the cells whose formulas had one of them
        std::vector<std::pair<QString, QString>> names;                       // defined names, with the text they had
        std::vector<std::pair<int, QVector<Validation>>> validations;         // rules of validation, as they were, of the sheets that had a formula to change
        std::vector<std::pair<int, QVector<CondRule>>> conditionalFormats;    // and of conditional formatting
        qint64 bytes() const;
    };
    // Every reference to the sheet `oldName` in every formula (the cells', the defined names', the rules'): changed to name `newName`, or, when it is empty (the sheet is deleted), #REF!. The
    // names formulas read the sheet by (setSheetNames) are to be made first for a rename. False, with nothing changed, if a formula reads a block of several sheets.
    bool rewriteSheetReferences(const QString &oldName, const QString &newName, SheetRewrite *record);
    // (the names of the sheet are put back first: the formulas read the sheet by name as they are written again)
    void undoSheetRewrite(const SheetRewrite &record);
    // Whether a formula (a cell's or a name's) reads a block of several sheets, which moving, adding, or deleting a sheet could change the meaning of.
    bool hasThreeDReferences() const;

    // ---- the defined names (a name's key is its scope, a "|", and its name in capitals). `scope` is the place of the sheet a name belongs to, -1 for the workbook's.
    // What the name stands for (the text of its formula, as a file writes it), by exactly that scope; false if there is none.
    bool nameText(int scope, const QString &name, QString *text) const;
    // Gives the name that text (a name that is not there is made), or takes it away when the text is empty; what uses it is worked out again.
    void setNameText(int scope, const QString &name, const QString &text);
    // The name given another name: every formula (the cells', the rules', the other names') that uses it follows, and `record` says what they were, for undoRenameName. A change of capitals alone
    // changes nothing (formulas read a name in any case).
    void renameName(int scope, const QString &oldName, const QString &newName, SheetRewrite *record);
    void undoRenameName(int scope, const QString &oldName, const QString &newName, const SheetRewrite &record);
    qint64 nameCount() const { return qint64(m_names.size()); }
    // The color of a sheet's tab, and the default sizes (see SheetFormat), put back.
    void setTabColor(int sheet, const ColorSpec &color)
    {
        if (sheet >= 0 && sheet < int(m_sheets.size())) m_sheets[size_t(sheet)].tabColor = color;
    }

    // (applyEdit does a Move as well: the cells of a block put in another place, over what is there, and what refers to them follows; Undo gives back both places.)

    // Puts right, at once, what a stop of the recalculation left to be put right (an edit does this first; a caller that is about to read the formulas' state does too).
    void settle() { abandonNow(); }

    // About the formulas.
    qint64 formulaCount() const { return m_formulaCount; }
    // The things edits of rows, columns, and cells have looked at (the formulas the graph named and those told where they read, the cells moved, the formulas written again): a test
    // counts it to see that an edit costs what moves and what refers to it, not the sheet.
    qint64 editWork() const { return m_editWork; }
    // What the edits of rows, columns, and cells did, counted (never timed), so that a test can see what an edit costs: how many cells it moved and deleted, how many formulas it looked at
    // and wrote again, how many texts of formulas it made, how many formulas were made again in the graph, how much the graph worked, and how many names and rules it followed. Undo, which
    // is an edit too, counts as one. Take the difference of two readings (`since`) for one edit.
    struct EditStats {
        qint64 edits = 0;
        qint64 cellsMoved = 0, cellsGone = 0;
        qint64 formulasVisited = 0;      // the formulas the graph named as reading the cells that move, and those that read where they are told
        qint64 formulasRewritten = 0;    // those whose text was written again (one of a shared family counts)
        qint64 textsMade = 0;            // the texts that made (a shared family's once for each distinct way its references change)
        qint64 nodesRemade = 0;          // formulas dropped from the graph and made again in it
        qint64 graphWork = 0;            // the graph's own count of the entries it built, moved, and looked at, during the edits
        qint64 namesRewritten = 0, rulesRewritten = 0;   // defined names, and lists of validations and conditional formats, written again
        // The window thread's time in the phases of an edit of rows or columns, in microseconds (for the bench's report; never asserted): setup, candidates, text rewrite, cells, graph (what is told it or asked
        // of the worker), the other structures, and the formulas written again.
        qint64 phaseMicros[7] = {0, 0, 0, 0, 0, 0, 0};
        EditStats since(const EditStats &o) const
        {
            EditStats d;
            d.edits = edits - o.edits;
            d.cellsMoved = cellsMoved - o.cellsMoved;
            d.cellsGone = cellsGone - o.cellsGone;
            d.formulasVisited = formulasVisited - o.formulasVisited;
            d.formulasRewritten = formulasRewritten - o.formulasRewritten;
            d.textsMade = textsMade - o.textsMade;
            d.nodesRemade = nodesRemade - o.nodesRemade;
            d.graphWork = graphWork - o.graphWork;
            d.namesRewritten = namesRewritten - o.namesRewritten;
            d.rulesRewritten = rulesRewritten - o.rulesRewritten;
            for (int i = 0; i < 7; ++i) d.phaseMicros[i] = phaseMicros[i] - o.phaseMicros[i];
            return d;
        }
    };
    const EditStats &editStats() const { return m_editStats; }
    // For tests: whether an edit of whole rows or columns asks the worker (the formulas whose text only moved keep their nodes and are worked out only if the edit could change them), or every formula
    // that was written again is made again and worked out, as an edit of cells does (the oracle the other is compared with).
    static void setGraphShifts(bool on);
    // Formulas the engine cannot work out and keeps apart (they keep the result the file stored); it walks the formulas, so it is for the times it is needed (a save).
    qint64 keptCount() const;
    qint64 untrackedCount() const { return m_untracked; }      // formulas the graph was too full to hold
    qint64 textSlotBytes() const;   // what the strings in the slots that hold the formulas' texts take, the free slots' too (a free slot holds none)
    qint64 heldBytes() const { return m_heldBytes; }   // what the texts and arrays that formulas gave take (the bound: CalcOptions::maxHeldBytes)
    // ---- what the worker is asked after an edit of whole rows or columns (docs/jeffcrunch-rowids.md, 4.6), counted: the questions answered (an edit that is rigid for every formula asks none) and the
    // formulas they marked to be worked out again. Four sets are asked of: (1) the formulas that can notice an empty row or column put inside a block they read (`blankInsensitive` is an allowlist of
    // functions) and whose block holds the gap; (2) the formulas that can see where cells are and whose places changed (`observesPosition`; each keeps the places it last saw); (3) the formulas that read
    // the rows or columns that died or came back, from outside them (a block that ended in a line that died keeps its id: too wide is never too narrow); (4) the formulas kept apart or told where they
    // read, which are made again with the edit itself.
    qint64 questionsAnswered() const { return m_questionsAnswered.load(std::memory_order_relaxed); }
    qint64 formulasAsked() const { return m_formulasAsked.load(std::memory_order_relaxed); }
    qint64 questionsPending() const { return qint64(m_questions.size()); }
    qint64 observerCount() const { return qint64(m_seen.size()); }   // formulas that keep the places they saw
    qint64 ownOnlyCount() const { return m_ownOnly; }                // formulas that see only their own cell (none keeps a place)
    qint64 observersLooked() const { return m_observersLooked; }     // places of those looked at by edits of lines, counted
    qint64 registrations() const { return m_registrations; }   // times a formula has told the graph what it reads (counted, so that a test can see that one that reads the same is not told again)
    qint64 alwaysListed() const { return qint64(m_always.size()); }   // formulas on the list of those worked out in every recalculation
    const jp::calc::DependencyGraph &graph() const { return *m_graph; }
    // The formulas the graph says read the block of places (asked as a change of those cells is), added to `out`; for what holds places and not the ids the graph holds. With `stop`, the search
    // ends when the flag is set (see DependencyGraph::dependentsOfCell); false is returned then.
    bool readersOf(const jp::calc::CellArea &places, std::vector<quint32> *out, const std::atomic<bool> *stop = nullptr) const;
    // The number of the formula in the cell (the graph's name for it); 0xFFFFFFFF if the cell holds none.
    quint32 nodeAt(int sheet, quint32 row, quint32 col) const;
    // Where the formula numbered `node` is (false if the number is none, or the formula is gone): for what the graph names.
    bool formulaPlace(quint32 node, int *sheet, quint32 *row, quint32 *col) const;
    bool isCircular(int sheet, quint32 row, quint32 col) const;
    bool isTainted(int sheet, quint32 row, quint32 col) const;
    bool isCascaded(int sheet, quint32 row, quint32 col) const;   // reads (through others) a formula that `verify` called a mismatch
    // For a formula kept apart: why, as the yardstick's classes say (U a function it lacks, V changes by itself, E another workbook, T a table's columns, P not parsed, N no
    // result stored or no formula, D a data table, L past the bounds, I circular in a workbook that iterates, C circular, R a rate found by iteration); 0 for a formula that was worked out.
    char keptClass(int sheet, quint32 row, quint32 col) const;

    // Every formula cell, in the order of the sheets and cells: what the yardstick compares.
    struct FormulaCell {
        int sheet;
        quint32 row, col;
        const Cell *cell;
        jp::calc::Value stored, ours;
        char kept;
        bool tainted, circular, mismatch, cascaded;
        bool worked;                                  // it has its value (a recalculation that was stopped, or timed out, left others)
        bool changesByItself, readsWhereToldOrNames;
        const jp::calc::Program *program;             // null for a formula with no text, or that did not parse; held as long as the calculation is
    };
    void forEachFormula(const std::function<void(const FormulaCell &)> &f) const;
    // Every cell that holds a formula of its own (not a cell of an array or a data table), with the formula as the file writes it: for what must look at every formula (a column of a table
    // renamed). Called with the worker held back.
    void forEachFormulaText(const std::function<void(int sheet, quint32 row, quint32 col, const QString &formula)> &f) const;

private:
    // (applyEdit, or the edit that undoes `undoOf`: the rows and columns it brought are taken out again, and the ones it took out are brought back)
    bool applyEditImpl(const StructEdit &edit, EditRecord *record, const EditRecord *undoOf);
    enum : quint8 { Dirty = 0, Queued = 1, Waiting = 2, Done = 3, Suspended = 4 };   // (Suspended: worked out, found to wait only for formulas on the way to it or in its own group)
    enum : quint16 { Kept = 1, NodeTainted = 2, Circular = 4, Always = 8, Untracked = 16, Told = 32, Array = 64, Dead = 128, Mismatch = 256, Cascade = 512, AlwaysListed = 1024, Retried = 2048, Worked = 4096, ThreeD = 8192, Iterating = 16384 };
    enum : quint8 { KindEmpty = 0, KindNumber, KindText, KindBool, KindError, KindArray };
    struct Node {
        double number = 0;
        qint32 sheet = 0;
        qint32 formula = -1;
        quint32 rowId = 0;   // the ids of the row and the column of the formula's cell (CellStore): not its place, which an edit of rows or columns changes and a node does not follow
        qint32 text = -1;
        quint16 colId = 0;
        quint8 kind = KindEmpty;
        quint8 state = Dirty;
        char kept = 0;
        quint8 traits = 0;   // what the formula can notice of an edit of whole lines (Insensitive, Observes)
        quint16 flags = 0;
    };
    // Node::traits. The default, none, is a formula that can notice an empty line put inside a block it reads and does not see where cells are.
    enum : quint8 { Insensitive = 1, Observes = 2, OwnOnly = 4 };   // (OwnOnly: it sees only its own cell, ROW() and COLUMN() with nothing in the brackets: no places are kept for it, an edit of lines asks the formulas at or past it)
    struct Compiled {
        std::shared_ptr<const jp::calc::Program> program;
        QString error;
        bool tried = false;
        bool traitsKnown = false;
        bool usesNames = false, dynamic = false, reaches = false, changesByItself = false, iterative = false;
        bool insensitive = false, observesPosition = false;   // see ProgramTraits::blankInsensitive and observesPosition
        std::vector<qint32> bound;
    };
    static constexpr qint32 kFreeFormula = -2;   // the text of a formula's place that is free to be used again
    struct SheetState {
        quint32 epoch = 0;                    // the structural epoch (see structEpoch)
        quint32 renumberEpoch = 0;            // the epoch of the last renumbering of its ids
        std::vector<quint32> nodeOf;          // by the formula's place in Sheet::formulas
        std::vector<Compiled> plain, array;   // by the formula's text
        // The texts and formulas that edits make are kept once and let go of when nothing has them: how many formulas have each text (made when an edit first needs it: textsCounted),
        // the texts the edits made by their characters, and the places of the texts and formulas that are free to be used again.
        std::vector<quint32> textRefs;
        bool textsCounted = false;
        QHash<QString, qint32> internOf;
        std::vector<qint32> freeTexts, freeFormulas;
    };

    Compiled &compiled(int sheet, qint32 text, bool array);
    // Texts and formulas, shared and let go of (see SheetState): the place of a text (a new one, or the one an edit made before with the same characters; it counts one more formula
    // for it), a formula's place made with a text it keeps a hold of, a text that one formula less has, and a formula that is not any cell's now.
    void countTexts(int sheet);
    qint32 internText(int sheet, const QString &text);
    void holdText(int sheet, qint32 text);
    void forgetTextFacts(int sheet, qint32 text);
    void releaseText(int sheet, qint32 text);
    qint32 allocFormula(int sheet, const Formula &f);
    void releaseFormula(int sheet, qint32 formula);
    // The text of the formula of this cell is now `text` for the cell itself (in place: the formula keeps its place, and a shared formula's cell is its own text from now).
    void retext(int sheet, Cell *cell, const QString &text);
    void classify(quint32 id, const Cell &cell, const Formula &f);
    void registerSheet(int sheet);
    void setSlotText(int sheet, qint32 slot, const QString &text);
    void readdNode(quint32 id);
    quint32 addNode(int sheet, quint32 row, quint32 col, const Cell &cell, qint32 formulaIndex);   // (the place of the cell)
    // The node of a formula whose cell is known by its ids (its own, when it is made again where it is).
    quint32 addNodeById(int sheet, quint32 rowId, quint32 colId, const Cell &cell, qint32 formulaIndex);
    // Where a node's cell is now (a node is of a live cell: its row and column are in the order), and the cell.
    quint32 rowOf(const Node &n) const { return m_sheets[size_t(n.sheet)].cells.rowPlaceOf(n.rowId); }
    quint32 colOf(const Node &n) const { return m_sheets[size_t(n.sheet)].cells.colPlaceOf(n.colId); }
    const Cell *cellOf(const Node &n) const { return m_sheets[size_t(n.sheet)].cells.findById(n.rowId, n.colId); }
    Cell *cellOfMutable(const Node &n) { return m_sheets[size_t(n.sheet)].cells.findMutableById(n.rowId, n.colId); }
    static quint64 stampKey(int sheet, quint32 rowId, quint32 colId) { return (quint64(quint32(sheet)) << 40) | (quint64(rowId) << 16) | colId; }
    void tellValue(const Node &n, const jp::calc::Value &v, bool fill) const;   // (counts the values it tells: m_valuesTold)
    void stampCell(int sheet, quint32 rowId, quint32 colId) { stampCell(sheet, rowId, colId, Target()); }   // (only while a value is in flight)
    void stampCell(int sheet, quint32 rowId, quint32 colId, const Target &redo);   // (a formula an edit wrote again or moved: see staleResult)
    void redirty(quint32 id);
    // The text of an array formula's or a data table's first cell changed in place (its range's cells point to its formula); false if the cell is none.
    bool rewriteArrayText(int sheet, quint32 row, quint32 col, const QString &text);
    // An array formula's cells made ordinary ones (the range at `slot` of the sheet's list is no longer valid), to be cleared or written over; made an array again at a place: the formula cell
    // there becomes its first cell over `range`, in the place `slot` of the list if it is free (else a new one, whose number is given back), with the others its members.
    void dissolveArray(int sheet, qint32 slot);
    qint32 makeArray(int sheet, quint32 row, quint32 col, const CellRange &range, qint32 slot, bool *added);
    void markArrayMembers(int sheet, quint32 row, quint32 col);
    void nameTextsChanged();
    void adjustValidations(const StructEdit &edit, const QStringList &sheetNames, EditRecord *record);
    void adjustConditionalFormats(const StructEdit &edit, const QStringList &sheetNames, EditRecord *record);
    // A cell that is deleted or written over, as it is, for Undo to put back (an array formula or a data table is put back as one: see putBackKinds).
    EditRecord::GoneCell goneCell(int sheet, quint32 row, quint32 col, const Cell &c) const;
    EditRecord::GoneCell placeGone(int sheet, quint32 row, quint32 col) const;   // (a place, with no content: for the values of the cells there to be let go of)
    void putBackKinds(const EditRecord &record);
    jp::calc::Value ruleFormula(const Validation &rule, const QString &text, int sheet, quint32 row, quint32 col, const jp::calc::CellSource &source, bool asArray) const;
    bool canMoveCells(const StructEdit &edit, QString *why) const;
    bool applyMove(const StructEdit &edit, EditRecord *record);
    bool undoMove(const EditRecord &record, EditRecord *done);
    void registerNode(quint32 id, const std::vector<jp::calc::CellArea> *told);
    jp::calc::Value nodeValue(const Node &n, quint32 id) const;
    void storeResult(Node &n, quint32 id, const jp::calc::Value &v, bool publish = true);
    void keepStored(Node &n, char cls);
    bool formulaValue(int sheet, quint32 row, quint32 col, const Cell &c, jp::calc::Value *out) const;
    jp::calc::CellArea areaOf(const Node &n) const;
    // The graph holds the ids of rows and columns (calc/idframe.h): a block of places as the ids it is told as (`reading`) or asked as, a sheet at a time; the block a change in a formula's cell
    // is asked as; and every formula told to the graph again (a sheet's lines were renumbered).
    void idAreasOn(const jp::calc::CellArea &places, bool reading, std::vector<jp::calc::CellArea> *out) const;
    jp::calc::CellArea idAreaOnSheet(int sheet, const jp::calc::CellArea &places, bool reading) const;
    jp::calc::CellArea idAreaOfNode(const Node &n) const;
    bool rebuildGraph();   // (true if some formula is to be worked out again for it)
    std::vector<jp::calc::CellArea> m_idAreas;
    void markDependents(std::vector<jp::calc::CellArea> *changed);
    void markDependentsOfIds(std::vector<jp::calc::CellArea> *changed);   // (the same, for blocks that are ids already)
    bool drainBeforeRenumbering(int sheet);
    void queueDependents(const jp::calc::CellArea &area);
    void giveName(quint32 id, std::vector<jp::calc::CellArea> *changed);
    void markDirty(quint32 id, std::vector<jp::calc::CellArea> *changed);
    bool circleKeptForRetry(Node &n) const;
    void beginRecalculation();
    // The parts of a recalculation that are done in slices of the time they are given: each does some of its work, and says whether it is all done. `timeUp` is asked every
    // few thousand things done. Each does at least one chunk, so that a call always gets on.
    // `spent` is the work of the slice so far, which each adds what it does to (a slice given a work budget is ended by it, as by the clock).
    bool stepInvalidate(const std::function<bool()> &timeUp, qint64 *spent);
    bool stepBegin(const std::function<bool()> &timeUp, qint64 *spent);
    bool stepGroup(Progress &pr, const std::function<bool()> &timeUp, qint64 *spent);
    bool stepRefuse(Progress &pr, const std::function<bool()> &timeUp, qint64 *spent);
    bool stepAbandon(const std::function<bool()> &timeUp, qint64 *spent);
    // Stops a recalculation that is going: what it had begun is to be worked out again. It takes what is to be put right (the stack, the formulas waiting in a group, those not yet
    // started) in its hands and puts it right a chunk at a time in the calls that follow (stepAbandon), so that a stop costs nothing in the call that is stopped; abandonNow() does all of
    // it at once, for an edit.
    void abandon();
    void abandonNow();
    int m_bulk = 0;   // (more than 0: many edits are being made one after another, and what is waiting was put right before the first)
    qint64 m_abandonWork = 0;
    void dropNode(quint32 id);
    void useExtent(int sheet, quint32 row, quint32 col);
    jp::calc::Value runFormula(quint32 id, qint64 *work);
    static constexpr quint32 kNone = 0xFFFFFFFFu;

    Progress recalculateSlice(qint64 workBudget, const std::atomic<bool> *cancel, qint64 timeBudgetMicros);
    CalcOptions m_options;
    qint64 m_items = 0;                       // everything the chunked steps have done (Progress::jobItems is the part of it in one call)
    std::function<void()> m_visitHook;
    const std::atomic<bool> *m_cancel = nullptr;
    ResultSink m_sink;
    struct RuleIndex;
    mutable std::vector<std::shared_ptr<RuleIndex>> m_ruleIndex;   // by sheet: where the rules of validation are (made when first asked, and again after the rules change); the window's thread only
    mutable QThread *m_ruleThread = nullptr;                       // the thread that asked (a debug build checks that it is the only one)
    void forgetRuleIndex() { m_ruleIndex.clear(); m_ruleThread = nullptr; }
    mutable qint64 m_ruleEvaluations = 0, m_ruleLookupWork = 0;
    std::unique_ptr<jp::calc::DependencyGraph> m_graph;
    jp::calc::Evaluator m_evaluator;
    jp::calc::EvalContext m_ctx;
    std::vector<Node> m_nodes;
    std::vector<SheetState> m_state;
    struct Stamp {
        quint32 epoch;
        Target redo;   // the cell whose formula is worked out again when a value for this one is let go of
    };
    std::unordered_map<quint64, Stamp> m_stamps;   // by sheet, row id, column id: the epoch of the edit that moved the formula there or away, or that the worker's questions marked it for
    mutable std::mutex m_stampLock;                // (the window reads the stamps as it takes results, and the worker makes them as it answers questions)
    mutable std::atomic<size_t> m_stampCount{0};
    mutable std::atomic<qint64> m_valuesTold{0};
    mutable std::atomic<qint64> m_valuesTaken{0};
    std::atomic<qint64> m_evaluations{0};
    mutable std::atomic<qint64> m_placeLookups{0};
    std::vector<quint32> m_freeNodes;
    qint64 m_editWork = 0;
    EditStats m_editStats;
    // The places a formula that can see where cells are saw when it was told the graph what it reads (its own cell, and the first and last cell of each block it names): after an edit of whole
    // lines the places are looked at again, and a formula for which one differs is worked out again. The size of the formulas that have the trait, not of the sheet.
    struct SeenPlace {
        qint32 sheet;
        quint32 rowId, colId;   // the ids of the cell,
        quint32 row, col;       // and its place then
    };
    struct Seen {
        std::vector<SeenPlace> at;
        bool always = false;   // more blocks than are kept: it is worked out again after every edit of lines
    };
    std::unordered_map<quint32, Seen> m_seen;
    bool m_hasUnknown = false;   // some formula is kept apart for a function the engine lacks (it may read the text of a formula): a formula written again by an edit of lines queues what reads it
    qint64 m_seenPlaces = 0;   // all the places kept (the sizes of the lists in m_seen)
    void recordSeen(quint32 id, const std::vector<jp::calc::CellArea> &blocks);
    bool seenDiffers(const Seen &s) const;
    // With `sheet`, only an edit of its lines from the place `first` on (rows or columns) is looked at: a formula all of whose places are before it cannot have moved.
    bool anObserverSawAnotherPlace(int sheet = -1, bool rows = true, quint32 first = 0, bool *own = nullptr) const;
    void noteSeenPlace(int sheet, quint32 row, quint32 col);
    qint64 m_ownOnly = 0;
    void setSeenAlways(Seen &s, bool on);
    mutable std::vector<quint32> m_seenMaxRow, m_seenMaxCol;   // by sheet: past the largest place any formula kept (an upper bound: it is not lowered)
    mutable qint64 m_observersLooked = 0;
    qint64 m_seenAlways = 0;                                    // formulas worked out after every edit of lines (more blocks than are kept, or a cell that moved alone)
    // What an edit of whole rows or columns asks of the worker (the sets of `questionsAnswered`), in ids: they stay true whatever edit comes next. The worker answers it a chunk at a time.
    struct Question {
        int sheet = 0;
        bool rows = true;
        quint32 epoch = 0;                              // the sheet's epoch the edit made, for the stamps
        quint32 gapBefore = 0xFFFFFFFFu, gapAfter = 0xFFFFFFFFu;   // set 1: the original lines either side of lines put in (or of inserted lines that died); none for no gap that a block can hold
        std::vector<jp::calc::IdRun> deadBlank;         // set 3, lines pushed off the end of the sheet that held no cell: the formulas that can notice an empty line, of which a block holds them
        std::vector<jp::calc::IdRun> dead;              // set 3: original lines that died or came back, as runs of ids
        bool deadPair = false;                          // set 3: inserted lines that held cells died or came back, between the original lines of the gap
        bool observers = false;                         // set 2
        bool ownOnly = false;                           // set 2, for the formulas that see only their own cell: those of the sheet at or past `ownFirst` (a row or a column)
        quint32 ownFirst = 0;
    };
    std::deque<Question> m_questions;
    std::atomic<qint64> m_questionsAnswered{0}, m_formulasAsked{0};
    bool questionHasAnAnswer(const Question &q) const;
    bool buildAsked(const Question &q, std::vector<quint32> *out, const std::atomic<bool> *stop) const;
    void markAsked(quint32 node, const Question &q, std::vector<jp::calc::CellArea> *changed);
    void stampAsked(const Node &n, quint32 epoch);
    std::vector<quint32> m_special;   // formulas that are told where they read (names, INDIRECT, ...) or kept apart: an edit that moves cells looks at these as well as at what the graph names (may hold gone and repeated ones)
    void specialNodes(std::vector<quint32> *out);
    QVector<QString> m_texts;
    std::vector<qint32> m_freeTexts;
    QHash<quint32, jp::calc::Value> m_arrays;
    std::unordered_set<quint32> m_subtotalNodes;     // the formulas that call SUBTOTAL (a place may be stale)
    std::unordered_set<quint32> m_structuredNodes;   // the formulas that name a table's columns (a place may be stale: tablesChanged looks at each)
    jp::calc::NameTraitsCache m_nameTraits;   // what each defined name calls (by its scope and name)
    std::unordered_map<quint32, std::pair<qint32, qint32>> m_threeDSpan;   // the sheets a 3-D formula's blocks span, first to last
    std::unordered_set<quint32> m_threeD;     // formulas that read a block of several sheets (a node on it has ThreeD); one leaves it when its formula goes or reads no more than one sheet
    std::vector<quint32> m_always;            // formulas worked out in every recalculation, each once (a node on it has AlwaysListed); those that are not, or are gone, are left out at the next
    void listAlways(quint32 id);
    std::vector<quint32> m_dirty;             // formulas to start from, and where in it the recalculation is
    size_t m_dirtyCursor = 0;
    struct Frame {
        quint32 node;
        quint32 parent;   // the formula that waits for it (kNone for one the recalculation started from)
    };
    std::vector<Frame> m_stack;
    // The groups of formulas that wait for each other (strongly connected components, found as the work goes: Tarjan's way, with the stack of the work as its depth-first
    // search): a formula that waits is given a number when it first does, and a formula worked out that waits only for those on the way to it, or in its own group, is left
    // suspended on the stack of the group until the group is complete. m_index and m_low are 0 for a formula not on that stack.
    std::vector<quint32> m_index, m_low;
    std::vector<quint64> m_toldHash;   // by node: a number that says what it touched the last time it was worked out, as the graph was told (0: not told)
    std::vector<jp::calc::CellArea> m_toldSet;
    std::vector<quint32> m_comp;
    quint32 m_counter = 0;
    // Closing a group of formulas that wait for each other, and waking the formulas suspended behind one that is worked out, are jobs of their own (a circle may be a million formulas):
    // closeGroup() and leaveGroup() begin one, and stepGroup() does it, a chunk at a time.
    void closeGroup(quint32 root, quint32 parent, Progress &pr);
    void leaveGroup(quint32 node, quint32 parent);
    struct GroupJob {
        enum Phase : quint8 { None, Collect, Sort, Apply, Iterate } phase = None;
        bool closing = false;      // closeGroup, or leaveGroup
        quint32 root = 0, parent = 0;
        bool fresh = false;        // some formula of the group has not had its second try
        std::vector<quint32> open;
        int sortPass = 0;          // for Sort: the passes of the radix sort done (each is two parts: the places counted, and put where they go, each a block at a time)
        int sortStage = 0;
        size_t sortAt = 0;
        std::vector<size_t> sortCount;
        std::vector<quint32> sortTmp;
        size_t cursor = 0;         // for Apply: those before it are still to be done (it goes from the last to the first)
        int stage = 0;             // for Iterate: 0 the formulas are made ready, 1 they are worked out round and round, 2 their values are given
        quint32 round = 0;         // the rounds done
        size_t at = 0;             // and the formula the round is at
        double change = 0;         // the most any value changed by in the round
        std::vector<jp::calc::Value> start;   // the values the rounds began from, and whether each was the file's result
        std::vector<char> fromFile;
    } m_job;
    struct AbandonJob {
        bool active = false;
        std::vector<quint32> open;              // formulas of a group that were not yet given back to the stack: those before openEnd
        size_t openEnd = 0, openAt = 0;
        std::vector<Frame> stack;
        size_t stackAt = 0;
        std::vector<quint32> comp;
        size_t compAt = 0;
        std::vector<quint32> dirty;             // those that were to be started, from dirtyAt
        size_t dirtyAt = 0;
    } m_abandon;
    struct RefuseJob {
        bool active = false;
        size_t stackAt = 0, compAt = 0, dirtyAt = 0;
    } m_refuse;
    struct BeginJob {
        bool active = false;
        size_t alwaysAt = 0, listed = 0;
        bool dependents = false;
        std::vector<jp::calc::CellArea> changed;
        std::vector<quint32> found;
        size_t foundAt = 0;
        jp::calc::CellArea queried;   // the block `found` is the readers of
        // The question at the front of m_questions, turned into the formulas it names (built), the ones of them marked so far, and the formulas that keep places they saw, looked at so far.
        bool built = false;
        std::vector<quint32> asked;
        size_t askedAt = 0;
        std::vector<quint32> observers;
        size_t observersAt = 0;
        size_t ownAt = 0;   // the node the look for formulas that see only their own cell has got to
    } m_begin;
    int m_onlySheet = -1;   // a recalculation of one sheet is going (invalidateSheet): the formulas of the others are left, and noted here for the next
    std::vector<quint32> m_deferredRoots;
    std::vector<jp::calc::CellArea> m_deferredAreas;
    jp::calc::CellArea m_sheetArea;   // the whole of the sheet a recalculation of one sheet worked out, which the formulas of other sheets that read it wait for (the sheet's own formulas are not marked again)
    int m_sheetAreaOwner = -1, m_sheetAreaPending = -1;
    void releaseDeferred(bool discardSheetArea = false);
    // Formulas typed or edited and not yet worked out, which a workbook worked out by hand works out at once (recalculateFresh); and those in circles, found as they are marked.
    std::unordered_set<quint32> m_fresh;
    bool m_onlyFresh = false;
    mutable std::unordered_set<quint32> m_circle;
    mutable qint64 m_circleWork = 0;
    const Node &circleNode(quint32 id) const;
    qint64 m_nameRebuilds = 0;
    // The formulas whose text mentions one of the names (or a name that mentions them) are read again; what else uses names is left.
    void nameTextsChangedFor(const QStringList &names);
    struct InvalidateJob {
        bool active = false;
        size_t at = 0;
        int sheet = -1;   // only the formulas of this sheet; -1 for all
    } m_invalidate;
    void markCircular(quint32 id, Progress &pr);
    bool finishNode(Node &n, quint32 id, const jp::calc::Value &v);   // true if the value was not held for want of room
    void giveError(Node &n, quint32 id);
    void refuseNode(quint32 id, Progress &pr);
    void refuseTheRest(Progress &pr);
    qint64 heldBy(const Node &n, quint32 id) const;
    void freeTextSlot(qint32 slot);
    static qint64 heldByValue(const jp::calc::Value &v);
    jp::calc::Value fitToRange(const Node &n, const jp::calc::Value &v) const;
    qint64 m_registrations = 0;
    qint64 m_workUsed = 0, m_workLimit = kMaxRecalcWork;   // the steps this recalculation has taken in all its slices, and the most it may
    qint64 m_heldBytes = 0, m_heldLimit = kMaxHeldBytes;
    bool m_running = false;
    double m_now = 0;
    qint64 m_notDone = 0, m_formulaCount = 0, m_untracked = 0;
    bool m_iterate = false;
    int m_iterCount = 100;
    double m_iterDelta = 0.001;
    // while a formula is being worked out
    mutable bool m_evaluating = false;
    mutable std::vector<quint32> m_pending;   // formulas it read that were not worked out yet
    mutable bool m_readTaint = false, m_readCascade = false;
    std::vector<jp::calc::CellArea> m_touched, m_areas;
};

} // namespace jp::office::sheet
