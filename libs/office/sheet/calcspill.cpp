// Spills: a formula typed in a cell whose result is a block fills the cells below and beside it. The formula's cell holds the formula and is worked out as an array (Cell::SpillFormula); the cells it
// fills are marked as members of it (ArrayMember with SpillMember) and hold no value of their own, only what the formula gave them. The block is made after each working out of the formula: it is as
// large as the result, and every cell of it, but the formula's own, must be empty or already filled by this formula. A cell in the way holds something else: the formula's cell has #SPILL! and nothing
// is filled. A block larger than the sheet, or than kMaxSpillCells, is the same error, so no formula makes a hang or a huge allocation by its size. What read the cells the block gained or lost is worked
// out again; what the formula reads is told to the graph with the block it asked for, so that clearing what was in the way works the formula out again. The spill range operator A1# is worked
// out here too (spillRange). A cell a person types in, that is a member, is written over: the formula's cell reads it as in the way, and is worked out again.
#include "sheet/calculation.h"

#include "calc/functions.h"

#include <QCoreApplication>

#include <algorithm>
#include <cstring>

namespace jp::office::sheet {

using jp::calc::CellArea;
using jp::calc::kGridRows;
using jp::calc::Value;

namespace {

// The functions whose result is a block: a formula that calls one is worked out as an array that spills.
bool givesBlock(const char *name)
{
    static const char *const names[] = {"FILTER", "SORT", "SORTBY", "UNIQUE", "SEQUENCE", "RANDARRAY", "TRANSPOSE", "XLOOKUP", "LET", "MMULT"};
    for (const char *n : names)
        if (std::strcmp(name, n) == 0) return true;
    return false;
}

} // namespace

bool Calculation::formulaCanSpill(const QString &formula)
{
    using namespace jp::calc;
    const ParseResult parsed = parseFormula(formula);
    if (!parsed.ok()) return false;
    const Program &p = *parsed.program;
    // (a block that an operator takes is arithmetic on a range: A1:A3*2; the stack says which values are blocks, as far as the instructions go in a line)
    std::vector<char> st;
    auto pop = [&]() {
        if (st.empty()) return char(0);
        const char top = st.back();
        st.pop_back();
        return top;
    };
    for (const Instr &in : p.code) {
        switch (in.op) {
        case Op::Spill:
        case Op::PushArray: return true;
        case Op::PushRef: {
            bool block = false;
            if (in.operand >= 0 && size_t(in.operand) < p.refs.size()) {
                const RefLit &r = p.refs[size_t(in.operand)];
                block = r.row1 != r.row2 || r.col1 != r.col2 || (r.flags & (kRefWholeCols | kRefWholeRows));
            }
            st.push_back(block ? 1 : 0);
            break;
        }
        case Op::Add: case Op::Sub: case Op::Mul: case Op::Div: case Op::Pow: case Op::Concat: case Op::Eq: case Op::Ne: case Op::Lt: case Op::Le: case Op::Gt: case Op::Ge: {
            const char b = pop(), a = pop();
            if (a || b) return true;
            st.push_back(0);
            break;
        }
        case Op::Neg:
        case Op::Percent:
            if (pop()) return true;
            st.push_back(0);
            break;
        case Op::Call:
            if (in.operand >= 0 && givesBlock(functionAt(in.operand).name)) return true;
            for (int k = 0; k < int(in.argc); ++k) pop();
            st.push_back(0);
            break;
        case Op::Bind:
        case Op::IfJump:
        case Op::TryJump:
        case Op::Choose: pop(); break;
        case Op::Jump: break;
        case Op::PushNumber: case Op::PushText: case Op::PushBool: case Op::PushError: case Op::PushMissing: case Op::PushName: case Op::PushTable: case Op::PushExternal:
        case Op::PushLocal: st.push_back(0); break;
        default: break;
        }
    }
    return false;
}

CellRange Calculation::spillBlock(int sheet, quint32 row, quint32 col) const
{
    CellRange out;
    if (sheet < 0 || sheet >= int(m_sheets.size())) return out;
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c || !c->hasFormula() || !(c->flags & SpillFormula) || c->formula < 0 || c->formula >= s.formulas.size()) return out;
    out.row1 = out.row2 = row;
    out.col1 = out.col2 = col;
    out.valid = true;
    const qint32 slot = s.formulas[c->formula].range;
    if (slot >= 0 && slot < s.ranges.size() && s.ranges[slot].valid) out = s.ranges[slot];
    return out;
}

bool Calculation::isSpillAnchor(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return false;
    const Cell *c = m_sheets[size_t(sheet)].cells.find(row, col);
    return c && c->hasFormula() && (c->flags & SpillFormula);
}

bool Calculation::isSpillMember(int sheet, quint32 row, quint32 col, CellRange *anchor) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return false;
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c || !(c->flags & SpillMember) || c->formula < 0) return false;
    if (anchor) {
        *anchor = CellRange();
        const SheetState &st = m_state[size_t(sheet)];
        const quint32 id = size_t(c->formula) < st.nodeOf.size() ? st.nodeOf[size_t(c->formula)] : kNone;
        if (id != kNone) {
            const quint32 r = rowOf(m_nodes[id]), cc = colOf(m_nodes[id]);
            *anchor = spillBlock(sheet, r, cc);
        }
    }
    return true;
}

jp::calc::CellSource::SpillAnswer Calculation::spillRange(int sheet, quint32 row, quint32 col, CellArea *area) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return SpillAnswer::NotSpill;
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c || !c->hasFormula() || !(c->flags & SpillFormula) || c->formula < 0) return SpillAnswer::NotSpill;
    const SheetState &st = m_state[size_t(sheet)];
    const quint32 id = size_t(c->formula) < st.nodeOf.size() ? st.nodeOf[size_t(c->formula)] : kNone;
    if (id == kNone) return SpillAnswer::NotSpill;
    const Node &n = m_nodes[id];
    if (m_evaluating && n.state != Done) {
        m_pending.push_back(id);
        return SpillAnswer::Pending;
    }
    if (n.flags & NodeTainted) m_readTaint = true;
    if (n.flags & (Mismatch | Cascade)) m_readCascade = true;
    if (n.kind == KindError) return SpillAnswer::NotSpill;   // (blocked, or an error: nothing is filled)
    *area = areaOf(n);
    return SpillAnswer::Area;
}

// Whether the cell is held by something that is not this formula's spill.
static bool heldByOther(const Cell &x, qint32 formula)
{
    if (x.flags & SpillMember) return x.formula != formula;
    if (x.flags & (HasFormula | ArrayMember | ArrayFormula | DataTable | SpillFormula)) return true;
    return x.type != CellType::Empty;
}

void Calculation::tellGone(int sheet, quint32 rowId, quint32 colId) const
{
    if (!m_sink || rowId == CellStore::kNoLine || colId == CellStore::kNoLine) return;
    m_valuesTold.fetch_add(1, std::memory_order_relaxed);
    m_sink(sheet, rowId, colId, Value());
}

// The cells of the block of `formula` (a spilling formula's place in the sheet's list) that it fills, let go of: they hold nothing now. `keep`, if valid, is the block that stays filled.
static std::vector<std::pair<quint32, quint32>> membersOutside(const CellStore &cells, const CellRange &range, const CellRange &keep, qint32 formula)
{
    std::vector<std::pair<quint32, quint32>> out;
    if (!range.valid || range.cellCount() <= 1) return out;
    cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](quint32 row, quint32 col, const Cell &c) {
        if ((c.flags & SpillMember) && c.formula == formula && !keep.contains(row, col)) out.emplace_back(row, col);
        return true;
    });
    return out;
}

void Calculation::letGoOfSpill(int sheet, qint32 formula)
{
    Sheet &s = m_sheets[size_t(sheet)];
    if (formula < 0 || formula >= s.formulas.size()) return;
    const qint32 slot = s.formulas[formula].range;
    if (slot < 0 || slot >= s.ranges.size()) return;
    const CellRange range = s.ranges[slot];
    const auto gone = membersOutside(s.cells, range, CellRange(), formula);
    for (const auto &[row, col] : gone) {
        const quint32 rowId = s.cells.rowIdAt(row), colId = s.cells.colIdAt(col);
        Cell *c = s.cells.findMutable(row, col);
        if (!c) continue;
        c->flags &= quint8(~(ArrayMember | SpillMember));
        c->formula = -1;
        c->type = CellType::Empty;
        c->number = 0;
        if (c->style == 0) s.cells.erase(row, col);
        tellGone(sheet, rowId, colId);
    }
    s.ranges[slot].valid = false;
    s.formulas[formula].range = -1;
    if (range.valid && range.cellCount() > 1) {
        CellArea a;
        a.sheet1 = a.sheet2 = sheet;
        a.row1 = range.row1;
        a.col1 = range.col1;
        a.row2 = range.row2;
        a.col2 = range.col2;
        queueDependents(a);
    }
}

Value Calculation::applySpill(Node &n, quint32 id, const Value &v)
{
    const Cell *head = cellOf(n);
    if (!head || !(head->flags & SpillFormula)) return v;
    const int sheet = n.sheet;
    Sheet &s = m_sheets[size_t(sheet)];
    const qint32 formula = n.formula;
    const quint32 row0 = rowOf(n), col0 = colOf(n);
    Value out = v;
    qint64 rows = 1, cols = 1;
    if (v.isArray()) {
        rows = v.rows();
        cols = v.cols();
    }
    bool wanted = true;   // the block asked for is the one told to the graph (too large a one is not)
    std::vector<CellArea> told;   // what the formula reads, besides its references: what it touched, and the block it asked for
    bool toldSet = false;
    if (rows * cols > kMaxSpillCells || qint64(row0) + rows > qint64(kGridRows) || qint64(col0) + cols > qint64(kMaxColumns)) {
        out = Value::error(jp::calc::ErrorKind::Spill);
        rows = cols = 1;
        wanted = false;
    } else if (rows * cols > 1) {
        bool blocked = false;
        s.cells.forEachInRange(row0, col0, row0 + quint32(rows) - 1, col0 + quint32(cols) - 1, [&](quint32 row, quint32 col, const Cell &c) {
            if (row == row0 && col == col0) return true;
            if (!heldByOther(c, formula)) return true;
            blocked = true;
            return false;
        });
        if (blocked) {
            out = Value::error(jp::calc::ErrorKind::Spill);
            // (the block that was asked for is what the formula reads, so that what is in the way being cleared works it out again)
            if (m_nodes[id].flags & Told) told = m_touched;
            CellArea a;
            a.sheet1 = a.sheet2 = sheet;
            a.row1 = row0;
            a.col1 = col0;
            a.row2 = row0 + quint32(rows) - 1;
            a.col2 = col0 + quint32(cols) - 1;
            told.push_back(a);
            toldSet = true;
            rows = cols = 1;
            wanted = false;
        }
    }
    if (out.isArray() && rows * cols == 1) out = out.at(0, 0);
    // the block the formula fills now, against the one it filled
    CellRange now;
    now.valid = true;
    now.row1 = row0;
    now.col1 = col0;
    now.row2 = row0 + quint32(rows) - 1;
    now.col2 = col0 + quint32(cols) - 1;
    qint32 slot = s.formulas[formula].range;
    CellRange was;
    was.valid = true;
    was.row1 = was.row2 = row0;
    was.col1 = was.col2 = col0;
    if (slot >= 0 && slot < s.ranges.size() && s.ranges[slot].valid) was = s.ranges[slot];
    if (!(was == now)) {
        const auto gone = membersOutside(s.cells, was, now, formula);
        std::vector<CellArea> changed;
        for (const auto &[row, col] : gone) {
            const quint32 rowId = s.cells.rowIdAt(row), colId = s.cells.colIdAt(col);
            Cell *c = s.cells.findMutable(row, col);
            if (!c) continue;
            c->flags &= quint8(~(ArrayMember | SpillMember));
            c->formula = -1;
            c->type = CellType::Empty;
            c->number = 0;
            if (c->style == 0) s.cells.erase(row, col);
            tellGone(sheet, rowId, colId);
        }
        if (now.cellCount() > 1) {
            for (quint32 row = now.row1; row <= now.row2; ++row)
                for (quint32 col = now.col1; col <= now.col2; ++col) {
                    if (row == row0 && col == col0) continue;
                    const Cell *have = s.cells.find(row, col);
                    if (have && (have->flags & SpillMember) && have->formula == formula) continue;
                    Cell member = have ? *have : Cell();
                    member.type = CellType::Empty;
                    member.number = 0;
                    member.flags |= ArrayMember | SpillMember;
                    member.formula = formula;
                    s.cells.set(row, col, member);
                }
            useExtent(sheet, now.row2, now.col2);
            s.extentRows = std::max(s.extentRows, now.row2 + 1);
            s.extentCols = std::max<quint32>(s.extentCols, now.col2 + 1);
        }
        if (slot >= 0 && slot < s.ranges.size()) {
            s.ranges[slot] = now;
        } else {
            slot = qint32(s.ranges.size());
            s.ranges.append(now);
            s.formulas[formula].range = slot;
        }
        // what read a cell the block gained or lost is worked out again (the formula's own cell is not among them: it is being worked out)
        for (const CellRange &r : {was, now}) {
            if (r.cellCount() <= 1) continue;
            CellArea a;
            a.sheet1 = a.sheet2 = sheet;
            a.row1 = r.row1;
            a.col1 = r.col1;
            a.row2 = r.row2;
            a.col2 = r.col2;
            changed.push_back(a);
        }
        if (!changed.empty()) markDependents(&changed);
    }
    // what it reads: its references, what it touched, and the block it asked for
    if (!toldSet) {
        if (m_nodes[id].flags & Told) told = m_touched;
        if (wanted && now.cellCount() > 1) {
            CellArea a;
            a.sheet1 = a.sheet2 = sheet;
            a.row1 = now.row1;
            a.col1 = now.col1;
            a.row2 = now.row2;
            a.col2 = now.col2;
            told.push_back(a);
        }
    }
    registerNode(id, &told);
    return out;
}

} // namespace jp::office::sheet
