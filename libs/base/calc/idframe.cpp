#include "calc/idframe.h"

#include "calc/formula.h"

#include <utility>

namespace jp::calc {

IdSpan idSpan(const LineMapCore &map, quint32 first, quint32 last, quint32 lastPlace, quint32 maxId, bool reading)
{
    constexpr quint32 kNone = LineMapCore::kNone;
    IdSpan s;
    if (reading && first == 0) {
        s.lo = 0;
    } else {
        s.lo = map.origBefore(first);
        if (s.lo == kNone) s.lo = 0;
    }
    if (reading && last >= lastPlace) {
        s.hi = maxId;
    } else {
        s.hi = map.origAfter(last);
        if (s.hi == kNone) s.hi = maxId;
    }
    return s;
}

bool idSpanOfEnds(const LineMapCore &map, qint64 startId, qint64 endId, quint32 maxId, IdSpan *out)
{
    constexpr quint32 kNone = LineMapCore::kNone;
    if (startId < 0 || endId < 0 || startId > qint64(maxId) || endId > qint64(maxId)) return false;
    quint32 a = quint32(startId), b = quint32(endId);
    const quint32 firstNew = map.limits().firstNew;
    // (the ends are in the order of the places, but a reference written before ids is put in the order of its ids; both alive and the other way round is the same range)
    const quint32 pa = map.posOf(a), pb = map.posOf(b);
    if (pa != kNone && pb != kNone && pa > pb) std::swap(a, b);
    auto lineOf = [&](quint32 id, bool start) -> quint32 {
        if (map.posOf(id) != kNone) return id;
        return start ? map.resolveStart(id) : map.resolveEnd(id);
    };
    if (a < firstNew) {
        out->lo = a;
    } else {
        const quint32 l = lineOf(a, true);
        const quint32 o = l == kNone ? kNone : map.origBefore(map.posOf(l));
        out->lo = o == kNone ? 0 : o;
    }
    if (b < firstNew) {
        out->hi = b;
    } else {
        const quint32 l = lineOf(b, false);
        const quint32 o = l == kNone ? kNone : map.origAfter(map.posOf(l));
        out->hi = o == kNone ? maxId : o;
    }
    if (out->lo > out->hi) std::swap(out->lo, out->hi);
    return true;
}

CellArea idArea(const RowMap &rows, const ColMap &cols, const CellArea &places, bool reading)
{
    CellArea a = places;
    const IdSpan r = idSpan(rows.core(), places.row1, places.row2, kGridRows - 1, kMaxRowId, reading);
    const IdSpan c = idSpan(cols.core(), places.col1, places.col2, kGridCols - 1, kMaxColId, reading);
    a.row1 = r.lo;
    a.row2 = r.hi;
    a.col1 = c.lo;
    a.col2 = c.hi;
    return a;
}

IdGap idGap(const LineMapCore &map, quint32 afterPlace)
{
    IdGap g;
    if (afterPlace > 0) g.before = map.origBefore(afterPlace - 1);
    g.after = map.origAfter(afterPlace);
    return g;
}

} // namespace jp::calc
