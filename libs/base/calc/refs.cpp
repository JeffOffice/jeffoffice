#include "calc/refs.h"

#include "calc/functions.h"

#include <algorithm>
#include <cstring>
#include <map>

namespace jp::calc {

namespace {

// The functions that skip empty cells and do not count where they are (see blankInsensitiveFunction). SUMIF and its kind are left off: a sum range of another shape is read as large as
// the range, so that a row put inside the one and not the other pairs other cells; COUNTIF counts the empty ones when it is asked to.
const char *const kBlankInsensitive[] = {"SUM",   "SUMSQ", "PRODUCT", "MIN",   "MAX",   "MINA",     "MAXA",   "COUNT", "COUNTA",   "AVERAGE", "AVERAGEA", "MEDIAN",
                                         "LARGE", "SMALL", "STDEV",   "STDEV.S", "STDEVP", "STDEV.P", "VAR", "VAR.S", "VARP", "VAR.P", "SUBTOTAL"};

// The calls that can see where a cell is.
bool observesPositionFunction(const char *name)
{
    static const char *const names[] = {"ROW", "COLUMN", "ADDRESS", "CELL", "OFFSET", "INDIRECT"};
    for (const char *n : names)
        if (std::strcmp(name, n) == 0) return true;
    return false;
}

bool isBlock(const RefLit &r) { return r.row1 != r.row2 || r.col1 != r.col2 || (r.flags & (kRefWholeCols | kRefWholeRows)); }

// Whether every block the program reads goes to a function on the list and nowhere else. The program is postfix, so the stack is followed: for each value, whether it is a block. The
// branches of IF, IFERROR, IFNA, and CHOOSE leave the stack as it was before them, and meet again after them (the block-ness of what each leaves is joined). Anything the walk cannot
// follow (a stack that does not agree, a name, a table, a union) makes the program sensitive.
bool blankInsensitiveProgram(const Program &p)
{
    std::vector<char> st;
    std::map<size_t, std::vector<char>> restore, join;   // by instruction: the stack to go back to (the else of an IF, a branch of CHOOSE), and the stack that joins what falls through
    bool ok = true;
    auto joinAt = [&](size_t at, const std::vector<char> &stack) {
        auto it = join.find(at);
        if (it == join.end()) {
            join.emplace(at, stack);
            return;
        }
        if (it->second.size() != stack.size()) {
            ok = false;
            return;
        }
        for (size_t i = 0; i < stack.size(); ++i) it->second[i] = char(it->second[i] | stack[i]);
    };
    auto pop = [&]() -> char {
        if (st.empty()) {
            ok = false;
            return 0;
        }
        const char top = st.back();
        st.pop_back();
        return top;
    };
    for (size_t i = 0; i < p.code.size() && ok; ++i) {
        const Instr &in = p.code[i];
        if (const auto r = restore.find(i); r != restore.end()) st = r->second;
        if (const auto j = join.find(i); j != join.end()) {
            if (j->second.size() != st.size()) {
                ok = false;
                break;
            }
            for (size_t k = 0; k < st.size(); ++k) st[k] = char(st[k] | j->second[k]);
        }
        switch (in.op) {
        case Op::PushNumber:
        case Op::PushText:
        case Op::PushBool:
        case Op::PushError:
        case Op::PushMissing:
        case Op::PushArray: st.push_back(0); break;
        case Op::PushRef: st.push_back(in.operand >= 0 && size_t(in.operand) < p.refs.size() && isBlock(p.refs[size_t(in.operand)]) ? 1 : 0); break;
        case Op::PushName:
        case Op::PushTable:
        case Op::PushExternal:
        case Op::Range:
        case Op::Union:
        case Op::Intersect:
        case Op::Spill: ok = false; break;
        case Op::Add: case Op::Sub: case Op::Mul: case Op::Div: case Op::Pow: case Op::Concat: case Op::Eq: case Op::Ne: case Op::Lt: case Op::Le: case Op::Gt: case Op::Ge: {
            const char b = pop(), a = pop();
            if (a || b) ok = false;
            st.push_back(0);
            break;
        }
        case Op::Neg:
        case Op::Percent:
        case Op::Single: {
            if (pop()) ok = false;
            st.push_back(0);
            break;
        }
        case Op::Call: {
            std::vector<char> args(in.argc);
            for (size_t k = in.argc; k-- > 0;) args[k] = pop();
            bool block = false;
            for (char a : args) block = block || a;
            if (block) {
                if (in.operand < 0 || (in.flags & kInstrArray)) {
                    ok = false;
                } else {
                    const FunctionInfo &f = functionAt(in.operand);
                    if (!blankInsensitiveFunction(f.name)) ok = false;
                    // (a block is given where the function takes it as it is, and not where it is taken by implicit intersection)
                    for (size_t k = 0; k < args.size() && ok; ++k)
                        if (args[k] && paramClass(f, int(k)) != 'r') ok = false;
                }
            }
            st.push_back(0);
            break;
        }
        case Op::Jump: joinAt(i + 1 + size_t(in.operand), st); break;
        case Op::IfJump: {
            if (pop()) ok = false;
            restore[i + 1 + size_t(in.operand)] = st;
            std::vector<char> failed = st;
            failed.push_back(0);
            joinAt(i + 1 + size_t(in.argc), failed);
            break;
        }
        case Op::TryJump: {
            joinAt(i + 1 + size_t(in.operand), st);   // (the value stays, and the jump goes past the other branch)
            pop();
            break;
        }
        case Op::Choose: {
            if (pop()) ok = false;
            // the next `argc` instructions are jumps to the branches: each branch begins with the stack as it is now
            std::vector<char> failed = st;
            failed.push_back(0);
            joinAt(i + 1 + size_t(in.operand), failed);
            for (size_t k = 1; k <= in.argc && i + k < p.code.size(); ++k) {
                const Instr &to = p.code[i + k];
                if (to.op != Op::Jump) {
                    ok = false;
                    break;
                }
                restore[i + k + 1 + size_t(to.operand)] = st;
            }
            i += in.argc;
            break;
        }
        }
    }
    if (!ok) return false;
    if (const auto j = join.find(p.code.size()); j != join.end()) {
        if (j->second.size() != st.size()) return false;
        for (size_t k = 0; k < st.size(); ++k) st[k] = char(st[k] | j->second[k]);
    }
    return st.size() == 1 && !st.back();   // (a block as the answer is a formula that gives an array, or picks a cell by where it is)
}

} // namespace

bool blankInsensitiveFunction(const char *name)
{
    for (const char *n : kBlankInsensitive)
        if (std::strcmp(name, n) == 0) return true;
    return false;
}

ProgramTraits traitsOf(const Program &program)
{
    ProgramTraits t;
    for (const Instr &in : program.code) {
        if (in.op == Op::PushName || in.op == Op::PushTable) t.usesNames = true;   // (a table's column stands for cells that are found as the formula is worked out)
        else if (in.op == Op::Range) t.reachesBeyond = true;
        else if (in.op == Op::Call && in.argc == 0 && in.operand >= 0) {
            const FunctionInfo &f = functionAt(in.operand);
            if (std::strcmp(f.name, "ROW") == 0 || std::strcmp(f.name, "COLUMN") == 0) t.observesPlace = true;
        }
    }
    for (const auto &call : program.calls) {
        const FunctionInfo &f = functionAt(call.first);
        if (std::strcmp(f.name, "INDIRECT") == 0 || std::strcmp(f.name, "OFFSET") == 0) t.dynamic = true;
        else if (f.flags & kFnVolatile) t.changesByItself = true;
        if (f.flags & kFnIterative) t.iterative = true;
        if (std::strcmp(f.name, "SUMIF") == 0 || std::strcmp(f.name, "AVERAGEIF") == 0) t.reachesBeyond = true;
        if (observesPositionFunction(f.name)) t.observesPosition = true;
    }
    // (a block that is found as the formula is worked out, by a name or by OFFSET or INDIRECT or the range operator, may hold an empty line that nothing here saw)
    t.blankInsensitive = !t.usesNames && !t.dynamic && !t.reachesBeyond && blankInsensitiveProgram(program);
    return t;
}

namespace {

void merge(ProgramTraits *into, const ProgramTraits &from)
{
    into->dynamic = into->dynamic || from.dynamic;
    into->changesByItself = into->changesByItself || from.changesByItself;
    into->iterative = into->iterative || from.iterative;
    into->reachesBeyond = into->reachesBeyond || from.reachesBeyond;
    into->observesPlace = into->observesPlace || from.observesPlace;
    into->observesPosition = into->observesPosition || from.observesPosition;
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
    t.blankInsensitive = false;   // (what a name stands for is not followed for its blocks)
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
