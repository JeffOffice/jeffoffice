#include "calc/refs.h"

#include "calc/functions.h"

#include <algorithm>
#include <cstring>

namespace jp::calc {

ProgramTraits traitsOf(const Program &program)
{
    ProgramTraits t;
    for (const Instr &in : program.code) {
        if (in.op == Op::PushName || in.op == Op::PushTable) t.usesNames = true;   // (a table's column stands for cells that are found as the formula is worked out)
        else if (in.op == Op::Range) t.reachesBeyond = true;
    }
    for (const auto &call : program.calls) {
        const FunctionInfo &f = functionAt(call.first);
        if (std::strcmp(f.name, "INDIRECT") == 0 || std::strcmp(f.name, "OFFSET") == 0) t.dynamic = true;
        else if (f.flags & kFnVolatile) t.changesByItself = true;
        if (f.flags & kFnIterative) t.iterative = true;
        if (std::strcmp(f.name, "SUMIF") == 0 || std::strcmp(f.name, "AVERAGEIF") == 0) t.reachesBeyond = true;
    }
    return t;
}

namespace {

void merge(ProgramTraits *into, const ProgramTraits &from)
{
    into->dynamic = into->dynamic || from.dynamic;
    into->changesByItself = into->changesByItself || from.changesByItself;
    into->iterative = into->iterative || from.iterative;
    into->reachesBeyond = into->reachesBeyond || from.reachesBeyond;
}

// The traits of the names the program uses (not of the program itself), added to `into`.
void followNames(const Program &p, const std::vector<qint32> &bound, int sheet, const CellSource &source, NameTraitsCache *cache, int depth, ProgramTraits *into)
{
    for (const Instr &in : p.code) {
        if (in.op != Op::PushName) continue;
        int s = sheet;
        if (in.argc > 0) {
            s = size_t(in.argc) - 1 < bound.size() ? bound[size_t(in.argc) - 1] : -1;
            if (s < 0) continue;
        }
        const QString &name = p.texts[size_t(in.operand)];
        const QString formula = source.definedName(s, name);
        if (formula.isEmpty()) continue;
        // (a name is known by its scope, as the evaluation knows it)
        const QString key = QString::number(formula == source.definedName(-1, name) ? -1 : s) + QLatin1Char('|') + name.toUpper();
        if (cache->contains(key)) {
            merge(into, cache->value(key));
            continue;
        }
        cache->insert(key, ProgramTraits());   // (a name that reads itself, through others or not, adds nothing the second time)
        if (depth >= kMaxNameDepth) continue;
        const ParseResult parsed = parseFormula(formula, ParseOptions());
        if (!parsed.program) continue;
        ProgramTraits t = traitsOf(*parsed.program);
        followNames(*parsed.program, bindSheetNames(*parsed.program, source), s, source, cache, depth + 1, &t);
        cache->insert(key, t);
        merge(into, t);
    }
}

} // namespace

ProgramTraits traitsFollowingNames(const Program &program, const std::vector<qint32> &boundSheets, int sheet, const CellSource &source, NameTraitsCache *cache)
{
    ProgramTraits t = traitsOf(program);
    if (!t.usesNames) return t;
    NameTraitsCache own;
    followNames(program, boundSheets, sheet, source, cache ? cache : &own, 0, &t);
    return t;
}

std::vector<qint32> bindSheetNames(const Program &program, const CellSource &source)
{
    std::vector<qint32> bound(size_t(program.sheetNames.size()), -1);
    for (int i = 0; i < program.sheetNames.size(); ++i) bound[size_t(i)] = source.sheetIndex(program.sheetNames[i]);
    return bound;
}

void resolveReferences(const Program &p, const std::vector<qint32> &bound, int sheetCount, int sheet, int shiftRows, int shiftCols, std::vector<CellArea> *out)
{
    for (const RefLit &r : p.refs) {
        qint32 s1 = r.sheet1 >= 0 ? bound[size_t(r.sheet1)] : sheet;
        qint32 s2 = r.sheet2 >= 0 ? bound[size_t(r.sheet2)] : s1;
        if (s1 < 0 || s2 < 0 || s1 >= sheetCount || s2 >= sheetCount) continue;
        if (s2 < s1) std::swap(s1, s2);
        qint64 r1 = (r.flags & kRefRow1Abs) ? qint64(r.row1) : qint64(r.row1) + shiftRows;
        qint64 r2 = (r.flags & kRefRow2Abs) ? qint64(r.row2) : qint64(r.row2) + shiftRows;
        qint64 c1 = (r.flags & kRefCol1Abs) ? qint64(r.col1) : qint64(r.col1) + shiftCols;
        qint64 c2 = (r.flags & kRefCol2Abs) ? qint64(r.col2) : qint64(r.col2) + shiftCols;
        if (r1 > r2) std::swap(r1, r2);
        if (c1 > c2) std::swap(c1, c2);
        if (r1 < 0 || r2 >= qint64(kGridRows) || c1 < 0 || c2 >= qint64(kGridCols)) continue;
        CellArea a;
        a.sheet1 = s1;
        a.sheet2 = s2;
        a.row1 = quint32(r1);
        a.row2 = quint32(r2);
        a.col1 = quint32(c1);
        a.col2 = quint32(c2);
        out->push_back(a);
    }
}

} // namespace jp::calc
