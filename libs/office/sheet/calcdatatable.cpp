// What-if (data) tables: the cells of a table are what the table's formula comes to when the cell it names as its input holds, in turn, each value at the edge of the table. Two edges and
// two inputs make a table of both (the formula is the cell at the corner); one input makes a table of one edge: with a column input, the values go down the column at the left and the
// formulas are in the row above the table; with a row input, they go across the row above and the formulas are in the column at the left.
//
// A table is worked out after the formulas, from the values they have then: the formula (and the formulas that read the input, directly or through others) is worked out again with the
// input read as the edge's value, in a source of cells of its own that changes only what it is asked to, so that nothing of the workbook is touched while it is done. The cells of the
// table that come out different are then put in the sheet and told to the window, and the formulas that read them are worked out after.

#include "sheet/calculation.h"

#include "calc/eval.h"

#include <QCoreApplication>

#include <map>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace jp::office::sheet {

using jp::calc::CellArea;
using jp::calc::CellSource;
using jp::calc::ErrorKind;
using jp::calc::EvalContext;
using jp::calc::Evaluator;
using jp::calc::Value;

namespace {

constexpr qint64 kMaxTableCells = 200000;
constexpr int kMaxTableDepth = 64;

struct Input {
    int sheet = 0;
    quint32 row = 0, col = 0;
    Value value;
};

}

// The cells as a data table's formula sees them with its inputs set: the inputs are the values asked for, a formula that depends on an input is worked out again (once for each set of
// inputs), and every other cell is as the workbook has it.
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

    // The value of a formula under the inputs (the formula itself is not in the closure if it does not depend on one: then it is what the workbook has).
    Value valueOf(quint32 id)
    {
        if (!m_closure.count(id)) return m_calc.nodeValue(m_calc.m_nodes[id], id);
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
        return m_calc.cell(sheet, row, col);
    }
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const override
    {
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

    quint32 nodeAt(int sheet, quint32 row, quint32 col) const { return m_calc.nodeOfCell(sheet, row, col); }

    Value evaluate(quint32 id)
    {
        const Node &n = m_calc.m_nodes[id];
        const Sheet &s = m_calc.m_sheets[size_t(n.sheet)];
        const Formula &f = s.formulas[n.formula];
        const Compiled &slot = m_calc.compiled(n.sheet, f.text, n.flags & Array);
        if (!slot.program) return m_calc.nodeValue(n, id);
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

} // namespace

// Works out one table. The places of the cells that came out different are put in `changed` (as places); how many is returned.
qint64 Calculation::workOutTable(quint32 id, std::vector<CellArea> *changed)
{
    Node &n = m_nodes[id];
    Sheet &s = m_sheets[size_t(n.sheet)];
    const Cell *first = s.cells.findById(n.rowId, n.colId);
    if (!first || !(first->flags & DataTable) || n.formula < 0) return 0;
    const qint32 rangeAt = s.formulas[n.formula].range;
    if (rangeAt < 0 || rangeAt >= s.ranges.size() || !s.ranges[rangeAt].valid) return 0;
    const CellRange range = s.ranges[rangeAt];
    if (range.cellCount() > kMaxTableCells || range.row1 == 0 || range.col1 == 0 || range.row2 >= jp::calc::kGridRows || range.col2 >= kMaxColumns) return 0;
    quint32 rr = 0, rc = 0, cr = 0, cc = 0;
    bool hasRow = false, hasCol = false;
    if (!tableInputs(s.formulaText(*first), &rr, &rc, &hasRow, &cr, &cc, &hasCol)) return 0;
    const int sheet = n.sheet;
    // the formulas that depend on the inputs, directly or through others: only these are worked out again for each value
    std::unordered_set<quint32> closure;
    bool unsafe = false;   // a formula that depends on an input is one the engine keeps apart (a function it lacks, a volatile one, another workbook's cell): the table is not worked out
    std::unordered_map<quint32, std::vector<quint32>> readersOfNode, precedentsOfNode;   // within the closure
    std::vector<quint32> roots;   // the formulas that read an input itself
    {
        std::vector<std::pair<CellArea, quint32>> frontier;   // an area, and the formula it is the cell of (kNone for an input)
        auto add = [&](quint32 row, quint32 col) {
            CellArea a;
            a.sheet1 = a.sheet2 = sheet;
            a.row1 = a.row2 = row;
            a.col1 = a.col2 = col;
            frontier.push_back({a, kNone});
        };
        if (hasRow) add(rr, rc);
        if (hasCol) add(cr, cc);
        std::vector<quint32> found;
        qint64 steps = 0;
        while (!frontier.empty() && steps++ < 4000000) {
            const auto [a, from] = frontier.back();
            frontier.pop_back();
            found.clear();
            readersOf(a, &found, nullptr);
            for (const quint32 reader : found) {
                if ((m_nodes[reader].flags & Dead) || m_tableNodes.count(reader)) continue;
                if (from == kNone) roots.push_back(reader);
                else {
                    readersOfNode[from].push_back(reader);
                    precedentsOfNode[reader].push_back(from);
                }
                if (closure.count(reader)) continue;
                if (m_nodes[reader].flags & (Kept | Always)) unsafe = true;
                closure.insert(reader);
                frontier.push_back({areaOf(m_nodes[reader]), reader});
            }
        }
        if (!frontier.empty()) unsafe = true;   // (more than the work allows)
    }
    if (unsafe) return 0;
    // the order the formulas of the closure are worked out in: each after those it reads
    std::vector<quint32> order;
    {
        std::unordered_map<quint32, int> waiting;
        for (const quint32 id : closure) waiting[id] = int(precedentsOfNode[id].size());
        std::vector<quint32> ready;
        for (const quint32 id : closure)
            if (waiting[id] == 0) ready.push_back(id);
        while (!ready.empty()) {
            const quint32 id = ready.back();
            ready.pop_back();
            order.push_back(id);
            for (const quint32 reader : readersOfNode[id])
                if (--waiting[reader] == 0) ready.push_back(reader);
        }
        // (formulas in a circle are left at the end, and worked out as they are asked for)
        if (order.size() < closure.size()) {
            std::unordered_set<quint32> placed(order.begin(), order.end());
            for (const quint32 id : closure)
                if (!placed.count(id)) order.push_back(id);
        }
    }
    std::unordered_map<quint32, std::vector<quint32>> neededFor;   // by the formula of the table: the part of the order it needs
    auto orderFor = [&](quint32 formulaNode) -> const std::vector<quint32> & {
        auto have = neededFor.find(formulaNode);
        if (have != neededFor.end()) return have->second;
        std::unordered_set<quint32> needed;
        std::vector<quint32> stack{formulaNode};
        needed.insert(formulaNode);
        while (!stack.empty()) {
            const quint32 id = stack.back();
            stack.pop_back();
            for (const quint32 p : precedentsOfNode[id])
                if (needed.insert(p).second) stack.push_back(p);
        }
        std::vector<quint32> mine;
        for (const quint32 id : order)
            if (needed.count(id)) mine.push_back(id);
        return neededFor.emplace(formulaNode, std::move(mine)).first->second;
    };
    WhatIf source(*this, closure);
    qint64 changedCells = 0;
    // the formula of a cell of the table: at the corner for two inputs, above its column for a column input, at the left of its row for a row input
    for (quint32 row = range.row1; row <= range.row2; ++row) {
        for (quint32 col = range.col1; col <= range.col2; ++col) {
            std::vector<Input> inputs;
            if (hasRow) inputs.push_back({sheet, rr, rc, cell(sheet, range.row1 - 1, col)});
            if (hasCol) inputs.push_back({sheet, cr, cc, cell(sheet, row, range.col1 - 1)});
            quint32 fRow = range.row1 - 1, fCol = range.col1 - 1;
            if (hasRow && !hasCol) fRow = row;
            else if (hasCol && !hasRow) fCol = col;
            Value v;
            const quint32 formulaNode = nodeOfCell(sheet, fRow, fCol);
            source.setInputs(inputs);
            if (formulaNode == kNone) v = cell(sheet, fRow, fCol);
            else {
                // (each formula it needs, after those it reads)
                if (closure.count(formulaNode))
                    for (const quint32 need : orderFor(formulaNode))
                        if (!source.knows(need)) source.remember(need, source.evaluateNode(need));
                v = source.valueOf(formulaNode);
            }
            if (v.isArray()) v = v.at(0, 0);
            // the cell: the node of the first cell, or a plain cell of the sheet
            const bool isFirst = row == range.row1 && col == range.col1;
            const Value was = isFirst ? nodeValue(n, id) : cell(sheet, row, col);
            if (sameValue(was, v)) continue;
            ++changedCells;
            if (isFirst) {
                storeResult(n, id, v, true);
            } else if (Cell *c = s.cells.findMutable(row, col)) {
                m_tableOriginal.emplace(std::make_tuple(sheet, c->rowId, c->colId), valueOf(s, *c));   // (the first time only: what the file stored)
                switch (v.kind()) {
                case Value::Kind::Number: c->type = CellType::Number; c->number = v.number(); break;
                case Value::Kind::Bool: c->type = CellType::Bool; c->number = v.boolean() ? 1 : 0; break;
                case Value::Kind::Error: c->type = CellType::Error; c->number = double(int(v.errorKind())); break;
                case Value::Kind::Text:
                    s.texts.append(v.text());
                    c->type = CellType::InlineString;
                    c->number = double(s.texts.size() - 1);
                    break;
                default: c->type = CellType::Empty; c->number = 0; break;
                }
                if (m_sink) {
                    m_valuesTold.fetch_add(1, std::memory_order_relaxed);
                    m_sink(sheet, c->rowId, c->colId, v);
                }
            }
            CellArea a;
            a.sheet1 = a.sheet2 = sheet;
            a.row1 = a.row2 = row;
            a.col1 = a.col2 = col;
            changed->push_back(a);
        }
    }
    return changedCells;
}

bool Calculation::dataTablePass()
{
    m_tablesStale = false;
    m_tablesForce = false;
    std::vector<CellArea> changedPlaces;
    for (const quint32 id : std::vector<quint32>(m_tableNodes.begin(), m_tableNodes.end())) {
        if (id >= m_nodes.size() || (m_nodes[id].flags & Dead) || dormant(m_nodes[id])) continue;
        m_tableCellsChanged += workOutTable(id, &changedPlaces);
    }
    if (changedPlaces.empty()) return false;
    // what reads the cells that changed is worked out after (the areas are told by the ids of their lines)
    std::vector<CellArea> ids;
    for (const CellArea &a : changedPlaces) idAreasOn(a, false, &ids);
    markDependentsOfIds(&ids);
    return true;
}

bool Calculation::dataTableOriginal(int sheet, quint32 rowId, quint32 colId, Value *out) const
{
    const auto it = m_tableOriginal.find(std::make_tuple(sheet, rowId, colId));
    if (it == m_tableOriginal.end()) return false;
    *out = it->second;
    return true;
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
