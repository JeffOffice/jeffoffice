#include "sheet/calculation.h"

#include "calc/budget.h"
#include "calc/formula.h"
#include "calc/functions.h"
#include "calc/refs.h"

#include <QDateTime>

#include <algorithm>
#include <chrono>
#include <cmath>

namespace jp::office::sheet {

using namespace jp::calc;

namespace {

// A number that says which blocks these are (in the order given): two lists with the same number are taken to be the same, as the graph takes them.
quint64 hashOfAreas(const std::vector<CellArea> &areas)
{
    quint64 h = 1469598103934665603ull ^ quint64(areas.size());
    auto mix = [&](quint64 v) {
        h ^= v + 0x9E3779B97F4A7C15ull + (h << 6) + (h >> 2);
        h *= 0xFF51AFD7ED558CCDull;
        h ^= h >> 32;
    };
    for (const CellArea &a : areas) {
        mix((quint64(quint32(a.sheet1)) << 32) | quint64(quint32(a.sheet2)));
        mix((quint64(a.row1) << 32) | quint64(a.row2));
        mix((quint64(a.col1) << 32) | quint64(a.col2));
    }
    return h ? h : 1;
}

constexpr qint64 kMaxArrayCellsMarked = 1000000;   // an array formula's range larger than this is not made members of (no real one is)
constexpr qint64 kMaxMembersAdded = 200000;        // the cells of arrays that the file has no cell for, which are put in (each costs the cells after it in its block of rows)

} // namespace

Calculation::Calculation(const Workbook &book, const CalcOptions &options) : WorkbookSource(book, options.workbookBytes), m_options(options), m_graph(new DependencyGraph(options.maxGraphBytes))
{
    m_evaluator.seedRandom(quint64(QDateTime::currentMSecsSinceEpoch()) * 0x9E3779B97F4A7C15ull + 12345);
    if (options.workLimit > 0) m_evaluator.setWorkLimit(options.workLimit);
    m_iterate = book.info().iterate;
    m_ctx.dateSystem = book.info().dateSystem;
    if (options.recalcWorkLimit > 0) m_workLimit = options.recalcWorkLimit;
    if (options.maxHeldBytes > 0) m_heldLimit = options.maxHeldBytes;
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
            slot.bound = bindSheetNames(*slot.program, *this);
            // (what the names it uses stand for counts: a name that calls RAND makes the formula change by itself)
            const ProgramTraits t = traitsFollowingNames(*slot.program, slot.bound, sheet, *this, &m_nameTraits);
            slot.usesNames = t.usesNames;
            slot.dynamic = t.dynamic;
            slot.reaches = t.reachesBeyond;
            slot.changesByItself = t.changesByItself;
            slot.iterative = t.iterative;
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
        const quint16 listed = m_nodes[id].flags & AlwaysListed;   // (a gone node may still be on the list of those worked out each time)
        m_nodes[id] = Node();
        m_nodes[id].flags = listed;
        m_toldHash[id] = 0;
    } else {
        id = quint32(m_nodes.size());
        m_nodes.emplace_back();
        m_index.push_back(0);
        m_low.push_back(0);
        m_toldHash.push_back(0);
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
    n.flags = quint16((n.flags & AlwaysListed) | (array ? Array : 0));
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
            told = slot.usesNames || slot.dynamic || slot.reaches;
            always = slot.dynamic || slot.changesByItself;
            if (m_options.keepApart) {
                if (!p.unknownFunctions.isEmpty()) kept = 'U';
                else if (p.external) kept = 'E';
                else if (p.structured) kept = 'T';
                else if (slot.changesByItself) kept = 'V';
                else if (slot.iterative) kept = 'R';
                else if (!cell.hasResult() && !m_options.workOutWhatHasNoResult) kept = 'N';
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
        listAlways(id);
    }
    if (told) n.flags |= Told;
    n.state = Dirty;
    ++m_notDone;
    m_dirty.push_back(id);
    registerNode(id, nullptr);
}

void Calculation::listAlways(quint32 id)
{
    Node &n = m_nodes[id];
    if (n.flags & AlwaysListed) return;
    n.flags |= AlwaysListed;
    m_always.push_back(id);
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
    storeResult(n, id, c ? valueOf(s, *c) : Value(), false);   // (what the file stored is what the grid shows already, and is not told)
}

void Calculation::registerNode(quint32 id, const std::vector<CellArea> *told)
{
    Node &n = m_nodes[id];
    // (what a formula's own references name does not change; what it touched as it was worked out is told only if it is not what it touched the last time)
    quint64 toldHash = 0;
    if (told) {
        m_toldSet.assign(told->begin(), told->end());
        std::sort(m_toldSet.begin(), m_toldSet.end());
        m_toldSet.erase(std::unique(m_toldSet.begin(), m_toldSet.end()), m_toldSet.end());
        toldHash = hashOfAreas(m_toldSet);
        if (m_toldHash[id] == toldHash) return;
    }
    const Sheet &s = m_sheets[size_t(n.sheet)];
    const Formula &f = s.formulas[n.formula];
    const Compiled &slot = compiled(n.sheet, f.text, n.flags & Array);
    m_areas.clear();
    if (slot.program) resolveReferences(*slot.program, slot.bound, sheetCount(), n.sheet, f.dRow, f.dCol, &m_areas);
    if (told) m_areas.insert(m_areas.end(), m_toldSet.begin(), m_toldSet.end());
    std::sort(m_areas.begin(), m_areas.end());
    m_areas.erase(std::unique(m_areas.begin(), m_areas.end()), m_areas.end());
    ++m_registrations;
    const bool tracked = m_graph->setPrecedents(id, m_areas);
    m_toldHash[id] = tracked ? toldHash : 0;   // (a formula the graph refused is told again next time)
    if (!tracked) {
        if (!(n.flags & Untracked)) {
            n.flags |= Untracked;
            ++m_untracked;
        }
        if (!(n.flags & Always)) {
            n.flags |= Always;
            listAlways(id);
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

// A slot of the texts that is free holds no string (the one it held is let go of, not kept for the slot's next use).
void Calculation::freeTextSlot(qint32 slot)
{
    m_texts[slot] = QString();
    m_freeTexts.push_back(slot);
}

qint64 Calculation::textSlotBytes() const
{
    qint64 bytes = 0;
    for (const QString &t : m_texts) bytes += qint64(t.capacity()) * 2;
    return bytes;
}

qint64 Calculation::heldByValue(const Value &v)
{
    if (v.isText()) return qint64(v.text().size()) * 2 + 64;
    if (v.isArray()) return qint64(v.rows()) * v.cols() * qint64(sizeof(Value)) + 96;
    return 0;
}

qint64 Calculation::heldBy(const Node &n, quint32 id) const
{
    if (n.kind == KindText && n.text >= 0) return qint64(m_texts.value(n.text).size()) * 2 + 64;
    if (n.kind == KindArray) return heldByValue(m_arrays.value(id));
    return 0;
}

// An array formula's cells are those of its range: the rest of the array it made is not held (a formula of one cell holds one value).
Value Calculation::fitToRange(const Node &n, const Value &v) const
{
    if (!(n.flags & Array) || !v.isArray()) return v;
    const CellArea a = areaOf(n);
    const int rows = int(std::min<qint64>(a.rows(), v.rows())), cols = int(std::min<qint64>(a.cols(), v.cols()));
    if (rows == v.rows() && cols == v.cols()) return v;
    if (rows == 1 && cols == 1) return v.at(0, 0);
    Value out = Value::array(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) out.set(i, j, v.at(i, j));
    return out;
}

void Calculation::storeResult(Node &n, quint32 id, const Value &given, bool publish)
{
    const Value v = fitToRange(n, given);
    if (publish && m_sink) {
        if (v.isArray()) {
            for (int i = 0; i < v.rows(); ++i)
                for (int j = 0; j < v.cols(); ++j) m_sink(n.sheet, n.row + quint32(i), quint32(n.col) + quint32(j), v.at(i, j));
        } else {
            m_sink(n.sheet, n.row, n.col, v);
        }
    }
    const qint64 held = heldBy(n, id);
    struct Account {
        Calculation *c;
        Node &n;
        quint32 id;
        qint64 before;
        ~Account() { c->m_heldBytes += c->heldBy(n, id) - before; }
    } account{this, n, id, held};
    if (n.kind == KindText && n.text >= 0 && !v.isText()) {
        freeTextSlot(n.text);
        n.text = -1;
    }
    if (n.kind == KindArray && !v.isArray()) m_arrays.remove(id);
    n.flags |= Worked;
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

Value Calculation::shown(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size()) || col >= kMaxColumns) return Value();
    const Sheet &s = m_sheets[size_t(sheet)];
    const Cell *c = s.cells.find(row, col);
    if (!c) return Value();
    if ((c->hasFormula() || (c->flags & ArrayMember)) && size_t(sheet) < m_state.size()) {
        const SheetState &st = m_state[size_t(sheet)];
        const quint32 id = c->formula >= 0 && size_t(c->formula) < st.nodeOf.size() ? st.nodeOf[size_t(c->formula)] : kNone;
        if (id != kNone && m_nodes[id].state != Done && !(m_nodes[id].flags & Worked)) return valueOf(s, *c);
        Value v;
        formulaValue(sheet, *c, &v);
        return v;
    }
    return valueOf(s, *c);
}

void Calculation::visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    const Sheet &s = m_sheets[size_t(sheet)];
    CellStore::RangeWalk walk;
    s.cells.forEachInRange(row1, col1, row2, std::min<quint32>(col2, kMaxColumns - 1), [&](const Cell &c) {
        if (c.hasFormula() || (c.flags & ArrayMember)) {
            Value v;
            if (!formulaValue(sheet, c, &v) || v.isEmpty()) return true;
            return f(c.row, c.col, v);
        }
        if (c.type == CellType::Empty) return true;
        if (c.flags & Tainted) m_readTaint = true;
        return f(c.row, c.col, valueOf(s, c));
    }, &walk);
    jp::calc::budget::addWork((walk.compared + walk.stepped) / 4);   // (the rows walked over to find the cells are work too: a column of a sheet of many rows is read row by row)
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// loading

bool Calculation::load()
{
    static_assert(sizeof(Node) == 32, "a formula's node is 32 bytes");
    const bool whole = loadAll();
    const int sheets = int(m_sheets.size());
    m_state.assign(size_t(sheets), SheetState());
    m_nodes.clear();
    m_index.clear();
    m_low.clear();
    m_toldHash.clear();
    m_comp.clear();
    m_counter = 0;
    m_dirty.clear();
    m_always.clear();
    std::vector<quint32> masters;   // array formulas, whose cells are then made members of them
    bool cancelled = false;
    for (int si = 0; si < sheets; ++si) {
        Sheet &s = m_sheets[size_t(si)];
        SheetState &st = m_state[size_t(si)];
        st.nodeOf.assign(size_t(s.formulas.size()), kNone);
        st.plain.resize(size_t(s.formulaTexts.size()));
        st.array.resize(size_t(s.formulaTexts.size()));
        quint32 rows = 0, cols = 0;
        qint64 visited = 0;
        s.cells.forEach([&](const Cell &c) {
            if (cancelled) return;
            // (the flag is looked at every few thousand cells: a window that is closed does not wait for the formulas of a big sheet to be registered)
            if ((++visited & 4095) == 0 && m_cancel && m_cancel->load(std::memory_order_relaxed)) {
                cancelled = true;
                return;
            }
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
        if (cancelled) return false;
    }
    // the cells of an array formula's range, but its first, are its: they read its value (a data table's, and those of an array that is kept apart, are the file's, marked)
    qint64 added = 0;
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
                if (const Cell *have = s.cells.find(row, col)) {
                    member = *have;
                } else {
                    if (++added > kMaxMembersAdded) continue;
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
    m_workUsed = 0;
    m_now = m_options.now > 0 ? m_options.now : clockSerial(m_ctx.dateSystem);
    // (what the recalculation begins with is marked a chunk at a time by stepBegin: the formulas that change by themselves, and all that read them)
    m_begin = BeginJob();
    m_begin.active = true;
}

// Marks the formulas that are worked out in every recalculation, and those that read them, to be worked out; a chunk at a time. True when all are marked.
bool Calculation::stepBegin(const std::function<bool()> &timeUp)
{
    constexpr size_t kChunk = 256;
    BeginJob &j = m_begin;
    size_t done = 0;
    // (the list is kept to the formulas that are on it: one that is gone, or not volatile now, is left off)
    while (!j.dependents && j.alwaysAt < m_always.size()) {
        if (done >= kChunk) {
            if (timeUp()) return false;
            done = 0;
        }
        ++done;
        const quint32 id = m_always[j.alwaysAt++];
        Node &n = m_nodes[id];
        if ((n.flags & Dead) || !(n.flags & Always)) {
            n.flags &= quint16(~AlwaysListed);
            continue;
        }
        m_always[j.listed++] = id;
        if ((n.flags & Kept) || n.state != Done) continue;
        markDirty(id, &j.changed);
    }
    if (!j.dependents) {
        m_always.resize(j.listed);
        j.dependents = true;
    }
    // what reads those marked: each block that changed asks the graph, and what it finds is marked and changes in its turn
    for (;;) {
        while (j.foundAt < j.found.size()) {
            if (done >= kChunk) {
                if (timeUp()) return false;
                done = 0;
            }
            ++done;
            const quint32 id = j.found[j.foundAt++];
            const Node &n = m_nodes[id];
            if ((n.flags & (Dead | Kept)) || n.state != Done) continue;
            markDirty(id, &j.changed);
        }
        if (j.changed.empty()) break;
        if (done >= kChunk) {
            if (timeUp()) return false;
            done = 0;
        }
        const CellArea a = j.changed.back();
        j.changed.pop_back();
        j.found.clear();
        j.foundAt = 0;
        if (a.single()) m_graph->dependentsOfCell(a.sheet1, a.row1, a.col1, &j.found);
        else m_graph->dependentsOfArea(a, &j.found);
        done += j.found.size() / 16 + 1;   // (what the graph looks through to find them)
    }
    m_dirtyCursor = 0;
    m_begin = BeginJob();
    return true;
}

void Calculation::invalidateAll()
{
    abandon();
    // (every formula is marked to be worked out, a chunk at a time by the next recalculation: asking costs nothing, and the count says how many are to be done)
    m_invalidate.active = true;
    m_invalidate.at = 0;
}

bool Calculation::stepInvalidate(const std::function<bool()> &timeUp)
{
    constexpr size_t kChunk = 512;
    size_t done = 0;
    while (m_invalidate.at < m_nodes.size()) {
        if (done >= kChunk) {
            if (timeUp()) return false;
            done = 0;
        }
        ++done;
        const quint32 id = quint32(m_invalidate.at++);
        Node &n = m_nodes[id];
        if ((n.flags & (Dead | Kept)) || n.state != Done) continue;
        n.state = Dirty;
        ++m_notDone;
        m_dirty.push_back(id);
    }
    m_invalidate = InvalidateJob();
    return true;
}

void Calculation::abandon()
{
    if (!m_running && m_stack.empty() && m_job.phase == GroupJob::None && !m_refuse.active) return;
    // what is waiting or queued is to be worked out again, with what was not yet started
    std::vector<quint32> again;
    // (the formulas of a group that was being closed, or woken, and not yet given back to the stack: to be worked out again too)
    if (m_job.phase != GroupJob::None) {
        const size_t end = m_job.phase == GroupJob::Apply ? m_job.cursor : m_job.open.size();   // (Collect and Sort: all of them are still to be given back)
        for (size_t i = 0; i < end; ++i) {
            Node &n = m_nodes[m_job.open[i]];
            if (n.state == Done) continue;
            n.state = Dirty;
            n.flags &= quint16(~Retried);
            again.push_back(m_job.open[i]);
        }
    }
    m_job = GroupJob();
    m_refuse = RefuseJob();
    m_begin = BeginJob();
    for (const Frame &f : m_stack) {
        Node &n = m_nodes[f.node];
        if (n.state == Waiting || n.state == Queued) {
            n.state = Dirty;
            n.flags &= quint16(~Retried);
            again.push_back(f.node);
        }
    }
    for (const quint32 x : m_comp) {
        Node &n = m_nodes[x];
        m_index[x] = m_low[x] = 0;
        if (n.state == Suspended) {
            n.state = Dirty;
            n.flags &= quint16(~Retried);
            again.push_back(x);
        }
    }
    m_comp.clear();
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
    *work = m_evaluator.work() + kArrayCellSteps * m_evaluator.peakArrayCells();
    return v;
}

// What a formula worked out is: its result, whether it read anything kept apart, and whether it is a mismatch with what the file stored (or reads one).
bool Calculation::finishNode(Node &n, quint32 id, const Value &v)
{
    bool refused = false;
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
    } else if (m_heldBytes - heldBy(n, id) + heldByValue(fitToRange(n, v)) > m_heldLimit) {
        // what the formulas hold would pass its bound: this one has the error (kept apart, the file's value), and the work goes on
        giveError(n, id);
        refused = true;
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
    n.flags &= quint16(~Retried);
    return refused;
}

// The error a formula has that is not worked out for want of steps or room: #VALUE! (kept apart, the file's value, of the class L).
void Calculation::giveError(Node &n, quint32 id)
{
    if (m_options.keepApart) {
        n.flags |= Kept | NodeTainted;
        n.kept = 'L';
        keepStored(n, 'L');
    } else {
        storeResult(n, id, Value::error(ErrorKind::Value));
        n.flags &= quint16(~(Circular | NodeTainted | Mismatch | Cascade));
    }
}

// A formula that is not worked out for want of steps: it has the error, and is done.
void Calculation::refuseNode(quint32 id, Progress &pr)
{
    Node &n = m_nodes[id];
    if ((n.flags & Dead) || n.state == Done) return;
    giveError(n, id);
    n.flags &= quint16(~Retried);
    n.state = Done;
    --m_notDone;
    ++pr.recalculated;
    ++pr.refused;
}

// The recalculation has taken the steps it may: every formula still to be worked out has the error, and the recalculation is over. It is done a chunk at a time (stepRefuse).
void Calculation::refuseTheRest(Progress &pr)
{
    Q_UNUSED(pr);
    m_refuse = RefuseJob();
    m_refuse.active = true;
}

bool Calculation::stepRefuse(Progress &pr, const std::function<bool()> &timeUp)
{
    constexpr size_t kChunk = 256;
    size_t done = 0;
    auto more = [&] {
        if (done++ < kChunk) return true;
        done = 0;
        return !timeUp();
    };
    RefuseJob &j = m_refuse;
    while (j.stackAt < m_stack.size()) {
        if (!more()) return false;
        refuseNode(m_stack[j.stackAt++].node, pr);
    }
    while (j.compAt < m_comp.size()) {
        if (!more()) return false;
        const quint32 x = m_comp[j.compAt++];
        refuseNode(x, pr);
        m_index[x] = m_low[x] = 0;
    }
    if (j.dirtyAt < m_dirtyCursor) j.dirtyAt = m_dirtyCursor;
    while (j.dirtyAt < m_dirty.size()) {
        if (!more()) return false;
        refuseNode(m_dirty[j.dirtyAt++], pr);
    }
    m_stack.clear();
    m_comp.clear();
    m_running = false;
    m_dirty.clear();
    m_dirtyCursor = 0;
    pr.overBudget = true;
    pr.finished = true;
    m_refuse = RefuseJob();
    return true;
}

// A formula in a circle has the result 0 (kept apart, the file's value, which marks what reads it).
void Calculation::markCircular(quint32 id, Progress &pr)
{
    Node &c = m_nodes[id];
    if (m_options.keepApart) {
        // (the file's value of a cell in a circle is what the last calculation left, 0 or another, or what iterating gave: it is kept, and marks what reads it)
        c.flags |= Kept | NodeTainted | Circular;
        c.kept = m_iterate ? 'I' : 'C';
        keepStored(c, c.kept);
    } else {
        storeResult(c, id, Value::number(0));
        c.flags |= Circular;
        c.flags &= quint16(~NodeTainted);
    }
    c.flags &= quint16(~Retried);
    c.state = Done;
    --m_notDone;
    ++pr.recalculated;
    ++pr.circular;
}

// One pass of a radix sort of places of nodes (a circle of a million has a million: three passes of eleven bits, each of which is a step of its own, where a sort that compares takes
// a dozen milliseconds of a slice).
static void radixPass(std::vector<quint32> &v, int shift)
{
    std::vector<quint32> tmp(v.size());
    std::vector<size_t> count(2049, 0);
    for (const quint32 x : v) ++count[((x >> shift) & 2047) + 1];
    for (size_t i = 0; i < 2048; ++i) count[i + 1] += count[i];
    for (const quint32 x : v) tmp[count[(x >> shift) & 2047]++] = x;
    v.swap(tmp);
}

// `root` waits for formulas that wait, directly or through others, for it: the formulas on the group's stack from it up are a group that waits for itself. Each was worked out
// with the formulas of the group not worked out (read as empty), so what one read may be a read it would not make with their values; a group is therefore given a second
// try, each formula worked out again, and one that waits again is in a circle: it has the result 0, as has every other formula of the group that waits again.
// Begins the job of that (a group may be a million formulas; stepGroup does it a chunk at a time, between which the clock is read).
void Calculation::closeGroup(quint32 root, quint32 parent, Progress &pr)
{
    Q_UNUSED(pr);
    m_job = GroupJob();
    m_job.phase = GroupJob::Collect;
    m_job.closing = true;
    m_job.root = root;
    m_job.parent = parent;
}

// `node` is worked out, and the formulas suspended since it began to wait were waiting for it or for a way round it: they are worked out again (a job, as closeGroup's).
void Calculation::leaveGroup(quint32 node, quint32 parent)
{
    m_job = GroupJob();
    m_job.phase = GroupJob::Collect;
    m_job.closing = false;
    m_job.root = node;
    m_job.parent = parent;
}

bool Calculation::stepGroup(Progress &pr, const std::function<bool()> &timeUp, qint64 *spent)
{
    constexpr size_t kChunk = 256;
    GroupJob &j = m_job;
    size_t done = 0;
    auto more = [&] {
        if (done++ < kChunk) return true;
        *spent += qint64(kChunk);
        done = 0;
        return !timeUp();
    };
    if (j.phase == GroupJob::Collect) {
        // the formulas of the group, from the top of its stack down to the one that began it
        while (!m_comp.empty()) {
            if (!more()) return false;
            const quint32 x = m_comp.back();
            m_comp.pop_back();
            m_index[x] = m_low[x] = 0;
            if (!j.closing && x == j.root) break;   // (the formula that is worked out is not woken)
            if (j.closing) ++pr.circleWork;
            const Node &c = m_nodes[x];
            if (c.state != Done) {
                j.open.push_back(x);
                if (j.closing && !(c.flags & Retried)) j.fresh = true;
            }
            if (j.closing && x == j.root) break;
        }
        j.phase = GroupJob::Sort;
        j.sortPass = 0;
    }
    if (j.phase == GroupJob::Sort) {
        if (j.open.size() < 512) {
            std::sort(j.open.begin(), j.open.end());
            j.sortPass = 3;
        }
        while (j.sortPass < 3) {
            if (j.sortPass > 0 && timeUp()) return false;
            radixPass(j.open, 11 * j.sortPass);
            ++j.sortPass;
            *spent += qint64(j.open.size()) / 8;
        }
        j.cursor = j.open.size();
        j.phase = GroupJob::Apply;
        if (j.sortPass > 0 && timeUp()) return false;
    }
    // the formulas of the group, each given back to the stack (the first of them is worked out first), or the result of a circle
    const bool circle = j.closing && !j.fresh;
    while (j.cursor > 0) {
        if (!more()) return false;
        const quint32 x = j.open[--j.cursor];
        if (circle) {
            markCircular(x, pr);
        } else if (j.closing) {
            m_nodes[x].flags |= Retried;
            m_nodes[x].state = Queued;
            m_stack.push_back({x, j.parent});
        } else {
            m_nodes[x].state = Queued;
            m_stack.push_back({x, j.parent});
        }
    }
    m_job = GroupJob();
    return true;
}

Calculation::Progress Calculation::recalculate(qint64 budget, const std::atomic<bool> *cancel, qint64 timeBudgetMicros)
{
    Progress pr;
    qint64 spent = 0;
    const auto started = std::chrono::steady_clock::now();
    const std::atomic<bool> *stop = cancel ? cancel : m_cancel;
    m_ctx.cancel = stop;
    auto stopped = [&] { return stop && stop->load(std::memory_order_relaxed); };
    const std::function<bool()> timeUp = [&] {
        return timeBudgetMicros > 0 && std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started).count() >= timeBudgetMicros;
    };
    // (marking every formula to be worked out again, and the marks a recalculation begins with, are done in slices of the time too)
    if (m_invalidate.active) {
        if (stopped()) {
            pr.cancelled = true;
            return pr;
        }
        if (!stepInvalidate(timeUp)) return pr;
    }
    if (!m_running) beginRecalculation();
    if (m_begin.active) {
        if (stopped()) {
            abandon();
            pr.cancelled = true;
            return pr;
        }
        if (!stepBegin(timeUp)) return pr;
    }
    for (;;) {
        if (stopped()) {
            abandon();
            pr.cancelled = true;
            return pr;
        }
        if (m_refuse.active) {
            stepRefuse(pr, timeUp);
            return pr;
        }
        if (m_job.phase != GroupJob::None) {
            if (!stepGroup(pr, timeUp, &spent)) return pr;
            if (budget > 0 && spent >= budget) return pr;
            continue;
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
            m_stack.push_back({root, kNone});
        }
        if (budget > 0 && spent >= budget) return pr;
        if (pr.evaluated > 0 && timeUp()) return pr;
        const Frame frame = m_stack.back();
        const quint32 id = frame.node;
        {
            const Node &top = m_nodes[id];
            if (top.state == Done || top.state == Suspended || (top.flags & Dead)) {
                m_stack.pop_back();
                continue;
            }
        }
        if (m_workUsed >= m_workLimit) {
            refuseTheRest(pr);
            continue;
        }
        qint64 work = 0;
        const Value v = runFormula(id, &work);
        if (m_evaluator.status() == EvalStatus::Cancelled) {
            // the formula was stopped in the middle: what it gave is of no worth, and it is to be worked out again
            abandon();
            pr.cancelled = true;
            return pr;
        }
        Node &n = m_nodes[id];
        spent += work;
        pr.work += work;
        m_workUsed += work;
        ++pr.evaluated;
        // (what it read through a name, INDIRECT, or OFFSET is told each time it is worked out, whether or not it can be finished: a circle it closes is broken by an edit to that)
        if (n.flags & Told) registerNode(id, &m_touched);
        if (m_pending.empty()) {
            if (finishNode(n, id, v)) ++pr.refused;
            n.state = Done;
            --m_notDone;
            ++pr.recalculated;
            m_stack.pop_back();
            if (m_index[id]) leaveGroup(id, frame.parent);
            continue;
        }
        // it read formulas that are not worked out: those that can be are worked out first, and it again after them
        std::sort(m_pending.begin(), m_pending.end());
        m_pending.erase(std::unique(m_pending.begin(), m_pending.end()), m_pending.end());
        if (!m_index[id]) {
            m_index[id] = m_low[id] = ++m_counter;
            m_comp.push_back(id);
        }
        const size_t before = m_stack.size();
        for (const quint32 y : m_pending) {
            ++pr.circleWork;
            Node &d = m_nodes[y];
            if (y == id || d.state == Waiting || d.state == Suspended) {
                // one it waits for already, or itself, or one suspended: it is in a group with the formulas from there up
                m_low[id] = std::min(m_low[id], m_index[y]);
            } else {
                if (d.state == Dirty) d.state = Queued;
                m_stack.push_back({y, id});
            }
        }
        if (m_stack.size() > before) {
            n.state = Waiting;
            pr.deepest = std::max<qint64>(pr.deepest, qint64(m_stack.size()));
            continue;
        }
        // it waits only for formulas on the way to it, or itself, or those suspended: it cannot be worked out until they are
        m_stack.pop_back();
        if (m_low[id] < m_index[id]) {
            n.state = Suspended;
            if (frame.parent != kNone) m_low[frame.parent] = std::min(m_low[frame.parent], m_low[id]);
            continue;
        }
        closeGroup(id, frame.parent, pr);
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
    m_heldBytes -= heldBy(n, id);
    if (n.kind == KindText && n.text >= 0) freeTextSlot(n.text);
    m_arrays.remove(id);
    m_graph->clearPrecedents(id);
    SheetState &st = m_state[size_t(n.sheet)];
    if (size_t(n.formula) < st.nodeOf.size()) st.nodeOf[size_t(n.formula)] = kNone;
    n.flags = quint16(Dead | (n.flags & AlwaysListed));
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
