#pragma once
// What a sheet's rules of conditional formatting do to the cells that are shown: for a cell, the rules whose ranges hold it are looked at in the order of their priority, and what they
// say is put together (a differential format changes the cell's font, fill, border, or number format; a color scale gives the fill a color; a data bar a bar; an icon set an icon), until a
// rule that says to stop. Only the cells that are drawn are worked out, the results are kept until a value changes, and what a rule needs of all its cells (the lowest and highest, the
// average, the number of times a value is there) is worked out once for the rule and kept until one of its cells changes.

#include "calc/eval.h"
#include "calc/formula.h"
#include "sheet/sheetmodel.h"
#include "sheet/styles.h"

#include <QRgb>

#include <deque>
#include <memory>
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
    explicit CondFormats(const GridSheet *owner);
    ~CondFormats();
    CondFormats(const CondFormats &) = delete;
    CondFormats &operator=(const CondFormats &) = delete;
    // The sheet's rules are new or changed (or its cells): everything that was worked out is let go.
    void reset();
    // Values changed: the results kept are let go, and so are the figures of the rules whose cells are among them.
    void valuesChanged();
    void valueChanged(quint32 row, quint32 col);
    // Whether the sheet has rules at all.
    bool any() const;
    // The day number TODAY() and the rules of dates read (the clock's day by default).
    void setToday(double serial)
    {
        m_today = serial;
        m_todayFixed = true;
        reset();
    }
    // What the rules do to the cell. The reference holds until the next call that may change what is kept.
    const CondCell &at(quint32 row, quint32 col) const;
    // The differential format a result names.
    const jp::office::sheet::DifferentialFormat &format(int index) const;
    // Whether the cell is in the range of some rule.
    bool covers(quint32 row, quint32 col) const;
    // The name of the icon set of a rule (by its place in the sheet's rules).
    QString iconSetName(int rule) const;
    // Counted work: the rule tests made (formulas worked out) and the passes over the cells of a rule's ranges.
    qint64 tests() const { return m_tests; }
    qint64 passes() const { return m_passes; }

private:
    struct RuleState;
    RuleState &state(size_t rule) const;
    void prepare(size_t rule) const;
    CondCell compute(quint32 row, quint32 col) const;
    bool testOf(size_t rule, quint32 row, quint32 col) const;
    double resolve(const jp::office::sheet::CondValue &v, RuleState &st, const std::vector<double> &sorted, double lo, double hi, size_t rule) const;
    int formatIndexOf(const jp::office::sheet::DifferentialFormat &merged) const;
    const GridSheet *m_owner;
    mutable std::vector<std::unique_ptr<RuleState>> m_states;
    mutable std::unordered_map<quint64, CondCell> m_cache;
    mutable std::deque<jp::office::sheet::DifferentialFormat> m_formats;
    mutable qint64 m_tests = 0, m_passes = 0;
    double m_today = 0;
    bool m_todayFixed = false;   // the day was given (else it is the clock's, taken again whenever the rules are new)
    mutable jp::calc::Evaluator m_evaluator;
};

} // namespace jp::grid
