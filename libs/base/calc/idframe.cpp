#include "calc/idframe.h"

#include "calc/formula.h"

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
