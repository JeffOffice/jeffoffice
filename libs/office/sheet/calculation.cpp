#include "sheet/calculation.h"

#include "calc/formula.h"
#include "calc/functions.h"
#include "calc/refs.h"

#include <QDateTime>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

using namespace jp::calc;

namespace {

constexpr qint64 kMaxArrayCellsMarked = 1000000;   // an array formula's range larger than this is not made members of (no real one is)

} // namespace

Calculation::Calculation(const Workbook &book, const CalcOptions &options) : WorkbookSource(book, options.workbookBytes), m_options(options), m_graph(new DependencyGraph(options.maxGraphEntries))
{
    m_evaluator.seedRandom(quint64(QDateTime::currentMSecsSinceEpoch()) * 0x9E3779B97F4A7C15ull + 12345);
    if (options.workLimit > 0) m_evaluator.setWorkLimit(options.workLimit);
    m_iterate = book.info().iterate;
    m_ctx.dateSystem = book.info().dateSystem;
}

Calculation::~Calculation() = default;

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// formulas as nodes

Calculation::Compiled &Calculation::compiled(int sheet, qint32 text, bool array)
{
    SheetState &st = m_state[size_t(sheet)];
    std::vector<Compiled> &slots = array ? st.array : st.plain;
    if (slots.size() <= size_t(text)) slots.resize(size_t(text) + 1);
    Compiled &slot = slots[size_t(text)];
    if (!slot.tried) {
        slot.tried = true;
        ParseOptions po;
        po.arrayFormula = array;
        const ParseResult pr = parseFormula(m_sheets[size_t(sheet)].formulaTexts[text], po);
        slot.program = pr.program;
        slot.error = pr.error;
        if (slot.program) {
            const ProgramTraits t = traitsOf(*slot.program);
            slot.usesNames = t.usesNames;
            slot.dynamic = t.dynamic;
            slot.changesByItself = t.changesByItself;
            slot.bound = bindSheetNames(*slot.program, *this);
        }
        slot.traitsKnown = true;
    }
    return slot;
}

quint32 Calculation::addNode(int sheet, const Cell &cell, qint32 formulaIndex)
{
    quint32 id;
    if (!m_freeNodes.empty()) {
        id = m_freeNodes.back();
        m_freeNodes.pop_back();
        m_nodes[id] = Node();
    } else {
        id = quint32(m_nodes.size());
        m_nodes.emplace_back();
    }
    Node &n = m_nodes[id];
    n.sheet = sheet;
    n.formula = formulaIndex;
    n.row = cell.row;
    n.col = cell.col;
    SheetState &st = m_state[size_t(sheet)];
    if (st.nodeOf.size() <= size_t(formulaIndex)) st.nodeOf.resize(size_t(formulaIndex) + 1, kNone);
    st.nodeOf[size_t(formulaIndex)] = id;
    ++m_formulaCount;
    classify(id, cell, m_sheets[size_t(sheet)].formulas[formulaIndex]);
    return id;
}

void Calculation::classify(quint32 id, const Cell &cell, const Formula &f)
{
    Node &n = m_nodes[id];
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const bool array = cell.flags & ArrayFormula;
    n.flags = array ? Array : 0;
    char kept = 0;
    bool always = false, told = false;
    if (cell.flags & DataTable) {
        kept = 'D';
    } else if (f.text < 0 || f.text >= s.formulaTexts.size() || s.formulaTexts[f.text].trimmed().isEmpty()) {
        kept = 'N';
    } else {
        const Compiled &slot = compiled(n.sheet, f.text, array);
        if (!slot.program) {
            kept = 'P';
        } else {
            const Program &p = *slot.program;
            told = slot.usesNames || slot.dynamic;
            always = slot.dynamic || slot.changesByItself;
            if (m_options.keepApart) {
                if (!p.unknownFunctions.isEmpty()) kept = 'U';
                else if (p.external) kept = 'E';
                else if (p.structured) kept = 'T';
                else if (slot.changesByItself) kept = 'V';
                else if (!cell.hasResult()) kept = 'N';
            }
        }
    }
    if (kept) {
        n.flags |= Kept;
        n.kept = kept;
        if (m_options.keepApart) n.flags |= NodeTainted;
        keepStored(n, kept);
        n.state = Done;
        return;
    }
    if (always) {
        n.flags |= Always;
        m_always.push_back(id);
    }
    if (told) n.flags |= Told;
    n.state = Dirty;
    ++m_notDone;
    m_dirty.push_back(id);
    registerNode(id, nullptr);
}

void Calculation::keepStored(Node &n, char cls)
{
    const quint32 id = quint32(&n - m_nodes.data());
    if (!m_options.keepApart && cls == 'P') {
        storeResult(n, id, Value::error(ErrorKind::Name));
        return;
    }
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const Cell *c = s.cells.find(n.row, n.col);
    storeResult(n, id, c ? valueOf(s, *c) : Value());
}

void Calculation::registerNode(quint32 id, const std::vector<CellArea> *told)
{
    Node &n = m_nodes[id];
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const Formula &f = s.formulas[n.formula];
    const Compiled &slot = compiled(n.sheet, f.text, n.flags & Array);
    m_areas.clear();
    if (slot.program) resolveReferences(*slot.program, slot.bound, sheetCount(), n.sheet, f.dRow, f.dCol, &m_areas);
    if (told) m_areas.insert(m_areas.end(), told->begin(), told->end());
    std::sort(m_areas.begin(), m_areas.end());
    m_areas.erase(std::unique(m_areas.begin(), m_areas.end()), m_areas.end());
    if (!m_graph->setPrecedents(id, m_areas)) {
        if (!(n.flags & Untracked)) {
            n.flags |= Untracked;
            ++m_untracked;
        }
        if (!(n.flags & Always)) {
            n.flags |= Always;
            m_always.push_back(id);
        }
    }
}

CellArea Calculation::areaOf(const Node &n) const
{
    CellArea a;
    a.sheet1 = a.sheet2 = n.sheet;
    a.row1 = a.row2 = n.row;
    a.col1 = a.col2 = n.col;
    if (n.flags & Array) {
        const Sheet &s = m_sheets[size_t(n.sheet)];
        const qint32 range = s.formulas[n.formula].range;
        if (range >= 0 && range < s.ranges.size() && s.ranges[range].valid) {
            a.row2 = std::min<quint32>(s.ranges[range].row2, kGridRows - 1);
            a.col2 = std::min<quint32>(s.ranges[range].col2, kMaxColumns - 1);
        }
    }
    return a;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// values

Value Calculation::nodeValue(const Node &n, quint32 id) const
{
    switch (n.kind) {
    case KindNumber: return Value::number(n.number);
    case KindText: return Value::text(m_texts.value(n.text));
    case KindBool: return Value::boolean(n.number != 0);
    case KindError: return Value::error(ErrorKind(int(n.number)));
    case KindArray: return m_arrays.value(id).at(0, 0);
    default: return Value();
    }
}

void Calculation::storeResult(Node &n, quint32 id, const Value &v)
{
    if (n.kind == KindText && n.text >= 0 && !v.isText()) {
        m_freeTexts.push_back(n.text);
        n.text = -1;
    }
    if (n.kind == KindArray && !v.isArray()) m_arrays.remove(id);
    switch (v.kind()) {
    case Value::Kind::Number: n.kind = KindNumber; n.number = v.number(); break;
    case Value::Kind::Bool: n.kind = KindBool; n.number = v.boolean() ? 1 : 0; break;
    case Value::Kind::Error: n.kind = KindError; n.number = double(int(v.errorKind())); break;
    case Value::Kind::Text:
        n.kind = KindText;
        if (n.text < 0) {
            if (!m_freeTexts.empty()) {
                n.text = m_freeTexts.back();
                m_freeTexts.pop_back();
            } else {
                n.text = qint32(m_texts.size());
                m_texts.append(QString());
            }
        }
        m_texts[n.text] = v.text();
        break;
    case Value::Kind::Array:
        n.kind = KindArray;
        m_arrays.insert(id, v);
        break;
    case Value::Kind::Empty: n.kind = KindEmpty; n.number = 0; break;
    }
}

bool Calculation::formulaValue(int sheet, const Cell &c, Value *out) const
{
    const SheetState &st = m_state[size_t(sheet)];
    const quint32 id = c.formula >= 0 && size_t(c.formula) < st.nodeOf.size() ? st.nodeOf[size_t(c.formula)] : kNone;
    if (id == kNone) {
        *out = Value();
        return true;
    }
    const Node &n = m_nodes[id];
    if (m_evaluating && n.state != Done) {
        m_pending.push_back(id);
        *out = Value();
        return false;
    }
    if (n.flags & NodeTainted) m_readTaint = true;
    if (n.flags & (Mismatch | Cascade)) m_readCascade = true;
    if (c.flags & ArrayMember) {
        const Value array = m_arrays.value(id);
        *out = array.isArray() ? array.at(int(c.row - n.row), int(c.col) - int(n.col)) : nodeValue(n, id);
    } else {
        *out = nodeValue(n, id);
    }
    return true;
}

Value Calculation::cell(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns) return Value();
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c) return Value();
    if (c->hasFormula() || (c->flags & ArrayMember)) {
        Value v;
        formulaValue(sheet, *c, &v);
        return v;
    }
    if (c->flags & Tainted) m_readTaint = true;
    return valueOf(s, *c);
}

void Calculation::visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    const Sheet &s = m_sheets[size_t(sheet)];
    s.cells.forEachInRange(row1, col1, row2, std::min<quint32>(col2, kMaxColumns - 1), [&](const Cell &c) {
        if (c.hasFormula() || (c.flags & ArrayMember)) {
            Value v;
            if (!formulaValue(sheet, c, &v) || v.isEmpty()) return true;
            return f(c.row, c.col, v);
        }
        if (c.type == CellType::Empty) return true;
        if (c.flags & Tainted) m_readTaint = true;
        return f(c.row, c.col, valueOf(s, c));
    });
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// loading

bool Calculation::load()
{
    const bool whole = loadAll();
    const int sheets = int(m_sheets.size());
    m_state.assign(size_t(sheets), SheetState());
    m_nodes.clear();
    m_dirty.clear();
    m_always.clear();
    std::vector<quint32> masters;   // array formulas, whose cells are then made members of them
    for (int si = 0; si < sheets; ++si) {
        Sheet &s = m_sheets[size_t(si)];
        SheetState &st = m_state[size_t(si)];
        st.nodeOf.assign(size_t(s.formulas.size()), kNone);
        st.plain.resize(size_t(s.formulaTexts.size()));
        st.array.resize(size_t(s.formulaTexts.size()));
        quint32 rows = 0, cols = 0;
        s.cells.forEach([&](const Cell &c) {
            if (c.type != CellType::Empty || c.hasFormula()) {
                rows = std::max(rows, c.row + 1);
                cols = std::max(cols, quint32(c.col) + 1);
            }
            if (!c.hasFormula() || c.formula < 0 || c.formula >= s.formulas.size()) return;
            const quint32 id = addNode(si, c, c.formula);
            if (m_nodes[id].flags & Array) masters.push_back(id);
        });
        m_rows[size_t(si)] = rows;
        m_cols[size_t(si)] = cols;
    }
    // the cells of an array formula's range, but its first, are its: they read its value (a data table's, and those of an array that is kept apart, are the file's, marked)
    for (const quint32 id : masters) {
        Node &n = m_nodes[id];
        Sheet &s = m_sheets[size_t(n.sheet)];
        const qint32 range = s.formulas[n.formula].range;
        if (range < 0 || range >= s.ranges.size() || !s.ranges[range].valid) continue;
        const CellRange r = s.ranges[range];
        if (r.cellCount() <= 1 || r.cellCount() > kMaxArrayCellsMarked) continue;
        for (quint32 row = r.row1; row <= r.row2; ++row)
            for (quint32 col = r.col1; col <= r.col2; ++col) {
                if (row == n.row && col == n.col) continue;
                Cell member;
                if (const Cell *have = s.cells.find(row, col)) member = *have;
                else {
                    member.row = row;
                    member.col = quint16(col);
                }
                if (member.hasFormula()) continue;
                if (n.flags & Kept) {
                    member.flags |= Tainted;
                } else {
                    member.flags |= ArrayMember;
                    member.formula = n.formula;
                }
                s.cells.set(member);
                m_rows[size_t(n.sheet)] = std::max(m_rows[size_t(n.sheet)], row + 1);
                m_cols[size_t(n.sheet)] = std::max(m_cols[size_t(n.sheet)], col + 1);
            }
    }
    // a data table's cells are the file's, marked
    for (int si = 0; si < sheets; ++si) {
        Sheet &s = m_sheets[size_t(si)];
        const SheetState &st = m_state[size_t(si)];
        for (size_t fi = 0; fi < st.nodeOf.size(); ++fi) {
            if (st.nodeOf[fi] == kNone || m_nodes[st.nodeOf[fi]].kept != 'D') continue;
            const qint32 range = s.formulas[qint32(fi)].range;
            if (range < 0 || range >= s.ranges.size() || !s.ranges[range].valid || s.ranges[range].cellCount() > kMaxArrayCellsMarked) continue;
            const CellRange r = s.ranges[range];
            for (quint32 row = r.row1; row <= r.row2; ++row)
                for (quint32 col = r.col1; col <= r.col2; ++col)
                    if (Cell *c = s.cells.findMutable(row, col)) c->flags |= Tainted;
        }
    }
    return whole;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// recalculation

void Calculation::markDirty(quint32 id, std::vector<CellArea> *changed)
{
    Node &n = m_nodes[id];
    n.state = Dirty;
    ++m_notDone;
    m_dirty.push_back(id);
    changed->push_back(areaOf(n));
}

void Calculation::markDependents(std::vector<CellArea> *changed)
{
    std::vector<quint32> found;
    while (!changed->empty()) {
        const CellArea a = changed->back();
        changed->pop_back();
        found.clear();
        if (a.single()) m_graph->dependentsOfCell(a.sheet1, a.row1, a.col1, &found);
        else m_graph->dependentsOfArea(a, &found);
        for (const quint32 id : found) {
            const Node &n = m_nodes[id];
            if ((n.flags & (Dead | Kept)) || n.state != Done) continue;
            markDirty(id, changed);
        }
    }
}

void Calculation::beginRecalculation()
{
    m_running = true;
    m_now = m_options.now > 0 ? m_options.now : clockSerial(m_ctx.dateSystem);
    std::vector<CellArea> changed;
    for (const quint32 id : m_always) {
        const Node &n = m_nodes[id];
        if ((n.flags & (Dead | Kept)) || !(n.flags & Always) || n.state != Done) continue;
        markDirty(id, &changed);
    }
    markDependents(&changed);
    m_dirtyCursor = 0;
}

void Calculation::abandon()
{
    if (!m_running && m_stack.empty()) return;
    // what is waiting or queued is to be worked out again, with what was not yet started
    std::vector<quint32> again;
    for (const Frame &f : m_stack) {
        Node &n = m_nodes[f.node];
        if (n.state == Waiting || n.state == Queued) {
            n.state = Dirty;
            again.push_back(f.node);
        }
    }
    for (size_t i = m_dirtyCursor; i < m_dirty.size(); ++i) again.push_back(m_dirty[i]);
    m_stack.clear();
    m_dirty.swap(again);
    m_dirtyCursor = 0;
    m_running = false;
}

Value Calculation::runFormula(quint32 id, qint64 *work)
{
    Node &n = m_nodes[id];
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const Formula &f = s.formulas[n.formula];
    const Compiled &slot = compiled(n.sheet, f.text, n.flags & Array);
    m_ctx.source = this;
    m_ctx.sheet = n.sheet;
    m_ctx.row = n.row;
    m_ctx.col = n.col;
    m_ctx.shiftRows = f.dRow;
    m_ctx.shiftCols = f.dCol;
    m_ctx.now = m_now;
    m_touched.clear();
    m_ctx.touched = (n.flags & Told) ? &m_touched : nullptr;
    m_pending.clear();
    m_readTaint = false;
    m_readCascade = false;
    m_evaluating = true;
    const Value v = m_evaluator.evaluate(*slot.program, m_ctx);
    m_evaluating = false;
    *work = m_evaluator.work();
    return v;
}

Calculation::Progress Calculation::recalculate(qint64 budget, const std::atomic<bool> *cancel)
{
    Progress pr;
    if (!m_running) beginRecalculation();
    qint64 spent = 0;
    for (;;) {
        if (cancel && cancel->load(std::memory_order_relaxed)) {
            abandon();
            pr.cancelled = true;
            return pr;
        }
        if (m_stack.empty()) {
            quint32 root = kNone;
            while (m_dirtyCursor < m_dirty.size()) {
                const quint32 id = m_dirty[m_dirtyCursor++];
                const Node &n = m_nodes[id];
                if (n.state == Dirty && !(n.flags & Dead)) {
                    root = id;
                    break;
                }
            }
            if (root == kNone) break;
            m_nodes[root].state = Queued;
            m_stack.push_back({root});
        }
        if (budget > 0 && spent >= budget) return pr;
        const quint32 id = m_stack.back().node;
        if (m_nodes[id].state == Done || (m_nodes[id].flags & Dead)) {
            m_stack.pop_back();
            continue;
        }
        qint64 work = 0;
        const Value v = runFormula(id, &work);
        Node &n = m_nodes[id];
        spent += work;
        pr.work += work;
        ++pr.evaluated;
        if (m_pending.empty()) {
            const EvalStatus status = m_evaluator.status();
            const int unsupported = m_evaluator.unsupportedKinds();
            if (m_options.keepApart && unsupported) {
                n.flags |= Kept | NodeTainted;
                n.kept = (unsupported & 1) ? 'E' : (unsupported & 2) ? 'T' : 'U';
                keepStored(n, n.kept);
            } else if (m_options.keepApart && status != EvalStatus::Ok) {
                n.flags |= Kept | NodeTainted;
                n.kept = 'L';
                keepStored(n, 'L');
            } else {
                storeResult(n, id, v);
                n.flags &= quint16(~Circular);
                if (m_readTaint) n.flags |= NodeTainted;
                else n.flags &= quint16(~NodeTainted);
                n.flags &= quint16(~(Mismatch | Cascade));
                if (m_readCascade) n.flags |= Cascade;
                if (m_options.verify) {
                    const Sheet &sh = m_sheets[size_t(n.sheet)];
                    const Cell *stored = sh.cells.find(n.row, n.col);
                    if (stored && !m_options.verify(valueOf(sh, *stored), nodeValue(n, id))) n.flags |= Mismatch;
                }
            }
            if (n.flags & Told) registerNode(id, &m_touched);
            n.state = Done;
            --m_notDone;
            ++pr.recalculated;
            m_stack.pop_back();
            continue;
        }
        // it read formulas that are not worked out: they are worked out first, and it again after them; one that is waiting (or itself) is a circle
        std::sort(m_pending.begin(), m_pending.end());
        m_pending.erase(std::unique(m_pending.begin(), m_pending.end()), m_pending.end());
        size_t circleFrom = m_stack.size();
        for (const quint32 y : m_pending) {
            if (y != id && m_nodes[y].state != Waiting) continue;
            size_t at = m_stack.size() - 1;
            if (y != id) {
                while (at > 0 && !(m_stack[at].node == y && m_nodes[y].state == Waiting)) --at;
            }
            circleFrom = std::min(circleFrom, at);
        }
        if (circleFrom < m_stack.size()) {
            for (size_t i = circleFrom; i < m_stack.size(); ++i) {
                Node &c = m_nodes[m_stack[i].node];
                if (c.state == Done || (c.state != Waiting && m_stack[i].node != id)) continue;
                if (m_options.keepApart && m_iterate) {
                    c.flags |= Kept | NodeTainted | Circular;
                    c.kept = 'I';
                    keepStored(c, 'I');
                } else {
                    storeResult(c, m_stack[i].node, Value::number(0));
                    c.flags |= Circular;
                    c.flags &= quint16(~NodeTainted);
                }
                c.state = Done;
                --m_notDone;
                ++pr.recalculated;
                ++pr.circular;
            }
            continue;
        }
        n.state = Waiting;
        for (const quint32 y : m_pending) {
            Node &d = m_nodes[y];
            if (d.state == Dirty) d.state = Queued;
            m_stack.push_back({y});
        }
        pr.deepest = std::max<qint64>(pr.deepest, qint64(m_stack.size()));
    }
    m_running = false;
    m_dirty.clear();
    m_dirtyCursor = 0;
    pr.finished = true;
    return pr;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// edits

void Calculation::useExtent(int sheet, quint32 row, quint32 col)
{
    m_rows[size_t(sheet)] = std::max(m_rows[size_t(sheet)], row + 1);
    m_cols[size_t(sheet)] = std::max(m_cols[size_t(sheet)], col + 1);
}

void Calculation::dropNode(quint32 id)
{
    Node &n = m_nodes[id];
    if (n.flags & Dead) return;
    if (!(n.flags & Kept) && n.state != Done) --m_notDone;
    if (n.kind == KindText && n.text >= 0) m_freeTexts.push_back(n.text);
    m_arrays.remove(id);
    m_graph->clearPrecedents(id);
    SheetState &st = m_state[size_t(n.sheet)];
    if (size_t(n.formula) < st.nodeOf.size()) st.nodeOf[size_t(n.formula)] = kNone;
    n.flags = Dead;
    n.state = Done;
    n.kind = KindEmpty;
    n.text = -1;
    --m_formulaCount;
    m_freeNodes.push_back(id);
}

bool Calculation::setValue(int sheet, quint32 row, quint32 col, const Value &v)
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns || row >= kGridRows || v.isArray()) return false;
    Sheet &s = m_sheets[size_t(sheet)];
    const Cell *found = s.cells.find(row, col);
    if (found && ((found->flags & (ArrayMember | ArrayFormula)))) return false;   // (a cell of an array is not edited alone)
    abandon();
    Cell c = found ? *found : Cell();
    c.row = row;
    c.col = quint16(col);
    if (found && found->hasFormula()) {
        const SheetState &st = m_state[size_t(sheet)];
        if (found->formula >= 0 && size_t(found->formula) < st.nodeOf.size() && st.nodeOf[size_t(found->formula)] != kNone) dropNode(st.nodeOf[size_t(found->formula)]);
    }
    c.flags &= quint8(~(HasFormula | SharedFormula | DataTable | Tainted));
    c.formula = -1;
    switch (v.kind()) {
    case Value::Kind::Number: c.type = CellType::Number; c.number = v.number(); break;
    case Value::Kind::Bool: c.type = CellType::Bool; c.number = v.boolean() ? 1 : 0; break;
    case Value::Kind::Error: c.type = CellType::Error; c.number = double(int(v.errorKind())); break;
    case Value::Kind::Text:
        if (c.type == CellType::InlineString && c.textIndex() >= 0 && c.textIndex() < s.texts.size()) {
            s.texts[c.textIndex()] = v.text();
        } else {
            s.texts.append(v.text());
            c.number = double(s.texts.size() - 1);
        }
        c.type = CellType::InlineString;
        break;
    default: c.type = CellType::Empty; c.number = 0; break;
    }
    if (c.type == CellType::Empty && c.style == 0) s.cells.erase(row, col);
    else s.cells.set(c);
    if (c.type != CellType::Empty) useExtent(sheet, row, col);
    std::vector<CellArea> changed;
    CellArea a;
    a.sheet1 = a.sheet2 = sheet;
    a.row1 = a.row2 = row;
    a.col1 = a.col2 = col;
    changed.push_back(a);
    markDependents(&changed);
    return true;
}

bool Calculation::setFormula(int sheet, quint32 row, quint32 col, const QString &formula)
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns || row >= kGridRows) return false;
    ParseOptions po;
    const ParseResult parsed = parseFormula(formula, po);
    if (!parsed.ok()) return false;
    Sheet &s = m_sheets[size_t(sheet)];
    const Cell *found = s.cells.find(row, col);
    if (found && (found->flags & (ArrayMember | ArrayFormula))) return false;
    abandon();
    SheetState &st = m_state[size_t(sheet)];
    Cell c = found ? *found : Cell();
    c.row = row;
    c.col = quint16(col);
    if (found && found->hasFormula() && found->formula >= 0 && size_t(found->formula) < st.nodeOf.size() && st.nodeOf[size_t(found->formula)] != kNone) dropNode(st.nodeOf[size_t(found->formula)]);
    s.formulaTexts.append(formula);
    Formula f;
    f.text = qint32(s.formulaTexts.size() - 1);
    s.formulas.append(f);
    c.flags = quint8((c.flags & ~(SharedFormula | DataTable | Tainted)) | HasFormula);
    c.formula = qint32(s.formulas.size() - 1);
    c.type = CellType::Empty;
    c.number = 0;
    s.cells.set(c);
    useExtent(sheet, row, col);
    const quint32 id = addNode(sheet, c, c.formula);
    std::vector<CellArea> changed;
    changed.push_back(areaOf(m_nodes[id]));
    markDependents(&changed);
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// about the formulas

quint32 Calculation::nodeAt(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return kNone;
    const Cell *c = m_sheets[size_t(sheet)].cells.find(row, col);
    if (!c || !c->hasFormula() || c->formula < 0) return kNone;
    const SheetState &st = m_state[size_t(sheet)];
    return size_t(c->formula) < st.nodeOf.size() ? st.nodeOf[size_t(c->formula)] : kNone;
}

bool Calculation::isCircular(int sheet, quint32 row, quint32 col) const
{
    const quint32 id = nodeAt(sheet, row, col);
    return id != kNone && (m_nodes[id].flags & Circular);
}

bool Calculation::isTainted(int sheet, quint32 row, quint32 col) const
{
    const quint32 id = nodeAt(sheet, row, col);
    return id != kNone && (m_nodes[id].flags & NodeTainted);
}

bool Calculation::isCascaded(int sheet, quint32 row, quint32 col) const
{
    const quint32 id = nodeAt(sheet, row, col);
    return id != kNone && (m_nodes[id].flags & Cascade);
}

char Calculation::keptClass(int sheet, quint32 row, quint32 col) const
{
    const quint32 id = nodeAt(sheet, row, col);
    return id != kNone ? m_nodes[id].kept : 0;
}

void Calculation::forEachFormula(const std::function<void(const FormulaCell &)> &f) const
{
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        const Sheet &s = m_sheets[size_t(si)];
        const SheetState &st = m_state[size_t(si)];
        s.cells.forEach([&](const Cell &c) {
            if (!c.hasFormula() || c.formula < 0 || size_t(c.formula) >= st.nodeOf.size() || st.nodeOf[size_t(c.formula)] == kNone) return;
            const quint32 id = st.nodeOf[size_t(c.formula)];
            const Node &n = m_nodes[id];
            FormulaCell out;
            out.sheet = si;
            out.cell = &c;
            out.stored = valueOf(s, c);
            out.ours = nodeValue(n, id);
            out.kept = n.kept;
            out.tainted = n.flags & NodeTainted;
            out.circular = n.flags & Circular;
            out.mismatch = n.flags & Mismatch;
            out.cascaded = n.flags & Cascade;
            out.worked = n.state == Done;
            out.changesByItself = false;
            out.readsWhereToldOrNames = n.flags & Told;
            out.program = nullptr;
            const Formula &fm = s.formulas[n.formula];
            if (fm.text >= 0 && fm.text < s.formulaTexts.size()) {
                const std::vector<Compiled> &slots = (n.flags & Array) ? st.array : st.plain;
                if (size_t(fm.text) < slots.size() && slots[size_t(fm.text)].program) {
                    const Compiled &slot = slots[size_t(fm.text)];
                    out.changesByItself = slot.changesByItself;
                    out.program = slot.program.get();
                }
            }
            f(out);
        });
    }
}

} // namespace jp::office::sheet
