// Spills: a formula typed in a cell whose result is a block fills the cells below and beside it. The formula's cell holds the formula and is worked out as an array (Cell::SpillFormula); the cells it
// fills are marked as members of it (ArrayMember with SpillMember) and hold no value of their own, only what the formula gave them. The block is made after each working out of the formula: it is as
// large as the result, and every cell of it, but the formula's own, must be empty or already filled by this formula. A cell in the way holds something else: the formula's cell has #SPILL! and nothing
// is filled. A block larger than the sheet, or than kMaxSpillCells, is the same error, so no formula makes a hang or a huge allocation by its size. What read the cells the block gained or lost is worked
// out again; what the formula reads is told to the graph with the block it asked for, so that clearing what was in the way works the formula out again. The spill range operator A1# is worked
// out here too (spillRange). A cell a person types in, that is a member, is written over: the formula's cell reads it as in the way, and is worked out again.
#include "sheet/calculation.h"

#include "calc/functions.h"

#include <QCoreApplication>
#include <QRegularExpression>

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
    for (size_t at = 0; at < p.code.size(); ++at) {
        const Instr &in = p.code[at];
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
        case Op::Single:   // (@ takes one value of a block: the old rule)
            pop();
            st.push_back(0);
            break;
        case Op::Call:
            if (in.operand >= 0 && givesBlock(functionAt(in.operand).name) && !(at + 1 < p.code.size() && p.code[at + 1].op == Op::Single)) return true;
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

// The formula of a cell that is not a dynamic array, with the mark @ where the old rule (one value of a block) is what it works by: before a range that an operator takes or a function takes as a value, and before a
// function that gives a block, in a formula that would give a block if it were a dynamic array. The cell holds the formula as the file wrote it: this is how it is shown.
QString Calculation::withImplicitMarks(const QString &formula)
{
    if (formula.contains(QLatin1Char('@')) || !formulaCanSpill(formula)) return formula;
    struct Tok {
        enum Kind : quint8 { Word, Call, Str, Op, Open, Close, Comma, Other } kind;
        int start, end;
        bool range;
        const jp::calc::FunctionInfo *fn;
        int match;   // the index of the token that closes a call's parenthesis
    };
    std::vector<Tok> toks;
    const int n = formula.size();
    static const QRegularExpression rangeText(QStringLiteral("^(\\$?[A-Za-z]{1,3}\\$?[0-9]+:\\$?[A-Za-z]{1,3}\\$?[0-9]+|\\$?[A-Za-z]{1,3}:\\$?[A-Za-z]{1,3}|\\$?[0-9]+:\\$?[0-9]+)$"));
    for (int i = 0; i < n;) {
        const QChar c = formula.at(i);
        if (c.isSpace()) {
            ++i;
            continue;
        }
        const int start = i;
        if (c == QLatin1Char('"')) {
            for (++i; i < n; ++i)
                if (formula.at(i) == QLatin1Char('"')) {
                    if (i + 1 < n && formula.at(i + 1) == QLatin1Char('"')) ++i;
                    else break;
                }
            i = std::min(n, i + 1);
            toks.push_back({Tok::Str, start, i, false, nullptr, -1});
        } else if (c == QLatin1Char('(')) {
            ++i;
            toks.push_back({Tok::Open, start, i, false, nullptr, -1});
        } else if (c == QLatin1Char(')')) {
            ++i;
            toks.push_back({Tok::Close, start, i, false, nullptr, -1});
        } else if (c == QLatin1Char(',')) {
            ++i;
            toks.push_back({Tok::Comma, start, i, false, nullptr, -1});
        } else if (c == QLatin1Char('{')) {
            while (i < n && formula.at(i) != QLatin1Char('}')) ++i;
            i = std::min(n, i + 1);
            toks.push_back({Tok::Other, start, i, false, nullptr, -1});
        } else if (QStringLiteral("+-*/^&=<>%").contains(c)) {
            ++i;
            toks.push_back({Tok::Op, start, i, false, nullptr, -1});
        } else if (c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('$') || c == QLatin1Char('\'') || c == QLatin1Char('.') || c == QLatin1Char('[')) {
            while (i < n) {
                const QChar d = formula.at(i);
                if (d == QLatin1Char('\'')) {
                    for (++i; i < n; ++i)
                        if (formula.at(i) == QLatin1Char('\'')) {
                            if (i + 1 < n && formula.at(i + 1) == QLatin1Char('\'')) ++i;
                            else break;
                        }
                    i = std::min(n, i + 1);
                } else if (d == QLatin1Char('[')) {
                    int depth = 0;
                    for (; i < n; ++i) {
                        if (formula.at(i) == QLatin1Char('[')) ++depth;
                        else if (formula.at(i) == QLatin1Char(']') && --depth == 0) {
                            ++i;
                            break;
                        }
                    }
                } else if (d.isLetterOrNumber() || d == QLatin1Char('_') || d == QLatin1Char('$') || d == QLatin1Char('.') || d == QLatin1Char('!') || d == QLatin1Char(':') || d == QLatin1Char('#')) {
                    ++i;
                } else {
                    break;
                }
            }
            if (i == start) ++i;
            int look = i;
            while (look < n && formula.at(look).isSpace()) ++look;
            const QString word = formula.mid(start, i - start);
            if (look < n && formula.at(look) == QLatin1Char('(')) {
                QString name = word.toUpper();
                for (const char *prefix : {"_XLFN.", "_XLWS."})
                    if (name.startsWith(QLatin1String(prefix))) name = name.mid(int(std::strlen(prefix)));
                toks.push_back({Tok::Call, start, i, false, jp::calc::findFunction(name), -1});
            } else {
                const QString part = word.mid(word.lastIndexOf(QLatin1Char('!')) + 1);
                toks.push_back({Tok::Word, start, i, rangeText.match(part).hasMatch(), nullptr, -1});
            }
        } else {
            ++i;
            toks.push_back({Tok::Other, start, i, false, nullptr, -1});
        }
    }
    // the parentheses: what closes each call's, and what each token is an argument of
    struct Frame {
        int call;   // the token of the function (-1 for a plain parenthesis)
        int arg;
    };
    std::vector<Frame> stack;
    std::vector<int> frameCall(toks.size(), -1), frameArg(toks.size(), 0);   // (-1: a plain parenthesis, or none)
    for (size_t k = 0; k < toks.size(); ++k) {
        const Tok &t = toks[k];
        if (!stack.empty()) {
            frameCall[k] = stack.back().call;
            frameArg[k] = stack.back().arg;
        }
        if (t.kind == Tok::Open) {
            const bool call = k > 0 && toks[k - 1].kind == Tok::Call;
            stack.push_back({call ? int(k - 1) : -1, 0});
        } else if (t.kind == Tok::Close) {
            if (stack.empty()) return formula;
            if (stack.back().call >= 0) toks[size_t(stack.back().call)].match = int(k);
            stack.pop_back();
        } else if (t.kind == Tok::Comma && !stack.empty()) {
            ++stack.back().arg;
        }
    }
    if (!stack.empty()) return formula;
    QVector<int> marks;
    for (size_t k = 0; k < toks.size(); ++k) {
        const Tok &t = toks[k];
        bool candidate = false;
        size_t last = k;
        if (t.kind == Tok::Word) {
            candidate = t.range;
        } else if (t.kind == Tok::Call && t.fn && givesBlock(t.fn->name) && t.match > 0) {
            candidate = true;
            last = size_t(t.match);
        }
        if (!candidate) continue;
        const bool before = k > 0 && toks[k - 1].kind == Tok::Op;
        const bool after = last + 1 < toks.size() && toks[last + 1].kind == Tok::Op;
        bool mark = before || after;
        const int call = frameCall[k];
        const jp::calc::FunctionInfo *f = call >= 0 ? toks[size_t(call)].fn : nullptr;
        const char cls = f ? jp::calc::paramClass(*f, frameArg[k]) : 0;
        if (cls == 'a') mark = false;   // (an argument worked out as an array: SUMPRODUCT, FILTER's)
        else if (!mark) mark = call < 0 || cls == 'v';
        if (mark) marks.push_back(t.start);
    }
    QString out = formula;
    for (int k = marks.size() - 1; k >= 0; --k) out.insert(marks[k], QLatin1Char('@'));
    return out;
}

CellRange Calculation::rangeOfFormula(const Sheet &s, qint32 formula) const
{
    // (the worker appends to the list and writes a formula's slot as a formula spills, under this lock; a reader on another thread takes it, and so does one that has the worker held back, which needs none but is
    // not told apart cheaply. The worker's own reads need none: it is the only writer.)
    std::unique_lock<std::mutex> lock(m_rangesLock, std::defer_lock);
    if (m_guard.workerServes() && !m_guard.isWorkerOrNone()) {
        lock.lock();
        m_rangesLockTakes.fetch_add(1, std::memory_order_relaxed);
    }
    if (formula < 0 || size_t(formula) >= size_t(s.formulas.size())) return CellRange();
    const qint32 slot = s.formulas[formula].range;
    if (slot >= 0 && slot < s.ranges.size() && s.ranges[slot].valid) return s.ranges[slot];
    return CellRange();
}

CellRange Calculation::spillBlock(int sheet, quint32 row, quint32 col) const
{
    CellRange out;
    if (sheet < 0 || sheet >= int(m_sheets.size())) return out;
    const Sheet &s = m_sheets[size_t(sheet)];
    const std::optional<Cell> c = s.cells.find(row, col);
    if (!c || !c->hasFormula() || !(c->flags & SpillFormula) || c->formula < 0 || c->formula >= s.formulas.size()) return out;
    out.row1 = out.row2 = row;
    out.col1 = out.col2 = col;
    out.valid = true;
    // (a thread that is neither the worker nor has it held back reads the blocks it was given, as one with the values: the sheet's ranges are the worker's, and it changes them as it works)
    if (s.cells.spills() && !m_guard.isWorkerOrNone() && !m_guard.heldHere()) {
        if (const SpillView v = s.cells.spillView())
            if (const SpillRect *r = v.table->ofFormula(c->formula)) {
                out.row1 = r->row1;
                out.col1 = r->col1;
                out.row2 = r->row2;
                out.col2 = r->col2;
            }
        return out;
    }
    if (const CellRange r = rangeOfFormula(s, c->formula); r.valid) out = r;
    return out;
}

bool Calculation::isSpillAnchor(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return false;
    const std::optional<Cell> c = m_sheets[size_t(sheet)].cells.find(row, col);
    return c && c->hasFormula() && (c->flags & SpillFormula);
}

bool Calculation::isSpillMember(int sheet, quint32 row, quint32 col, CellRange *anchor) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return false;
    const Sheet &s = m_sheets[size_t(sheet)];
    const std::optional<Cell> c = s.cells.find(row, col);
    if (!c || !(c->flags & SpillMember) || c->formula < 0) return false;
    if (anchor) {
        *anchor = CellRange();
        if (s.cells.spills() && !m_guard.isWorkerOrNone() && !m_guard.heldHere()) {
            // (a thread that is neither the worker nor has it held back reads the block it was given, not the node, which is the worker's)
            if (const SpillView v = s.cells.spillView())
                if (const SpillRect *r = v.table->at(row, col)) *anchor = spillBlock(sheet, r->row1, r->col1);
            return true;
        }
        // (the block's first cell is the formula's own: its range names it, so no node is looked at, which is the worker's)
        if (const CellRange r = rangeOfFormula(s, c->formula); r.valid) *anchor = spillBlock(sheet, r.row1, r.col1);
    }
    return true;
}

jp::calc::CellSource::SpillAnswer Calculation::spillRange(int sheet, quint32 row, quint32 col, CellArea *area) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return SpillAnswer::NotSpill;
    const Sheet &s = m_sheets[size_t(sheet)];
    const std::optional<Cell> c = s.cells.find(row, col);
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

// ---- the spill index (sheet/spillindex.h): the blocks table of a sheet, kept only when the index is on (a store has no table when it is off, and then none of this is called)

// Whether the block is one a spill fills: a formula that spills and is worked out (not one kept apart: its cells are the file's), of more than one cell and no more than a spill can have.
bool Calculation::spillRegisterable(int sheet, qint32 formula, const CellRange &range) const
{
    const Sheet &s = m_sheets[size_t(sheet)];
    if (!range.valid || range.cellCount() <= 1 || range.cellCount() > kMaxSpillCells) return false;
    if (formula < 0 || formula >= s.formulas.size()) return false;
    const SheetState &st = m_state[size_t(sheet)];
    if (size_t(formula) < st.nodeOf.size() && st.nodeOf[size_t(formula)] != kNone && (m_nodes[st.nodeOf[size_t(formula)]].flags & Kept)) return false;
    const std::optional<Cell> head = s.cells.find(range.row1, range.col1);
    return head && (head->flags & SpillFormula) && head->formula == formula;
}

// Puts the block of a formula in the sheet's table, as the thread that has the table does: the worker, or one that has the worker held back (and then the others are given the copy at once). The count of
// the store's writes moves, for what holds the cells by that (a lookup cache).
void Calculation::putSpillBlock(int sheet, const SpillRect &r)
{
    Sheet &s = m_sheets[size_t(sheet)];
    SpillPublisher *pub = s.cells.spills();
    if (!pub) return;
    if (!pub->mine().set(r)) pub->mine().erase(r.formula);
    pub->noteChanged();
    s.cells.noteSpillChange();
    if (!m_guard.isWorkerOrNone() && !m_deferPublish) pub->publish();
}

void Calculation::dropSpillBlock(int sheet, qint32 formula)
{
    Sheet &s = m_sheets[size_t(sheet)];
    SpillPublisher *pub = s.cells.spills();
    if (!pub || !pub->mine().ofFormula(formula)) return;
    pub->mine().erase(formula);
    pub->noteChanged();
    s.cells.noteSpillChange();
    if (!m_guard.isWorkerOrNone() && !m_deferPublish) pub->publish();
}

// The block of a loaded or edited spill, registered if it is one (see spillRegisterable) and meets no other block: the first of two that meet is the one that stands, the other is blocked when it is worked out.
bool Calculation::registerSpill(int sheet, qint32 formula, const CellRange &range)
{
    SpillPublisher *pub = m_sheets[size_t(sheet)].cells.spills();
    if (!pub || !spillRegisterable(sheet, formula, range)) return false;
    if (pub->mine().meetsOther(range.row1, range.col1, range.row2, range.col2, formula)) return false;
    putSpillBlock(sheet, SpillRect{range.row1, range.col1, range.row2, range.col2, formula});
    if (!pub->mine().ofFormula(formula)) return false;   // (the table is full: no block, and the cells the file stored are what they were)
    useExtent(sheet, range.row2, range.col2);
    return true;
}

// The table made again from the sheet's formulas, by the same rule and in the same order as a load: one block for the range each formula names (not each range of the sheet's list, which may hold one no formula names,
// or the same twice), the formulas in the order of their first cells.
void Calculation::rebuildSpillTable(int sheet)
{
    Sheet &s = m_sheets[size_t(sheet)];
    SpillPublisher *pub = s.cells.spills();
    if (!pub) return;
    pub->mine().clear();
    pub->noteChanged();
    s.cells.noteSpillChange();
    struct Anchor {
        quint32 row, col;
        qint32 formula;
    };
    std::vector<Anchor> anchors;
    for (qint32 f = 0; f < s.formulas.size(); ++f) {
        const qint32 slot = s.formulas[f].range;
        if (slot < 0 || slot >= s.ranges.size()) continue;
        const CellRange &r = s.ranges[slot];
        if (!r.valid || r.cellCount() <= 1) continue;
        anchors.push_back({r.row1, r.col1, f});
    }
    std::sort(anchors.begin(), anchors.end(), [](const Anchor &a, const Anchor &b) { return a.row != b.row ? a.row < b.row : a.col != b.col ? a.col < b.col : a.formula < b.formula; });
    ++m_deferPublish;   // (one copy for the others when the table is whole, not one for each block put: that copies the table as many times as there are blocks)
    for (const Anchor &a : anchors) registerSpill(sheet, a.formula, s.ranges[s.formulas[a.formula].range]);
    --m_deferPublish;
    if (!m_guard.isWorkerOrNone()) pub->publish();
}

// The window is told that the cells of the block `was` that the block `keep` (if valid) does not hold, but for the formula's own cell, have no value now. Only the cells of the block the table holds for the
// formula are looked at (at most what a spill can have), whatever the range says: a formula that holds no block (a stored range over the size a spill can have, or one of a legacy array) has told nothing.
void Calculation::tellGoneBlock(int sheet, qint32 formula, const CellRange &was, const CellRange &keep, quint32 row0, quint32 col0) const
{
    if (!m_sink || !was.valid || was.cellCount() <= 1) return;
    const Sheet &s = m_sheets[size_t(sheet)];
    const SpillPublisher *pub = s.cells.spills();
    const SpillRect *held = pub ? pub->mine().ofFormula(formula) : nullptr;
    if (!held) return;
    const quint32 row1 = std::max(was.row1, held->row1), row2 = std::min(was.row2, held->row2), col1 = std::max(was.col1, held->col1), col2 = std::min(was.col2, held->col2);
    if (row1 > row2 || col1 > col2) return;
    const CellStore &cells = s.cells;
    std::vector<quint32> colIds;
    for (quint32 col = col1; col <= col2; ++col) colIds.push_back(cells.colIdAt(col));
    for (quint32 row = row1; row <= row2; ++row) {
        const quint32 rowId = cells.rowIdAt(row);
        for (quint32 col = col1; col <= col2; ++col) {
            m_goneWalked.fetch_add(1, std::memory_order_relaxed);
            if ((row == row0 && col == col0) || keep.contains(row, col)) continue;
            tellGone(sheet, rowId, colIds[col - col1]);
        }
    }
}

// The cells of the block of `formula` (a spilling formula's place in the sheet's list) that it fills, let go of: they hold nothing now. `keep`, if valid, is the block that stays filled.
static std::vector<std::pair<quint32, quint32>> membersOutside(const CellStore &cells, const CellRange &range, const CellRange &keep, qint32 formula)
{
    std::vector<std::pair<quint32, quint32>> out;
    if (!range.valid || range.cellCount() <= 1) return out;
    cells.forEachInRangeReal(range.row1, range.col1, range.row2, range.col2, [&](quint32 row, quint32 col, const Cell &c) {
        if ((c.flags & SpillMember) && c.formula == formula && !keep.contains(row, col)) out.emplace_back(row, col);
        return true;
    });
    return out;
}

// The cells of a loaded spill that its block let go of, taken from the store by a thread that has the worker held back (or by the one that serves when no worker does).
void Calculation::releaseStrays()
{
    if (m_strays.empty()) return;
    const std::vector<Stray> list = std::move(m_strays);
    m_strays.clear();
    m_strayPlaces.clear();
    m_strayCount.store(0, std::memory_order_release);
    for (const Stray &st : list) {
        if (st.sheet < 0 || st.sheet >= int(m_sheets.size())) continue;
        Sheet &s = m_sheets[size_t(st.sheet)];
        const quint32 row = s.cells.rowPlaceOf(st.rowId), col = s.cells.colPlaceOf(st.colId);
        if (row == CellStore::kNoLine || col == CellStore::kNoLine) continue;
        if (st.kind != 0) {
            Cell *p = s.cells.findMutable(row, col);
            if (!p || p->type != st.was.type || p->number != st.was.number || p->flags != st.was.flags || p->formula != st.was.formula) continue;   // (it is not what it was: someone's edit)
            if (st.kind == 1) {
                p->flags |= ArrayMember | SpillMember;
                p->formula = st.formula;
            } else {
                p->flags &= quint8(~(ArrayMember | SpillMember));
                p->formula = -1;
                p->type = CellType::Empty;
                p->number = 0;
                if (p->style == 0) s.cells.erase(row, col);
                ++m_straysReleased;
            }
            continue;
        }
        const SpillPublisher *pub = s.cells.spills();
        const SpillRect *held = pub && !st.formulaGoing ? pub->mine().ofFormula(st.formula) : nullptr;
        if (held && held->contains(row, col)) continue;   // (the formula's block holds it again)
        Cell *c = s.cells.findMutable(row, col);
        if (!c || !(c->flags & SpillMember) || c->formula != st.formula) continue;
        c->flags &= quint8(~(ArrayMember | SpillMember));
        c->formula = -1;
        c->type = CellType::Empty;
        c->number = 0;
        if (c->style == 0) s.cells.erase(row, col);
        ++m_straysReleased;
    }
}

std::vector<std::array<quint32, 4>> Calculation::resizedSpills() const
{
    std::vector<std::array<quint32, 4>> out;
    out.reserve(m_resizedSpills.size());
    for (const auto &[key, seq] : m_resizedSpills) out.push_back({key[0], key[1], key[2], seq});
    return out;
}

void Calculation::forgetResizedSpills(const std::vector<std::array<quint32, 4>> &written)
{
    m_guard.check("Calculation::forgetResizedSpills");
    for (const std::array<quint32, 4> &w : written) {
        const auto it = m_resizedSpills.find({w[0], w[1], w[2]});
        if (it != m_resizedSpills.end() && it->second == w[3]) m_resizedSpills.erase(it);
    }
}

void Calculation::letGoOfSpill(int sheet, qint32 formula)
{
    Sheet &s = m_sheets[size_t(sheet)];
    if (formula < 0 || formula >= s.formulas.size()) return;
    const qint32 slot = s.formulas[formula].range;
    if (slot < 0 || slot >= s.ranges.size()) return;
    const CellRange range = s.ranges[slot];
    if (s.cells.spills()) {
        tellGoneBlock(sheet, formula, range, CellRange(), range.row1, range.col1);   // (the cells the table gave are gone: the window is told, as it is of those a spill took cells of)
        dropSpillBlock(sheet, formula);
    }
    const auto gone = membersOutside(s.cells, range, CellRange(), formula);
    // (with the index and a worker that serves, the window reads the store beside this thread: the cells are told empty and taken by a thread that has the worker held back, as applySpill's are)
    const bool defer = !gone.empty() && s.cells.spills() && m_guard.workerServes() && !m_guard.heldHere();   // (a thread that has the worker held back takes them itself)
    for (const auto &[row, col] : gone) {
        const quint32 rowId = s.cells.rowIdAt(row), colId = s.cells.colIdAt(col);
        if (defer) {
            Stray st{sheet, rowId, colId, formula};
            st.formulaGoing = true;
            m_strays.push_back(st);
            m_strayPlaces.insert(StrayPlace{sheet, rowId, colId});
            m_strayCount.store(qint64(m_strays.size()), std::memory_order_release);
            m_letGoDeferred.fetch_add(1, std::memory_order_relaxed);
            tellGone(sheet, rowId, colId);
            continue;
        }
        Cell *c = s.cells.findMutable(row, col);
        if (!c) continue;
        c->flags &= quint8(~(ArrayMember | SpillMember));
        c->formula = -1;
        c->type = CellType::Empty;
        c->number = 0;
        if (c->style == 0) s.cells.erase(row, col);
        tellGone(sheet, rowId, colId);
    }
    {
        const std::lock_guard<std::mutex> lock(m_rangesLock);   // (the window reads the list and the slot: see rangeOfFormula)
        s.ranges[slot].valid = false;
        s.formulas[formula].range = -1;
    }
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
    const std::optional<Cell> head = cellOf(n);
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
    // (a file saved by the releases that kept a spill's old range over a grown block has the cells the block grew into as plain constants: the first time the formula is worked out after the load, with no edit
    // made since, constants that are exactly what it fills there are its own, and those of them that a cell in the way keeps it from are let go of, so that clearing that cell lets it fill whole)
    const bool loaded = m_loadedSpills.erase({sheet, formula}) > 0;
    struct Equal {
        quint32 row, col;
        Cell cell;
    };
    std::vector<Equal> adopt;
    bool wanted = true;   // the block asked for is the one told to the graph (too large a one is not)
    std::vector<CellArea> told;   // what the formula reads, besides its references: what it touched, and the block it asked for
    bool toldSet = false;
    // (with the index and a worker that serves, the window reads the store beside this thread: what is to be written to a cell of it is left to a thread that has the worker held back)
    const bool defer = s.cells.spills() && m_guard.workerServes();
    auto sameConstant = [&](const Cell &c, const Value &want) {
        if (c.flags & (HasFormula | ArrayMember | ArrayFormula | DataTable | SpillFormula | SpillMember)) return false;
        switch (c.type) {
        case CellType::Number: return want.isNumber() && want.number() == c.number;
        case CellType::Bool: return want.isBool() && want.boolean() == (c.number != 0);
        case CellType::SharedString: return want.isText() && want.text() == m_book.sharedStrings().at(c.textIndex());
        case CellType::InlineString: return want.isText() && want.text() == s.texts.value(c.textIndex());
        default: return false;
        }
    };
    if (rows * cols > kMaxSpillCells || qint64(row0) + rows > qint64(kGridRows) || qint64(col0) + cols > qint64(kMaxColumns)) {
        out = Value::error(jp::calc::ErrorKind::Spill);
        rows = cols = 1;
        wanted = false;
    } else if (rows * cols > 1) {
        bool blocked = false;
        // (the blocks of the other formulas, and the cells the store holds: decided by what is there now, never by a later round)
        if (const SpillPublisher *pub = s.cells.spills()) blocked = pub->mine().meetsOther(row0, col0, row0 + quint32(rows) - 1, col0 + quint32(cols) - 1, formula);
        if (!blocked)
        s.cells.forEachInRangeReal(row0, col0, row0 + quint32(rows) - 1, col0 + quint32(cols) - 1, [&](quint32 row, quint32 col, const Cell &c) {
            if (row == row0 && col == col0) return true;
            if (!heldByOther(c, formula)) return true;
            if (loaded && v.isArray() && sameConstant(c, v.at(int(row - row0), int(col - col0)))) {
                adopt.push_back({row, col, c});
                return true;
            }
            if (m_strayCount.load(std::memory_order_relaxed) > 0) {
                // (a cell of a loaded spill that its block let go of is no longer anyone's, though it waits to be taken from the store)
                const quint32 rowId = s.cells.rowIdAt(row), colId = s.cells.colIdAt(col);
                ++m_strayChecks;
                if (m_strayPlaces.count(StrayPlace{sheet, rowId, colId})) return true;
            }
            blocked = true;
            return loaded;   // (a loaded spill looks on through the block: the constants it finds are the old file's, whatever is in the way)
        });
        for (const Equal &e : adopt) {
            const quint32 rowId = s.cells.rowIdAt(e.row), colId = s.cells.colIdAt(e.col);
            if (defer) {
                m_strays.push_back({sheet, rowId, colId, formula, quint8(blocked ? 2 : 1), e.cell});
                m_strayCount.store(qint64(m_strays.size()), std::memory_order_release);
                if (blocked) tellGone(sheet, rowId, colId);
                continue;
            }
            Cell member = e.cell;
            if (blocked) {
                member.flags &= quint8(~(ArrayMember | SpillMember));
                member.formula = -1;
                member.type = CellType::Empty;
                member.number = 0;
                if (member.style == 0) s.cells.erase(e.row, e.col);
                else s.cells.set(e.row, e.col, member);
                tellGone(sheet, rowId, colId);
            } else {
                member.flags |= ArrayMember | SpillMember;
                member.formula = formula;
                s.cells.set(e.row, e.col, member);
            }
        }
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
        m_resizedSpills[{quint32(sheet), s.cells.rowIdAt(row0), s.cells.colIdAt(col0)}] = ++m_resizeSeq;
        const auto gone = membersOutside(s.cells, was, now, formula);
        std::vector<CellArea> changed;
        // (the cells of a loaded spill that its block no longer holds are told empty, and with `defer` let go of by a thread that has the worker held back)
        for (const auto &[row, col] : gone) {
            const quint32 rowId = s.cells.rowIdAt(row), colId = s.cells.colIdAt(col);
            if (defer) {
                m_strays.push_back({sheet, rowId, colId, formula});
                m_strayPlaces.insert(StrayPlace{sheet, rowId, colId});
                m_strayCount.store(qint64(m_strays.size()), std::memory_order_release);
                tellGone(sheet, rowId, colId);
                continue;
            }
            Cell *c = s.cells.findMutable(row, col);
            if (!c) continue;
            c->flags &= quint8(~(ArrayMember | SpillMember));
            c->formula = -1;
            c->type = CellType::Empty;
            c->number = 0;
            if (c->style == 0) s.cells.erase(row, col);
            tellGone(sheet, rowId, colId);
        }
        if (now.cellCount() > 1 && s.cells.spills()) {
            // (no cell is made: the block is a rectangle of the sheet's table)
            tellGoneBlock(sheet, formula, was, now, row0, col0);
            putSpillBlock(sheet, SpillRect{now.row1, now.col1, now.row2, now.col2, formula});
            useExtent(sheet, now.row2, now.col2);   // (the sheet's extentRows and extentCols are the window's: its reads add what the table reaches, CellStore::spillExtent)
        } else if (s.cells.spills()) {
            tellGoneBlock(sheet, formula, was, now, row0, col0);
            dropSpillBlock(sheet, formula);
        } else if (now.cellCount() > 1) {
            for (quint32 row = now.row1; row <= now.row2; ++row)
                for (quint32 col = now.col1; col <= now.col2; ++col) {
                    if (row == row0 && col == col0) continue;
                    const std::optional<Cell> have = s.cells.find(row, col);
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
        {
            const std::lock_guard<std::mutex> lock(m_rangesLock);   // (the window reads the list and the slot: see rangeOfFormula; a write to a list that an undo record shares makes a copy of it, which swaps its storage)
            if (slot >= 0 && slot < s.ranges.size()) {
                s.ranges[slot] = now;
            } else {
                slot = qint32(s.ranges.size());
                s.ranges.append(now);
                s.formulas[formula].range = slot;
            }
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
    // (a table that does not hold the block, as after an undo that took back the ranges, is made right here)
    if (SpillPublisher *pub = s.cells.spills()) {
        const SpillRect *have = pub->mine().ofFormula(formula);
        if (now.cellCount() > 1) {
            if (!have || have->row1 != now.row1 || have->col1 != now.col1 || have->row2 != now.row2 || have->col2 != now.col2) putSpillBlock(sheet, SpillRect{now.row1, now.col1, now.row2, now.col2, formula});
        } else if (have) {
            dropSpillBlock(sheet, formula);
        }
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
