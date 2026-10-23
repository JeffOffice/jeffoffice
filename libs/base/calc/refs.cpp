#include "calc/refs.h"

#include "calc/functions.h"

#include <algorithm>
#include <cstring>

namespace jp::calc {

ProgramTraits traitsOf(const Program &program)
{
    ProgramTraits t;
    for (const Instr &in : program.code)
        if (in.op == Op::PushName) {
            t.usesNames = true;
            break;
        }
    for (const auto &call : program.calls) {
        const FunctionInfo &f = functionAt(call.first);
        if (std::strcmp(f.name, "INDIRECT") == 0 || std::strcmp(f.name, "OFFSET") == 0) t.dynamic = true;
        else if (f.flags & kFnVolatile) t.changesByItself = true;
    }
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
