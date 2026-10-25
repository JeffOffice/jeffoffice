#include "core/auditing.h"

#include "calc/formula.h"

#include <algorithm>
#include <atomic>
#include <set>
#include <tuple>

namespace jp::crunch {

using jp::office::sheet::CellRange;

namespace {

struct Walk {
    const jp::office::sheet::Calculation &calc;
    std::vector<TraceBlock> *out;
    bool truncated = false;
    int names = 0;

    void add(int sheet, quint32 r1, quint32 c1, quint32 r2, quint32 c2)
    {
        if (int(out->size()) >= kMaxTraceBlocks) {
            truncated = true;
            return;
        }
        TraceBlock b;
        b.sheet = sheet;
        b.cells.valid = true;
        b.cells.row1 = std::min(r1, r2);
        b.cells.row2 = std::max(r1, r2);
        b.cells.col1 = std::min(c1, c2);
        b.cells.col2 = std::max(c1, c2);
        out->push_back(b);
    }

    void formula(const QString &text, int sheet, bool isName, int depth)
    {
        jp::calc::ParseOptions po;
        po.definedName = isName;
        const jp::calc::ParseResult parsed = jp::calc::parseFormula(text, po);
        if (!parsed.ok()) return;
        const jp::calc::Program &p = *parsed.program;
        for (const jp::calc::Instr &in : p.code) {
            if (in.op == jp::calc::Op::PushRef && in.operand >= 0 && size_t(in.operand) < p.refs.size()) {
                const jp::calc::RefLit &r = p.refs[size_t(in.operand)];
                int first = sheet, last = sheet;
                if (r.sheet1 >= 0 && r.sheet1 < p.sheetNames.size()) first = last = calc.sheetIndex(p.sheetNames[r.sheet1]);
                if (r.sheet2 >= 0 && r.sheet2 < p.sheetNames.size()) last = calc.sheetIndex(p.sheetNames[r.sheet2]);
                if (first < 0 || last < 0) continue;   // (a sheet that is not there: #REF!)
                for (int s = std::min(first, last); s <= std::max(first, last) && s < calc.sheetCount(); ++s) add(s, r.row1, r.col1, r.row2, r.col2);
            } else if (in.op == jp::calc::Op::PushName && in.operand >= 0 && size_t(in.operand) < p.texts.size() && depth < kMaxTraceNameDepth) {
                if (++names > kMaxTraceNames) {
                    truncated = true;
                    continue;
                }
                const QString name = p.texts[size_t(in.operand)];
                int scope = sheet;
                if (in.argc > 0 && size_t(in.argc - 1) < size_t(p.sheetNames.size())) scope = calc.sheetIndex(p.sheetNames[in.argc - 1]);
                QString text;
                if (scope >= 0 && calc.nameText(scope, name, &text)) formula(text, scope, true, depth + 1);
                else if (calc.nameText(-1, name, &text)) formula(text, sheet, true, depth + 1);
            }
        }
    }
};

} // namespace

std::vector<TraceBlock> cellsRead(const jp::office::sheet::Calculation &calc, int sheet, quint32 row, quint32 col, bool *truncated)
{
    std::vector<TraceBlock> out;
    if (truncated) *truncated = false;
    if (sheet < 0 || sheet >= calc.sheetCount()) return out;
    const jp::office::sheet::Calculation::CellContent c = calc.content(sheet, row, col);
    if (!c.exists || !c.isFormula || c.formula.isEmpty()) return out;
    Walk w{calc, &out};
    w.formula(c.formula, sheet, false, 0);
    if (truncated) *truncated = w.truncated;
    return out;
}

std::vector<TraceCell> readersOf(const jp::office::sheet::Calculation &calc, int sheet, quint32 row, quint32 col, qint64 cap, qint64 *total)
{
    std::vector<TraceCell> out;
    std::vector<quint32> nodes;
    // (the graph is told to stop at once: it ends the answer after a few thousand formulas, so that a cell that two hundred thousand formulas read costs no more than one that four thousand do)
    static const std::atomic<bool> enough{true};
    const bool whole = calc.graph().dependentsOfCell(sheet, row, col, &nodes, &enough);
    std::vector<TraceCell> all;
    all.reserve(nodes.size());
    for (const quint32 node : nodes) {
        TraceCell c;
        if (calc.formulaPlace(node, &c.sheet, &c.row, &c.col)) all.push_back(c);
    }
    // (only the first of them are put in order: a cell that two hundred thousand formulas read is not sorted whole on the window's thread; twice the bound is kept, for the repeats)
    const qint64 readers = qint64(all.size());
    const size_t keep = std::min<size_t>(all.size(), size_t(std::max<qint64>(1, cap)) * 2);
    auto before = [](const TraceCell &a, const TraceCell &b) { return std::tie(a.sheet, a.row, a.col) < std::tie(b.sheet, b.row, b.col); };
    std::partial_sort(all.begin(), all.begin() + std::ptrdiff_t(keep), all.end(), before);
    all.resize(keep);
    all.erase(std::unique(all.begin(), all.end(), [](const TraceCell &a, const TraceCell &b) { return a.sheet == b.sheet && a.row == b.row && a.col == b.col; }), all.end());
    if (total) *total = !whole ? std::max<qint64>(readers, cap + 1) : (keep < size_t(readers) ? readers : qint64(all.size()));
    for (size_t i = 0; i < all.size() && qint64(i) < cap; ++i) out.push_back(all[i]);
    return out;
}

} // namespace jp::crunch
