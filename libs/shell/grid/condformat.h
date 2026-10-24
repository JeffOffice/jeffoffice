#pragma once
// What a sheet's rules of conditional formatting do to the cells that are shown: for a cell, the rules whose ranges hold it are looked at in the order of their priority, and what they
// say is put together (a differential format changes the cell's font, fill, border, or number format; a color scale gives the fill a color; a data bar a bar; an icon set an icon), until a
// rule that says to stop. Only the cells that are drawn are worked out, and the results are kept until a value changes.
//
// What a rule needs of all its cells (the lowest and highest, the average, the number of times a value is there) is worked out once for a set of ranges, however many rules have it, in
// slices a few milliseconds long (a pass over a million cells is not one turn of the program): a rule whose figures are not ready shows nothing until they are, and the window is told to
// draw again. A figure is kept up to date as the cells change, one cell at a time, not made again. What costs more than a paint may have (a rule's formula worked out for many cells) is
// finished in the paints after it, and a limit that stops a rule altogether is said once (`takeNotice`), never passed over in silence.

#include "calc/eval.h"
#include "calc/formula.h"
#include "sheet/sheetmodel.h"
#include "sheet/styles.h"

#include <QRgb>
#include <QString>

#include <algorithm>
#include <deque>
#include <map>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace jp::grid {

class GridSheet;

// A data bar in a cell: where it begins and ends across the cell (0 at the left edge, 1 at the right), and how it looks.
struct CondBar {
    double from = 0, to = 0;
    QRgb fill = 0, fillEnd = 0, border = 0, axis = 0;
    bool gradient = true, hasBorder = false, hasAxis = false;
    double axisAt = 0;      // where the axis is (0 to 1) when there is one
    bool negative = false;
};

// What the rules do to one cell.
struct CondCell {
    bool any = false;
    int format = -1;                 // the differential formats put together (CondFormats::format), or -1
    bool hasScale = false;           // a color scale gives the fill this color
    QRgb scale = 0;
    bool hasBar = false;
    CondBar bar;
    int icon = -1;                   // an icon set gives this icon (from 0, the lowest) of the set `iconSet` (see iconSetCount)
    int iconSet = -1;
    bool hideValue = false;          // the rule says to show the bar or the icon and not the value
};

// The icon sets (names as the file has them): how many icons each has, or 0 for a name it does not know.
int iconSetCount(const QString &name);

class CondFormats {
public:
    // The limits (see the notices below for what each says when it stops something).
    static constexpr qint64 kSyncCells = 100000;          // the cells of a pass that are looked at as the cells are drawn; a bigger pass goes on in slices
    static constexpr qint64 kSliceCells = 20000;          // the cells of a slice
    static constexpr qint64 kMaxStatCells = 2000000;      // the cells that the figures of a sheet's rules are held for, all together
    static constexpr qint64 kMaxIndexedRanges = 200000;   // the ranges of a sheet's rules that are looked at, all together (the rules after that are not shown)
    static constexpr qint64 kPaintWork = 2000000;         // the steps of rules' formulas that a paint (or a slice) works out; the cells left over are done in the paints after it
    static constexpr qint64 kTestWork = 100000;           // the steps one formula may take for one cell

    // The limits of a sheet's rules, as a test sets them smaller to see a limit reached (`setLimits` for each, a limit of 0 or less is left as it is).
    struct Limits {
        qint64 syncCells = kSyncCells, sliceCells = kSliceCells, statCells = kMaxStatCells, ranges = kMaxIndexedRanges, paintWork = kPaintWork, testWork = kTestWork;
    };
    void setLimits(const Limits &limits)
    {
        m_limits = limits;
        reset();
    }
    const Limits &limits() const { return m_limits; }

    explicit CondFormats(const GridSheet *owner);
    ~CondFormats();
    CondFormats(const CondFormats &) = delete;
    CondFormats &operator=(const CondFormats &) = delete;
    // The sheet's rules are new or changed (or its cells): everything that was worked out is let go.
    void reset();
    // Values changed: the results kept are let go, and the figures are made again (they are kept up to date by `valueChanged` when it is cell by cell).
    void valuesChanged();
    // One cell's value changed: the figures of the rules that have it among their cells are changed by it (one cell is looked at, not the cells of the rule).
    void valueChanged(quint32 row, quint32 col);
    // What was worked out for the cells is let go, but not the figures of the rules (a value changed that no rule has among its cells, but a rule's formula may read it).
    void resultsChanged() { m_stale = true; }
    // Whether the sheet has rules at all.
    bool any() const;
    // The day number TODAY() and the rules of dates read (the clock's day by default).
    void setToday(double serial)
    {
        m_today = serial;
        m_todayFixed = true;
        reset();
    }
    // What the rules do to the cell. A value, not a reference: what is kept can be let go (to keep it bounded) while a paint holds what it was told.
    CondCell at(quint32 row, quint32 col) const;
    // The differential format a result names.
    const jp::office::sheet::DifferentialFormat &format(int index) const;
    // Whether the cell is in the range of some rule (found through an index of the ranges, so that a drawn cell costs what the rules that have it cost, not all the rules).
    bool covers(quint32 row, quint32 col) const;
    // The name of the icon set of a rule (by its place in the sheet's rules).
    QString iconSetName(int rule) const;

    // ---- work that is not done at once
    // A paint is about to begin: it has the steps of rules' formulas that `kPaintWork` says.
    void beginPaint() const { m_paintWork = 0; }
    // Whether something is left to do: figures being made, or cells left plain for want of the steps a paint has.
    bool wantsMore() const { return m_building || m_incomplete; }
    // Makes figures for about `budgetMicros` microseconds (and gives the next paint its steps); true if there is more to do after.
    bool step(qint64 budgetMicros);
    // Makes all the figures now, and lets the cells be worked out without a limit on what a paint may do (for a test, and for a tool that draws once).
    void settle();
    // What a limit stopped, since it was last asked: a sentence for the status bar, or empty. Each is said once for the sheet's rules.
    QString takeNotice() const;
    // Said by the painter: it asked about fewer cells of a part than it holds, for want of room.
    void reportTruncated() const;

    // Counted work: the rule tests made (formulas worked out), the passes over the cells of a set of ranges (a set that several rules have is passed over once), the cells the figures
    // are held for, and the figures of a cell that were changed one cell at a time.
    qint64 tests() const { return m_tests; }
    qint64 passes() const { return m_passes; }
    qint64 statCells() const { return m_statCells; }
    qint64 incrementalChanges() const { return m_incremental; }
    // The rectangles of the index and the entries in them that were looked at to find the rules of cells.
    qint64 indexSteps() const { return m_indexSteps; }

private:
    struct RuleState;
    struct RangeStats;
    struct Index;
    struct RangeSetLess {
        bool operator()(const std::vector<jp::office::sheet::CellRange> &a, const std::vector<jp::office::sheet::CellRange> &b) const
        {
            return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](const jp::office::sheet::CellRange &x, const jp::office::sheet::CellRange &y) {
                return std::tie(x.row1, x.col1, x.row2, x.col2) < std::tie(y.row1, y.col1, y.row2, y.col2);
            });
        }
    };
    enum class Ready { Yes, Pending, Never };
    enum class Limit { StatCells, Ranges, FormulaWork, Truncated };
    RuleState &state(size_t rule) const;
    Ready prepare(size_t rule) const;
    void derive(size_t rule) const;
    RangeStats *statsFor(size_t rule) const;
    void advance(RangeStats *stats, qint64 cells) const;
    void applyChange(RangeStats *stats, quint32 row, quint32 col) const;
    const Index &index() const;
    void rulesAt(quint32 row, quint32 col, std::vector<int> *out) const;
    CondCell compute(quint32 row, quint32 col, bool *incomplete) const;
    bool testOf(size_t rule, quint32 row, quint32 col) const;
    int formatIndexOf(const jp::office::sheet::DifferentialFormat &merged) const;
    void noteLimit(Limit which) const;
    const GridSheet *m_owner;
    mutable std::vector<std::unique_ptr<RuleState>> m_states;
    mutable std::map<std::vector<jp::office::sheet::CellRange>, std::unique_ptr<RangeStats>, RangeSetLess> m_stats;   // the figures, by the ranges they are of
    mutable std::unique_ptr<Index> m_index;
    mutable std::unordered_map<quint64, CondCell> m_cache;
    mutable bool m_stale = false;   // the results kept are of values that have changed: let go when next asked for (clearing a big table for each of a batch of values would cost more than they do)
    mutable std::deque<jp::office::sheet::DifferentialFormat> m_formats;
    mutable qint64 m_tests = 0, m_passes = 0, m_statCells = 0, m_incremental = 0, m_indexSteps = 0;
    Limits m_limits;
    mutable qint64 m_paintWork = 0;
    mutable bool m_building = false, m_incomplete = false, m_unlimited = false;
    mutable unsigned m_noticesGiven = 0, m_noticesWanted = 0;   // by Limit
    mutable std::vector<int> m_scratch;
    double m_today = 0;
    bool m_todayFixed = false;   // the day was given (else it is the clock's, taken again whenever the rules are new)
    mutable jp::calc::Evaluator m_evaluator;
};

} // namespace jp::grid
