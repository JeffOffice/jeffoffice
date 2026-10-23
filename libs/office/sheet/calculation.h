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
#include "calc/refs.h"
#include "sheet/recalc.h"

#include <QHash>

#include <atomic>
#include <memory>
#include <vector>

namespace jp::office::sheet {

// What one recalculation may take, in steps (all its slices together), and what the formulas of a workbook may hold of texts and arrays, in bytes: past them the formulas still
// to be worked out, or whose values would not fit, have the error #VALUE! (kept apart, they keep what the file stored and are of the class L, past the bounds).
constexpr qint64 kMaxRecalcWork = 1000000000;
// A cell of an array that a formula builds takes the time of a few steps (the cells alive at once, at the peak of an evaluation, are counted this many steps each, besides the steps).
constexpr qint64 kArrayCellSteps = 5;
constexpr qint64 kMaxHeldBytes = qint64(512) << 20;

struct CalcOptions {
    bool keepApart = false;           // formulas the engine cannot work out keep the stored result (and mark what reads them): see the class
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
        qint64 circleWork = 0;      // steps taken to place what a formula waits for, and the formulas of the groups that wait for each other (counted: a test can see that a circle is not found
                                    // by going over the stack again and again)
    };
    // Works out what is to be worked out (and the formulas that change by themselves): all of it, or about `workBudget` steps of it, then stops with `finished` false and the
    // rest to be done by the next call. If `cancel` is set it stops at once, with `cancelled` true.
    Progress recalculate(qint64 workBudget = 0, const std::atomic<bool> *cancel = nullptr);
    // The day number NOW and TODAY give from the next recalculation (0: the clock is read at each, as it is when the calculation is made without a time).
    void setNow(double serial) { m_options.now = serial; }
    // Marks every formula to be worked out again (the next recalculation is a whole one).
    void invalidateAll();
    // Formulas that have yet to be worked out.
    bool isDirty() const { return m_notDone > 0 || m_running; }
    qint64 dirtyCount() const { return m_notDone; }

    // What the cells hold now: a formula's value is its last result, a constant what was put or read.
    jp::calc::Value cell(int sheet, quint32 row, quint32 col) const override;
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const jp::calc::Value &)> &f) const override;

    // Edits. Each marks what depends on the cell; the values change at the next recalculation. False, with nothing changed, if the cell is off the sheet or the text no formula.
    bool setValue(int sheet, quint32 row, quint32 col, const jp::calc::Value &value);   // an empty value empties the cell
    bool setFormula(int sheet, quint32 row, quint32 col, const QString &formula);        // as the file writes it, with no "=" in front
    bool clearCell(int sheet, quint32 row, quint32 col) { return setValue(sheet, row, col, jp::calc::Value()); }

    // About the formulas.
    qint64 formulaCount() const { return m_formulaCount; }
    qint64 untrackedCount() const { return m_untracked; }      // formulas the graph was too full to hold
    qint64 textSlotBytes() const;   // what the strings in the slots that hold the formulas' texts take, the free slots' too (a free slot holds none)
    qint64 heldBytes() const { return m_heldBytes; }   // what the texts and arrays that formulas gave take (the bound: CalcOptions::maxHeldBytes)
    qint64 alwaysListed() const { return qint64(m_always.size()); }   // formulas on the list of those worked out in every recalculation
    const jp::calc::DependencyGraph &graph() const { return *m_graph; }
    // The number of the formula in the cell (the graph's name for it); 0xFFFFFFFF if the cell holds none.
    quint32 nodeAt(int sheet, quint32 row, quint32 col) const;
    bool isCircular(int sheet, quint32 row, quint32 col) const;
    bool isTainted(int sheet, quint32 row, quint32 col) const;
    bool isCascaded(int sheet, quint32 row, quint32 col) const;   // reads (through others) a formula that `verify` called a mismatch
    // For a formula kept apart: why, as the yardstick's classes say (U a function it lacks, V changes by itself, E another workbook, T a table's columns, P not parsed, N no
    // result stored or no formula, D a data table, L past the bounds, I circular in a workbook that iterates, C circular, R a rate found by iteration); 0 for a formula that was worked out.
    char keptClass(int sheet, quint32 row, quint32 col) const;

    // Every formula cell, in the order of the sheets and cells: what the yardstick compares.
    struct FormulaCell {
        int sheet;
        const Cell *cell;
        jp::calc::Value stored, ours;
        char kept;
        bool tainted, circular, mismatch, cascaded;
        bool worked;                                  // it has its value (a recalculation that was stopped, or timed out, left others)
        bool changesByItself, readsWhereToldOrNames;
        const jp::calc::Program *program;             // null for a formula with no text, or that did not parse; held as long as the calculation is
    };
    void forEachFormula(const std::function<void(const FormulaCell &)> &f) const;

private:
    enum : quint8 { Dirty = 0, Queued = 1, Waiting = 2, Done = 3, Suspended = 4 };   // (Suspended: worked out, found to wait only for formulas on the way to it or in its own group)
    enum : quint16 { Kept = 1, NodeTainted = 2, Circular = 4, Always = 8, Untracked = 16, Told = 32, Array = 64, Dead = 128, Mismatch = 256, Cascade = 512, AlwaysListed = 1024, Retried = 2048 };
    enum : quint8 { KindEmpty = 0, KindNumber, KindText, KindBool, KindError, KindArray };
    struct Node {
        double number = 0;
        qint32 sheet = 0;
        qint32 formula = -1;
        quint32 row = 0;
        qint32 text = -1;
        quint16 col = 0;
        quint8 kind = KindEmpty;
        quint8 state = Dirty;
        char kept = 0;
        quint16 flags = 0;
    };
    struct Compiled {
        std::shared_ptr<const jp::calc::Program> program;
        QString error;
        bool tried = false;
        bool traitsKnown = false;
        bool usesNames = false, dynamic = false, reaches = false, changesByItself = false, iterative = false;
        std::vector<qint32> bound;
    };
    struct SheetState {
        std::vector<quint32> nodeOf;          // by the formula's place in Sheet::formulas
        std::vector<Compiled> plain, array;   // by the formula's text
    };

    Compiled &compiled(int sheet, qint32 text, bool array);
    void classify(quint32 id, const Cell &cell, const Formula &f);
    quint32 addNode(int sheet, const Cell &cell, qint32 formulaIndex);
    void registerNode(quint32 id, const std::vector<jp::calc::CellArea> *told);
    jp::calc::Value nodeValue(const Node &n, quint32 id) const;
    void storeResult(Node &n, quint32 id, const jp::calc::Value &v);
    void keepStored(Node &n, char cls);
    bool formulaValue(int sheet, const Cell &c, jp::calc::Value *out) const;
    jp::calc::CellArea areaOf(const Node &n) const;
    void markDependents(std::vector<jp::calc::CellArea> *changed);
    void markDirty(quint32 id, std::vector<jp::calc::CellArea> *changed);
    void beginRecalculation();
    void abandon();
    void dropNode(quint32 id);
    void useExtent(int sheet, quint32 row, quint32 col);
    jp::calc::Value runFormula(quint32 id, qint64 *work);
    static constexpr quint32 kNone = 0xFFFFFFFFu;

    CalcOptions m_options;
    std::unique_ptr<jp::calc::DependencyGraph> m_graph;
    jp::calc::Evaluator m_evaluator;
    jp::calc::EvalContext m_ctx;
    std::vector<Node> m_nodes;
    std::vector<SheetState> m_state;
    std::vector<quint32> m_freeNodes;
    QVector<QString> m_texts;
    std::vector<qint32> m_freeTexts;
    QHash<quint32, jp::calc::Value> m_arrays;
    jp::calc::NameTraitsCache m_nameTraits;   // what each defined name calls (by its scope and name)
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
    std::vector<quint32> m_comp;
    quint32 m_counter = 0;
    void closeGroup(quint32 root, quint32 parent, Progress &pr);
    void leaveGroup(quint32 node, quint32 parent);
    void markCircular(quint32 id, Progress &pr);
    bool finishNode(Node &n, quint32 id, const jp::calc::Value &v);   // true if the value was not held for want of room
    void giveError(Node &n, quint32 id);
    void refuseNode(quint32 id, Progress &pr);
    void refuseTheRest(Progress &pr);
    qint64 heldBy(const Node &n, quint32 id) const;
    void freeTextSlot(qint32 slot);
    static qint64 heldByValue(const jp::calc::Value &v);
    jp::calc::Value fitToRange(const Node &n, const jp::calc::Value &v) const;
    qint64 m_workUsed = 0, m_workLimit = kMaxRecalcWork;   // the steps this recalculation has taken in all its slices, and the most it may
    qint64 m_heldBytes = 0, m_heldLimit = kMaxHeldBytes;
    bool m_running = false;
    double m_now = 0;
    qint64 m_notDone = 0, m_formulaCount = 0, m_untracked = 0;
    bool m_iterate = false;
    // while a formula is being worked out
    mutable bool m_evaluating = false;
    mutable std::vector<quint32> m_pending;   // formulas it read that were not worked out yet
    mutable bool m_readTaint = false, m_readCascade = false;
    std::vector<jp::calc::CellArea> m_touched, m_areas;
};

} // namespace jp::office::sheet
