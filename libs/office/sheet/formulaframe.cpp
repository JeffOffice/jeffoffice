#include "sheet/formulaframe.h"

#include "sheet/refshift.h"
#include "sheet/sheetmodel.h"

#include <algorithm>

namespace jp::office::sheet {

using jp::calc::kGridCols;
using jp::calc::kGridRows;
using jp::calc::ParseLimits;
using jp::calc::RefLit;

bool FrameSheets::allPlain() const
{
    for (int i = 0, n = frameSheetCount(); i < n; ++i)
        if (!frameCells(i).isPlain()) return false;
    return true;
}

namespace {

bool sameCells(const RefLit &a, const RefLit &b)
{
    return a.row1 == b.row1 && a.row2 == b.row2 && a.col1 == b.col1 && a.col2 == b.col2 && a.flags == b.flags;
}

} // namespace

bool mapReferences(const FrameSheets &sheets, int ownSheet, const QString &text, const ParseLimits &limits, const RefMapper &map, QString *out, bool *threeD)
{
    *out = text;
    jp::calc::ParseOptions po;
    po.limits = limits;
    std::vector<jp::calc::ReferenceAt> refs;
    if (!jp::calc::findReferences(text, po, &refs)) return false;
    QString result;
    int at = 0;
    bool any = false;
    for (const jp::calc::ReferenceAt &r : refs) {
        if (r.afterWorkbook) continue;   // (the cells of another workbook, whatever its sheet is called)
        int sheet = ownSheet;
        if (!r.sheet1.isEmpty()) {
            sheet = sheets.frameSheetIndex(r.sheet1);
            if (sheet < 0) continue;
            if (!r.sheet2.isEmpty()) {
                const int last = sheets.frameSheetIndex(r.sheet2);
                if (last < 0) continue;
                if (last != sheet) {
                    if (threeD) *threeD = true;
                    continue;
                }
            }
        }
        RefLit lit = r.ref;
        const MapResult m = map(sheet, &lit);
        if (m == MapResult::Fail) return false;
        if (m == MapResult::Same) continue;
        // (a range that closed up to one cell is written as one cell)
        if (m == MapResult::Changed && !(lit.flags & (jp::calc::kRefWholeCols | jp::calc::kRefWholeRows)) && lit.row1 == lit.row2 && lit.col1 == lit.col2 && (r.ref.row1 != r.ref.row2 || r.ref.col1 != r.ref.col2))
            lit.flags &= quint8(~jp::calc::kRefArea);
        result += text.mid(at, r.body - at);   // (what is gone keeps the sheet it was written with: "Sheet2!#REF!")
        if (m == MapResult::Gone) {
            result += QStringLiteral("#REF!");
        } else {
            if (sameCells(lit, r.ref)) {
                result += text.mid(r.body, r.end - r.body);
            } else {
                result += jp::calc::referenceCellsText(lit);
            }
        }
        at = r.end;
        any = true;
    }
    if (any) *out = result + text.mid(at);
    return true;
}

bool nameLooksWide(const QString &name)
{
    // $?LETTERS$?DIGITS with up to 4 letters and up to 8 digits, which the grid's limits do not take for a cell and the wide limits do
    int i = 0;
    const int n = int(name.size());
    if (i < n && name[i] == QLatin1Char('$')) ++i;
    const int l0 = i;
    quint64 col = 0;
    while (i < n && name[i].unicode() < 128 && name[i].isLetter()) {
        col = col * 26 + quint64(name[i].toUpper().unicode() - 'A' + 1);
        ++i;
    }
    if (i == l0 || i - l0 > 5) return false;
    if (i < n && name[i] == QLatin1Char('$')) ++i;
    const int d0 = i;
    quint64 row = 0;
    while (i < n && name[i].unicode() >= '0' && name[i].unicode() <= '9' && i - d0 < 9) {
        row = row * 10 + quint64(name[i].unicode() - '0');
        ++i;
    }
    if (i == d0 || i != n || row < 1) return false;
    const bool grid = col <= kGridCols && row <= kGridRows;
    const bool wide = col <= (quint64(1) << 16) && row <= (quint64(1) << 24);
    return wide && !grid;
}

bool placesToIds(const FrameSheets &sheets, int ownSheet, const QString &typed, QString *held, bool *wide)
{
    *held = typed;
    *wide = false;
    if (sheets.allPlain()) {
        // (every id is its place: the text is as it is, unless it has a reference through several sheets, which the caller finds out from the program)
        return true;
    }
    bool threeD = false;
    bool tooWide = false;
    const auto toIds = [&](int sheet, RefLit *lit) {
        const CellStore &cells = sheets.frameCells(sheet);
        if (cells.isPlain()) return MapResult::Same;
        const bool wholeCols = lit->flags & jp::calc::kRefWholeCols, wholeRows = lit->flags & jp::calc::kRefWholeRows;
        if (!wholeCols) {
            quint32 s0, e0;
            bool sa, ea;
            jp::calc::refEnds(*lit, true, &s0, &e0, &sa, &ea);
            const quint32 a = cells.rowIdAt(s0), b = cells.rowIdAt(e0);
            if (a == CellStore::kNoLine || b == CellStore::kNoLine) return MapResult::Fail;
            jp::calc::setRefEnds(lit, true, a, b, sa, ea);
            tooWide = tooWide || a >= kGridRows || b >= kGridRows;
        }
        if (!wholeRows) {
            quint32 s0, e0;
            bool sa, ea;
            jp::calc::refEnds(*lit, false, &s0, &e0, &sa, &ea);
            const quint32 a = cells.colIdAt(s0), b = cells.colIdAt(e0);
            if (a == CellStore::kNoLine || b == CellStore::kNoLine) return MapResult::Fail;
            jp::calc::setRefEnds(lit, false, a, b, sa, ea);
            tooWide = tooWide || a >= kGridCols || b >= kGridCols;
        }
        return MapResult::Changed;
    };
    if (!mapReferences(sheets, ownSheet, typed, ParseLimits::grid(), toIds, held, &threeD) || threeD) {
        *held = typed;
        return false;
    }
    *wide = tooWide;
    return true;
}

QString idsToPlaces(const FrameSheets &sheets, int ownSheet, const QString &held, const ParseLimits &limits)
{
    if (sheets.allPlain()) return held;
    const auto toPlaces = [&](int sheet, RefLit *lit) {
        const CellStore &cells = sheets.frameCells(sheet);
        if (cells.isPlain()) return MapResult::Same;
        const bool wholeCols = lit->flags & jp::calc::kRefWholeCols, wholeRows = lit->flags & jp::calc::kRefWholeRows;
        // (an end that was deleted is the line next to where it was; a reference with nothing left of it is gone)
        if (!wholeCols) {
            quint32 s0, e0, a = 0, b = 0;
            bool sa, ea;
            jp::calc::refEnds(*lit, true, &s0, &e0, &sa, &ea);
            if (!cells.rowRangeOf(s0, e0, &a, &b)) return MapResult::Gone;
            jp::calc::setRefEnds(lit, true, a, b, sa, ea);
        }
        if (!wholeRows) {
            quint32 s0, e0, a = 0, b = 0;
            bool sa, ea;
            jp::calc::refEnds(*lit, false, &s0, &e0, &sa, &ea);
            if (!cells.colRangeOf(s0, e0, &a, &b)) return MapResult::Gone;
            jp::calc::setRefEnds(lit, false, a, b, sa, ea);
        }
        return MapResult::Changed;
    };
    QString out;
    if (!mapReferences(sheets, ownSheet, held, limits, toPlaces, &out)) return held;
    return out;
}

} // namespace jp::office::sheet
