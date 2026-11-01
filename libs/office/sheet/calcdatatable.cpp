// What-if (data) tables: the cells of a table are what the table's formula comes to when the cell it names as its input holds, in turn, each value at the edge of the table. Two edges and
// two inputs make a table of both (the formula is the cell at the corner); one input makes a table of one edge: with a column input, the values go down the column at the left and the
// formulas are in the row above the table; with a row input, they go across the row above and the formulas are in the column at the left.
//
// A table is worked out after the formulas, from the values they have then: the formula (and the formulas that read the input, directly or through others) is worked out again with the
// input read as the edge's value, in a source of cells of its own that changes only what it is asked to, so that nothing of the workbook is touched while it is done. What the table comes to
// is the value of its first cell, as an array's is (the cells of the table read it, as those of an array do), and is told to the window as a formula's results are. The worker never writes a
// cell of the sheet: the one place the cells of a table are written is markTableMembers, which marks them as an array's members (as load does for an array's), when the formulas are
// registered, before the worker serves anyone (on the loading thread, or with the worker held back for an edit that adds a sheet). The values they hold are still the file's.
//
// A table is worked out again only when something it read has changed: what it read is kept (each cell or area, and a hash of what was there), and looked at again when the formulas have been
// worked out. Tables that read each other's cells are worked out in the order they depend on each other (one that depends on a circle of them is left as the file has it), and all of
// them together take only the work a recalculation may.

#include "sheet/calculation.h"

#include "calc/eval.h"

#include <QCoreApplication>

#include <array>
#include <cstring>
#include <functional>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace jp::office::sheet {

using jp::calc::CellArea;
using jp::calc::CellSource;
using jp::calc::ErrorKind;
using jp::calc::EvalContext;
using jp::calc::EvalStatus;
using jp::calc::Evaluator;
using jp::calc::Value;

namespace {

constexpr int kMaxTableDepth = 64;
constexpr size_t kMaxTableReads = 400000;

struct Input {
    int sheet = 0;
    quint32 row = 0, col = 0;
    Value value;
};

quint64 mixIn(quint64 h, quint64 x)
{
    h ^= x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
}

quint64 valueHash(const Value &v)
{
    quint64 h = quint64(v.kind()) + 1;
    switch (v.kind()) {
    case Value::Kind::Number: {
        double d = v.number();
        if (d == 0) d = 0;   // (minus zero is zero)
        quint64 bits;
        std::memcpy(&bits, &d, 8);
        return mixIn(h, bits);
    }
    case Value::Kind::Bool: return mixIn(h, v.boolean() ? 1 : 0);
    case Value::Kind::Error: return mixIn(h, quint64(int(v.errorKind())));
    case Value::Kind::Text: return mixIn(h, quint64(qHash(v.text())));
    default: return h;
    }
}

}

// The cells as a data table's formula sees them with its inputs set: the inputs are the values asked for, a formula that depends on an input is worked out again (once for each set of
// inputs), and every other cell is as the workbook has it. What it reads of the workbook is kept: the cells (one by one) and the areas it looked at.
class Calculation::WhatIf : public CellSource {
public:
    WhatIf(Calculation &calc, const std::unordered_set<quint32> &closure) : m_calc(calc), m_closure(closure) {}

    void setInputs(const std::vector<Input> &inputs)
    {
        m_inputs = inputs;
        m_memo.clear();
        m_busy.clear();
        m_depth = 0;
    }

    // Sets a value worked out already (the formulas are worked out in the order they depend on each other, so that none waits for a deep chain of the others).
    void remember(quint32 id, const Value &v) { m_memo[id] = v; }
    bool knows(quint32 id) const { return m_memo.count(id) != 0; }
    Value evaluateNode(quint32 id) { return evaluate(id); }
    // True once the work was stopped, or ran out: what was worked out since is of no worth.
    bool stopped() const { return m_stopped; }
    bool outOfWork() const { return m_outOfWork; }
    // The cell as the workbook has it (kept as read).
    Value baseCell(int sheet, quint32 row, quint32 col)
    {
        noteCell(sheet, row, col);
        return m_calc.cell(sheet, row, col);
    }
    // What was read of the workbook, one cell at a time and by areas (each only once).
    const std::vector<std::array<quint32, 5>> &cellsRead() const { return m_cellsRead; }
    const std::vector<std::array<quint32, 5>> &areasRead() const { return m_areasRead; }
    bool tooMuchRead() const { return m_tooMuch; }

    // The value of a formula under the inputs (the formula itself is not in the closure if it does not depend on one: then it is what the workbook has).
    Value valueOf(quint32 id)
    {
        if (!m_closure.count(id)) {
            const Node &n = m_calc.m_nodes[id];
            noteCell(n.sheet, m_calc.rowOf(n), m_calc.colOf(n));
            return m_calc.nodeValue(n, id);
        }
        const auto done = m_memo.find(id);
        if (done != m_memo.end()) return done->second;
        if (m_busy.count(id) || m_depth >= kMaxTableDepth) return Value::error(ErrorKind::Value);
        m_busy.insert(id);
        ++m_depth;
        const Value v = evaluate(id);
        --m_depth;
        m_busy.erase(id);
        m_memo.emplace(id, v);
        return v;
    }

    int sheetCount() const override { return m_calc.sheetCount(); }
    int sheetIndex(const QString &name) const override { return m_calc.sheetIndex(name); }
    Value cell(int sheet, quint32 row, quint32 col) const override
    {
        for (const Input &in : m_inputs)
            if (in.sheet == sheet && in.row == row && in.col == col) return in.value;
        if (const quint32 id = nodeAt(sheet, row, col); id != kNone && m_closure.count(id)) return const_cast<WhatIf *>(this)->valueOf(id);
        const_cast<WhatIf *>(this)->noteCell(sheet, row, col);
        return m_calc.cell(sheet, row, col);
    }
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const override
    {
        const_cast<WhatIf *>(this)->noteArea(sheet, row1, col1, row2, col2);
        // the cells the workbook has in the block, each as this source sees it, and then the inputs in the block that the workbook has no cell for
        std::vector<std::pair<quint32, quint32>> seen;
        bool stopped = false;
        m_calc.visit(sheet, row1, col1, row2, col2, [&](quint32 r, quint32 c, const Value &v) {
            Value shown = v;
            bool known = false;
            for (const Input &in : m_inputs)
                if (in.sheet == sheet && in.row == r && in.col == c) {
                    shown = in.value;
                    known = true;
                }
            if (known) seen.push_back({r, c});
            else if (const quint32 id = nodeAt(sheet, r, c); id != kNone && m_closure.count(id)) shown = const_cast<WhatIf *>(this)->valueOf(id);
            if (!f(r, c, shown)) {
                stopped = true;
                return false;
            }
            return true;
        });
        if (stopped) return;
        for (const Input &in : m_inputs) {
            if (in.sheet != sheet || in.row < row1 || in.row > row2 || in.col < col1 || in.col > col2) continue;
            if (std::find(seen.begin(), seen.end(), std::make_pair(in.row, in.col)) != seen.end()) continue;
            if (!f(in.row, in.col, in.value)) return;
        }
    }
    void usedExtent(int sheet, quint32 *rows, quint32 *cols) const override { m_calc.usedExtent(sheet, rows, cols); }
    QString definedName(int sheet, const QString &name) const override { return m_calc.definedName(sheet, name); }
    jp::calc::StructuredAnswer structuredRef(const QString &text, int sheet, quint32 row, quint32 col, CellArea *area, ErrorKind *error) const override
    {
        return m_calc.structuredRef(text, sheet, row, col, area, error);
    }
    int rowHiddenKind(int sheet, quint32 row) const override { return m_calc.rowHiddenKind(sheet, row); }
    bool rowRange(int sheet, quint32 startId, quint32 endId, quint32 *first, quint32 *last) const override { return m_calc.rowRange(sheet, startId, endId, first, last); }
    bool colRange(int sheet, quint32 startId, quint32 endId, quint32 *first, quint32 *last) const override { return m_calc.colRange(sheet, startId, endId, first, last); }
    QString heldName(int sheet, const QString &name, bool *inIds) const override { return m_calc.heldName(sheet, name, inIds); }
    bool isSubtotal(int sheet, quint32 row, quint32 col) const override { return m_calc.isSubtotal(sheet, row, col); }
    qint64 generation() const override { return m_calc.generation(); }

private:
    Calculation &m_calc;
    const std::unordered_set<quint32> &m_closure;
    std::vector<Input> m_inputs;
    std::unordered_map<quint32, Value> m_memo;
    std::unordered_set<quint32> m_busy;
    int m_depth = 0;
    Evaluator m_evaluator;
    bool m_stopped = false, m_outOfWork = false, m_tooMuch = false;
    std::vector<std::array<quint32, 5>> m_cellsRead, m_areasRead;   // sheet, then row and column (a cell), or the four corners (an area)
    std::unordered_set<quint64> m_cellKeys;
    std::set<std::array<quint32, 5>> m_areaKeys;

    quint32 nodeAt(int sheet, quint32 row, quint32 col) const { return m_calc.nodeOfCell(sheet, row, col); }

    void noteCell(int sheet, quint32 row, quint32 col)
    {
        const quint64 key = (quint64(quint32(sheet)) << 52) ^ (quint64(row) << 20) ^ quint64(col);
        if (!m_cellKeys.insert(key).second) return;
        if (m_cellsRead.size() >= kMaxTableReads) {
            m_tooMuch = true;
            return;
        }
        m_cellsRead.push_back({quint32(sheet), row, col, row, col});
    }
    void noteArea(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2)
    {
        const std::array<quint32, 5> a{quint32(sheet), row1, col1, row2, col2};
        if (!m_areaKeys.insert(a).second) return;
        if (m_areasRead.size() >= kMaxTableReads) {
            m_tooMuch = true;
            return;
        }
        m_areasRead.push_back(a);
    }

    Value evaluate(quint32 id)
    {
        const Node &n = m_calc.m_nodes[id];
        const Sheet &s = m_calc.m_sheets[size_t(n.sheet)];
        const Formula &f = s.formulas[n.formula];
        const Compiled &slot = m_calc.compiled(n.sheet, f.text, n.flags & Array);
        if (!slot.program) return m_calc.nodeValue(n, id);
        if (m_calc.m_workUsed >= m_calc.m_workLimit) {
            m_outOfWork = true;
            return Value::error(ErrorKind::Value);
        }
        EvalContext ctx = m_calc.m_ctx;
        ctx.source = this;
        ctx.sheet = n.sheet;
        ctx.transition = s.transitionEvaluation;
        ctx.row = m_calc.rowOf(n);
        ctx.col = m_calc.colOf(n);
        ctx.shiftRows = f.dRow;
        ctx.shiftCols = f.dCol;
        ctx.idFrame = inIds(s, f.text);
        ctx.now = m_calc.m_now;
        ctx.touched = nullptr;
        m_evaluator.setWorkLimit(m_calc.m_workLimit > 0 ? m_calc.m_workLimit : 0);
        Value v = m_evaluator.evaluate(*slot.program, ctx);
        m_calc.m_workUsed += m_evaluator.work() + 8;
        if (m_evaluator.status() == EvalStatus::Cancelled || (ctx.cancel && ctx.cancel->load(std::memory_order_relaxed))) m_stopped = true;
        if (v.isArray()) v = v.at(0, 0);
        return v;
    }
};

quint32 Calculation::nodeOfCell(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return kNone;
    const Cell *c = m_sheets[size_t(sheet)].cells.find(row, col);
    if (!c || !c->hasFormula() || c->formula < 0) return kNone;
    const SheetState &st = m_state[size_t(sheet)];
    return size_t(c->formula) < st.nodeOf.size() ? st.nodeOf[size_t(c->formula)] : kNone;
}

namespace {

bool sameValue(const Value &a, const Value &b)
{
    if (a.isNumber() && b.isNumber()) return jp::calc::sameTo15Digits(a.number(), b.number());
    if (a.isArray() || b.isArray()) return false;
    return a.debug() == b.debug();
}

// The inputs a table's text names: "TABLE(row input,column input)", either of which may be empty.
bool tableInputs(const QString &text, quint32 *r1, quint32 *c1, bool *hasRow, quint32 *r2, quint32 *c2, bool *hasCol)
{
    *hasRow = *hasCol = false;
    if (!text.startsWith(QLatin1String("TABLE(")) || !text.endsWith(QLatin1Char(')'))) return false;
    const QStringList parts = text.mid(6, text.size() - 7).split(QLatin1Char(','));
    if (parts.size() != 2) return false;
    const QString a = QString(parts[0]).remove(QLatin1Char('$')).trimmed(), b = QString(parts[1]).remove(QLatin1Char('$')).trimmed();
    if (!a.isEmpty()) {
        if (!parseCellRef(a, r1, c1)) return false;
        *hasRow = true;
    }
    if (!b.isEmpty()) {
        if (!parseCellRef(b, r2, c2)) return false;
        *hasCol = true;
    }
    return *hasRow || *hasCol;
}

CellArea cellArea(int sheet, quint32 row, quint32 col)
{
    CellArea a;
    a.sheet1 = a.sheet2 = sheet;
    a.row1 = a.row2 = row;
    a.col1 = a.col2 = col;
    return a;
}

bool areasMeet(const CellArea &a, const CellRange &r, int sheet)
{
    return a.sheet1 <= sheet && sheet <= a.sheet2 && a.row1 <= r.row2 && a.row2 >= r.row1 && a.col1 <= r.col2 && a.col2 >= r.col1;
}

} // namespace

// The cells of a table that is worked out read its value, as those of an array do (the first cell holds the array; what the file stored is its value until the table is worked out). A table too large
// to work out has its cells marked as the file's. This writes the flags of the cells (and adds the cells a table has none for), once, as the formulas are registered: never while a worker is
// working formulas out, so no cell is written and read at once; the values in the cells are not changed.
void Calculation::markTableMembers(quint32 id)
{
    Node &n = m_nodes[id];
    Sheet &s = m_sheets[size_t(n.sheet)];
    const qint32 rangeAt = n.formula >= 0 ? s.formulas[n.formula].range : -1;
    if (rangeAt < 0 || rangeAt >= s.ranges.size() || !s.ranges[rangeAt].valid || s.ranges[rangeAt].cellCount() > kMaxArrayCellsMarked) return;
    const CellRange r = s.ranges[rangeAt];
    if (!m_options.workOutDataTables || r.cellCount() > kMaxTableCells || r.cellCount() < 1) {
        for (quint32 row = r.row1; row <= r.row2; ++row)
            for (quint32 col = r.col1; col <= r.col2; ++col)
                if (Cell *c = s.cells.findMutable(row, col)) c->flags |= Tainted;
        return;
    }
    Value array = Value::array(int(r.row2 - r.row1 + 1), int(r.col2 - r.col1 + 1));
    if (array.isEmpty()) return;
    for (quint32 row = r.row1; row <= r.row2; ++row)
        for (quint32 col = r.col1; col <= r.col2; ++col) {
            Cell *c = s.cells.findMutable(row, col);
            const bool first = row == rowOf(n) && col == colOf(n);
            if (first) {
                if (c) array.set(int(row - r.row1), int(col - r.col1), valueOf(s, *c));
                continue;
            }
            if (!c) {
                Cell member;
                member.flags |= ArrayMember;
                member.formula = n.formula;
                s.cells.set(row, col, member);
                m_rows[size_t(n.sheet)] = std::max(m_rows[size_t(n.sheet)], row + 1);
                m_cols[size_t(n.sheet)] = std::max(m_cols[size_t(n.sheet)], col + 1);
                continue;
            }
            if (c->hasFormula()) continue;
            array.set(int(row - r.row1), int(col - r.col1), valueOf(s, *c));
            c->flags |= ArrayMember;
            c->formula = n.formula;
        }
    storeResult(n, id, array, false);
}

// The structure of a table: its inputs, the formulas that depend on them (the closure) in the order they are worked out, the formulas it is made of, and the formulas that depend on its own cells.
bool Calculation::prepareTable(quint32 id, TableCache &t)
{
    t.prepared = true;
    t.structGen = m_structGen;
    t.worked = false;
    t.reads.clear();
    t.closure.clear();
    t.forward.clear();
    t.watched.clear();
    t.order.clear();
    t.precedents.clear();
    t.edges.clear();
    t.dependsOn.clear();
    t.left = TableCache::Left::Unsafe;
    Node &n = m_nodes[id];
    Sheet &s = m_sheets[size_t(n.sheet)];
    const Cell *first = s.cells.findById(n.rowId, n.colId);
    if (!first || !(first->flags & DataTable) || n.formula < 0) return false;
    const qint32 rangeAt = s.formulas[n.formula].range;
    if (rangeAt < 0 || rangeAt >= s.ranges.size() || !s.ranges[rangeAt].valid) return false;
    t.range = s.ranges[rangeAt];
    t.sheet = n.sheet;
    const CellRange &range = t.range;
    if (range.cellCount() > kMaxTableCells) {
        t.left = TableCache::Left::Size;
        return false;
    }
    if (range.row1 == 0 || range.col1 == 0 || range.row2 >= jp::calc::kGridRows || range.col2 >= kMaxColumns) return false;
    const QString text = s.formulaText(*first);
    if (!tableInputs(text, &t.inRow, &t.inRowCol, &t.hasRow, &t.inCol, &t.inColCol, &t.hasCol)) return false;
    t.key = QStringLiteral("%1|%2|%3:%4:%5:%6|%7").arg(text).arg(t.sheet).arg(range.row1).arg(range.col1).arg(range.row2).arg(range.col2).arg(id);
    const int sheet = n.sheet;
    // the formulas that depend on the inputs, directly or through others: only these are worked out again for each value
    bool unsafe = false;   // a formula that depends on an input is one the engine keeps apart (a function it lacks, a volatile one, another workbook's cell): the table is not worked out
    std::unordered_map<quint32, std::vector<quint32>> readersOfNode;   // within the closure
    {
        std::vector<std::pair<CellArea, quint32>> frontier;   // an area, and the formula it is the cell of (kNone for an input)
        if (t.hasRow) frontier.push_back({cellArea(sheet, t.inRow, t.inRowCol), kNone});
        if (t.hasCol) frontier.push_back({cellArea(sheet, t.inCol, t.inColCol), kNone});
        std::vector<quint32> found;
        qint64 steps = 0;
        while (!frontier.empty() && steps++ < 4000000) {
            const auto [a, from] = frontier.back();
            frontier.pop_back();
            found.clear();
            readersOf(a, &found, nullptr);
            for (const quint32 reader : found) {
                if ((m_nodes[reader].flags & Dead) || m_tableNodes.count(reader)) continue;
                if (from != kNone) {
                    readersOfNode[from].push_back(reader);
                    t.precedents[reader].push_back(from);
                }
                if (t.closure.count(reader)) continue;
                if (m_nodes[reader].flags & (Kept | Always)) unsafe = true;
                t.closure.insert(reader);
                frontier.push_back({areaOf(m_nodes[reader]), reader});
            }
        }
        if (!frontier.empty()) unsafe = true;   // (more than the work allows)
    }
    if (unsafe) return false;
    // the order the formulas of the closure are worked out in: each after those it reads
    {
        std::unordered_map<quint32, int> waiting;
        for (const quint32 f : t.closure) waiting[f] = int(t.precedents[f].size());
        std::vector<quint32> ready;
        for (const quint32 f : t.closure)
            if (waiting[f] == 0) ready.push_back(f);
        while (!ready.empty()) {
            const quint32 f = ready.back();
            ready.pop_back();
            t.order.push_back(f);
            for (const quint32 reader : readersOfNode[f])
                if (--waiting[reader] == 0) ready.push_back(reader);
        }
        // (formulas in a circle are left at the end, and worked out as they are asked for)
        if (t.order.size() < t.closure.size()) {
            std::unordered_set<quint32> placed(t.order.begin(), t.order.end());
            for (const quint32 f : t.closure)
                if (!placed.count(f)) t.order.push_back(f);
        }
    }
    // what the table is made of: the formulas of its edges, and the cells of its edges
    t.watched = t.closure;
    auto formulaAt = [&](quint32 row, quint32 col) {
        t.edges.push_back(cellArea(sheet, row, col));
        const quint32 f = nodeOfCell(sheet, row, col);
        if (f != kNone) t.watched.insert(f);
    };
    for (quint32 row = range.row1; row <= range.row2; ++row)
        for (quint32 col = range.col1; col <= range.col2; ++col) {
            if (row == range.row1 && t.hasRow) t.edges.push_back(cellArea(sheet, range.row1 - 1, col));
            if (col == range.col1 && t.hasCol) t.edges.push_back(cellArea(sheet, row, range.col1 - 1));
            if (t.hasRow && !t.hasCol) {
                if (col == range.col1) formulaAt(row, range.col1 - 1);
            } else if (t.hasCol && !t.hasRow) {
                if (row == range.row1) formulaAt(range.row1 - 1, col);
            } else if (row == range.row1 && col == range.col1) {
                formulaAt(range.row1 - 1, range.col1 - 1);
            }
        }
    // the formulas that depend on the table's own cells, through others as well
    {
        std::vector<CellArea> frontier;
        CellArea own;
        own.sheet1 = own.sheet2 = sheet;
        own.row1 = range.row1;
        own.row2 = range.row2;
        own.col1 = range.col1;
        own.col2 = range.col2;
        frontier.push_back(own);
        std::vector<quint32> found;
        qint64 steps = 0;
        while (!frontier.empty() && steps++ < 4000000) {
            const CellArea a = frontier.back();
            frontier.pop_back();
            found.clear();
            readersOf(a, &found, nullptr);
            for (const quint32 reader : found) {
                if ((m_nodes[reader].flags & Dead) || m_tableNodes.count(reader)) continue;
                if (!t.forward.insert(reader).second) continue;
                frontier.push_back(areaOf(m_nodes[reader]));
            }
        }
    }
    t.left = TableCache::Left::No;
    return true;
}

// The hash of what a read found at the time (the inputs, and the formulas that depend on them, are not the workbook's to read: they are as the table sets them).
quint64 Calculation::tableReadHash(const TableCache &t, const TableRead &r) const
{
    auto input = [&](quint32 row, quint32 col) {
        return r.sheet == t.sheet && ((t.hasRow && row == t.inRow && col == t.inRowCol) || (t.hasCol && row == t.inCol && col == t.inColCol));
    };
    if (r.row1 == r.row2 && r.col1 == r.col2) {
        if (input(r.row1, r.col1)) return 0;
        const quint32 node = nodeOfCell(r.sheet, r.row1, r.col1);
        if (node != kNone && t.closure.count(node)) return 0;
        return valueHash(cell(r.sheet, r.row1, r.col1));
    }
    quint64 h = 7;
    visit(r.sheet, r.row1, r.col1, r.row2, r.col2, [&](quint32 row, quint32 col, const Value &v) {
        if (input(row, col)) return true;
        const quint32 node = nodeOfCell(r.sheet, row, col);
        if (node != kNone && t.closure.count(node)) return true;
        h = mixIn(mixIn(h, (quint64(row) << 20) | col), valueHash(v));
        return true;
    });
    return h;
}

// Whether a table is as it was worked out: the same structure and the same things read.
bool Calculation::tableCurrent(const TableCache &t) const
{
    if (!t.worked || t.structGen != m_structGen) return false;
    for (const TableRead &r : t.reads)
        if (tableReadHash(t, r) != r.hash) return false;
    return true;
}

// Works out one table. The places of the cells that came out different are put in `changed`.
Calculation::TableOutcome Calculation::workOutTable(quint32 id, TableCache &t, std::vector<CellArea> *changed)
{
    Node &n = m_nodes[id];
    const int sheet = n.sheet;
    const CellRange range = t.range;
    WhatIf source(*this, t.closure);
    auto stopRequested = [&] { return source.stopped() || (m_ctx.cancel && m_ctx.cancel->load(std::memory_order_relaxed)); };
    std::unordered_map<quint32, std::vector<quint32>> neededFor;   // by the formula of the table: the part of the order it needs
    auto orderFor = [&](quint32 formulaNode) -> const std::vector<quint32> & {
        auto have = neededFor.find(formulaNode);
        if (have != neededFor.end()) return have->second;
        std::unordered_set<quint32> needed;
        std::vector<quint32> stack{formulaNode};
        needed.insert(formulaNode);
        while (!stack.empty()) {
            const quint32 f = stack.back();
            stack.pop_back();
            const auto pre = t.precedents.find(f);
            if (pre == t.precedents.end()) continue;
            for (const quint32 p : pre->second)
                if (needed.insert(p).second) stack.push_back(p);
        }
        std::vector<quint32> mine;
        for (const quint32 f : t.order)
            if (needed.count(f)) mine.push_back(f);
        return neededFor.emplace(formulaNode, std::move(mine)).first->second;
    };
    // the formula of a cell of the table: at the corner for two inputs, above its column for a column input, at the left of its row for a row input
    const quint32 rows = range.row2 - range.row1 + 1, cols = range.col2 - range.col1 + 1;
    Value results = Value::array(int(rows), int(cols));
    if (results.isEmpty()) return TableOutcome::Left;
    m_tablesWorked.fetch_add(1, std::memory_order_relaxed);
    for (quint32 row = range.row1; row <= range.row2; ++row) {
        for (quint32 col = range.col1; col <= range.col2; ++col) {
            if (stopRequested()) return TableOutcome::Stopped;
            if (m_workUsed >= m_workLimit || source.outOfWork()) {
                t.left = TableCache::Left::Cost;
                return TableOutcome::Left;
            }
            std::vector<Input> inputs;
            if (t.hasRow) inputs.push_back({sheet, t.inRow, t.inRowCol, source.baseCell(sheet, range.row1 - 1, col)});
            if (t.hasCol) inputs.push_back({sheet, t.inCol, t.inColCol, source.baseCell(sheet, row, range.col1 - 1)});
            quint32 fRow = range.row1 - 1, fCol = range.col1 - 1;
            if (t.hasRow && !t.hasCol) fRow = row;
            else if (t.hasCol && !t.hasRow) fCol = col;
            Value v;
            const quint32 formulaNode = nodeOfCell(sheet, fRow, fCol);
            source.setInputs(inputs);
            if (formulaNode == kNone || !t.closure.count(formulaNode)) {
                v = source.baseCell(sheet, fRow, fCol);
            } else {
                // (each formula it needs, after those it reads)
                for (const quint32 need : orderFor(formulaNode))
                    if (!source.knows(need)) source.remember(need, source.evaluateNode(need));
                v = source.valueOf(formulaNode);
            }
            if (stopRequested()) return TableOutcome::Stopped;
            if (source.outOfWork()) {
                t.left = TableCache::Left::Cost;
                return TableOutcome::Left;
            }
            if (v.isArray()) v = v.at(0, 0);
            // (a table that comes to a text is left as the file has it: the texts of a sheet are read by the window as the formulas are worked out, and are not added to from here)
            if (v.isText()) {
                t.left = TableCache::Left::Unsafe;
                return TableOutcome::Left;
            }
            results.set(int(row - range.row1), int(col - range.col1), v);
            ++m_workUsed;
        }
    }
    // what it read, kept for the next time it is asked whether it is as it was worked out
    t.reads.clear();
    if (source.tooMuchRead()) {
        t.worked = false;   // (read more than is kept: it is worked out whenever it is asked)
    } else {
        for (const auto &c : source.cellsRead()) t.reads.push_back({int(c[0]), c[1], c[2], c[3], c[4], 0});
        for (const auto &a : source.areasRead()) t.reads.push_back({int(a[0]), a[1], a[2], a[3], a[4], 0});
        for (TableRead &r : t.reads) r.hash = tableReadHash(t, r);
        t.worked = true;
    }
    // the tables whose cells it read
    t.dependsOn.clear();
    for (const TableRead &r : t.reads)
        for (const quint32 other : m_tableNodes) {
            if (other == id) continue;
            const auto have = m_tableCache.find(other);
            if (have == m_tableCache.end() || !have->second.prepared) continue;
            CellArea a;
            a.sheet1 = a.sheet2 = r.sheet;
            a.row1 = r.row1;
            a.row2 = r.row2;
            a.col1 = r.col1;
            a.col2 = r.col2;
            if (have->second.sheet == r.sheet && areasMeet(a, have->second.range, r.sheet) && std::find(t.dependsOn.begin(), t.dependsOn.end(), other) == t.dependsOn.end()) t.dependsOn.push_back(other);
        }
    // what came out different from the table's cells now
    const Value was = m_arrays.value(id);
    qint64 differing = 0;
    for (quint32 row = range.row1; row <= range.row2; ++row)
        for (quint32 col = range.col1; col <= range.col2; ++col) {
            const int i = int(row - range.row1), j = int(col - range.col1);
            const Value before = was.isArray() && i < was.rows() && j < was.cols() ? was.at(i, j) : Value();
            if (sameValue(before, results.at(i, j))) continue;
            ++differing;
            changed->push_back(cellArea(sheet, row, col));
        }
    if (differing > 0) {
        m_tableCellsChanged += differing;
        storeResult(n, id, results, true);
    }
    return TableOutcome::Worked;
}

bool Calculation::dataTablePass()
{
    const bool force = m_tablesForce;
    m_tablesStale = false;
    m_tablesForce = false;
    std::vector<quint32> ids;
    for (const quint32 id : m_tableNodes)
        if (id < m_nodes.size() && !(m_nodes[id].flags & Dead) && !dormant(m_nodes[id])) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    // the structure of each, when it is not the structure now
    for (const quint32 id : ids) {
        TableCache &t = m_tableCache[id];
        if (!t.prepared || t.structGen != m_structGen) prepareTable(id, t);
        if (t.left == TableCache::Left::Cost || t.left == TableCache::Left::Circle) t.left = TableCache::Left::No;   // (asked again: the work this recalculation has left, and the circle as the tables are now)
    }
    // the order they are worked out in: each after the tables whose cells its formulas read; tables in a circle of them, or that depend on one, are left alone
    std::unordered_map<quint32, std::vector<quint32>> after;   // the tables that wait for a table
    std::unordered_map<quint32, int> waiting;
    std::vector<quint32> live;
    for (const quint32 id : ids)
        if (m_tableCache[id].left == TableCache::Left::No) live.push_back(id);
    for (const quint32 id : live) waiting[id] = 0;
    for (const quint32 b : live) {
        const TableCache &tb = m_tableCache[b];
        std::vector<quint32> deps = tb.dependsOn;
        for (const quint32 a : live) {
            if (a == b) continue;
            const TableCache &ta = m_tableCache[a];
            bool reads = false;
            for (const quint32 f : tb.watched)
                if (ta.forward.count(f)) {
                    reads = true;
                    break;
                }
            if (!reads)
                for (const CellArea &e : tb.edges)
                    if (e.sheet1 == ta.sheet && areasMeet(e, ta.range, ta.sheet)) {
                        reads = true;
                        break;
                    }
            if (reads && std::find(deps.begin(), deps.end(), a) == deps.end()) deps.push_back(a);
        }
        // (a table whose own formulas read its own cells reads itself)
        for (const quint32 f : tb.watched)
            if (tb.forward.count(f)) {
                m_tableCache[b].left = TableCache::Left::Circle;
                break;
            }
        for (const quint32 a : deps) {
            if (a == b || !waiting.count(a)) continue;
            after[a].push_back(b);
            ++waiting[b];
        }
    }
    std::vector<quint32> order;
    {
        std::vector<quint32> ready;
        for (const quint32 id : live)
            if (waiting[id] == 0 && m_tableCache[id].left == TableCache::Left::No) ready.push_back(id);
        std::sort(ready.begin(), ready.end(), std::greater<quint32>());
        while (!ready.empty()) {
            const quint32 id = ready.back();
            ready.pop_back();
            order.push_back(id);
            for (const quint32 b : after[id])
                if (--waiting[b] == 0 && m_tableCache[b].left == TableCache::Left::No) ready.push_back(b);
        }
    }
    {
        std::unordered_set<quint32> placed(order.begin(), order.end());
        for (const quint32 id : live)
            if (!placed.count(id)) m_tableCache[id].left = TableCache::Left::Circle;
    }
    std::vector<CellArea> changedPlaces;
    bool waitForDependents = false;
    for (size_t k = 0; k < order.size() && !waitForDependents; ++k) {
        const quint32 id = order[k];
        TableCache &t = m_tableCache[id];
        if (t.left != TableCache::Left::No) continue;
        if (!force && tableCurrent(t)) continue;
        if (m_workUsed >= m_workLimit) {
            t.left = TableCache::Left::Cost;
            continue;
        }
        const size_t before = changedPlaces.size();
        const TableOutcome outcome = workOutTable(id, t, &changedPlaces);
        if (outcome == TableOutcome::Stopped) {
            // (stopped in the middle of one: none of its cells is changed, and all are looked at again when the work is gone on with)
            m_tablesStale = true;
            m_tablesForce = force;
            updateTableCounts();
            return true;
        }
        if (outcome == TableOutcome::Worked && changedPlaces.size() > before) {
            // a table that reads this one waits until what reads this one's cells is worked out
            for (const quint32 b : after[id])
                if (m_tableCache[b].left == TableCache::Left::No) waitForDependents = true;
        }
    }
    updateTableCounts();
    if (changedPlaces.empty()) return false;
    // what reads the cells that changed is worked out after (the areas are told by the ids of their lines), and the tables are looked at again then
    std::vector<CellArea> ids2;
    for (const CellArea &a : changedPlaces) idAreasOn(a, false, &ids2);
    markDependentsOfIds(&ids2);
    m_tablesStale = true;
    return true;
}

void Calculation::updateTableCounts()
{
    qint64 costly = 0, circles = 0;
    for (const auto &entry : m_tableCache) {
        if (!m_tableNodes.count(entry.first)) continue;
        if (entry.second.left == TableCache::Left::Cost || entry.second.left == TableCache::Left::Size) ++costly;
        else if (entry.second.left == TableCache::Left::Circle) ++circles;
    }
    m_tablesTooCostly.store(costly, std::memory_order_relaxed);
    m_tablesInCircles.store(circles, std::memory_order_relaxed);
}

qint64 Calculation::workOutDataTablesNow()
{
    if (m_tableNodes.empty()) return 0;
    const qint64 before = m_tableCellsChanged;
    m_tablesStale = true;
    m_tablesForce = true;
    recalculate();
    return m_tableCellsChanged - before;
}

} // namespace jp::office::sheet
