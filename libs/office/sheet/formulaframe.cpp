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

void order(RefLit *lit)
{
    auto flip = [&](quint32 *a, quint32 *b, quint8 abs1, quint8 abs2) {
        if (*a <= *b) return;
        std::swap(*a, *b);
        const bool x = lit->flags & abs1, y = lit->flags & abs2;
        lit->flags = quint8((lit->flags & ~(abs1 | abs2)) | (y ? abs1 : 0) | (x ? abs2 : 0));
    };
    flip(&lit->row1, &lit->row2, jp::calc::kRefRow1Abs, jp::calc::kRefRow2Abs);
    flip(&lit->col1, &lit->col2, jp::calc::kRefCol1Abs, jp::calc::kRefCol2Abs);
}

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
        result += text.mid(at, (m == MapResult::Gone ? r.start : r.body) - at);
        if (m == MapResult::Gone) {
            result += QStringLiteral("#REF!");
        } else {
            order(&lit);
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
            const quint32 a = cells.rowIdAt(lit->row1), b = cells.rowIdAt(lit->row2);
            if (a == CellStore::kNoLine || b == CellStore::kNoLine) return MapResult::Fail;
            lit->row1 = a;
            lit->row2 = b;
            tooWide = tooWide || a >= kGridRows || b >= kGridRows;
        }
        if (!wholeRows) {
            const quint32 a = cells.colIdAt(lit->col1), b = cells.colIdAt(lit->col2);
            if (a == CellStore::kNoLine || b == CellStore::kNoLine) return MapResult::Fail;
            lit->col1 = a;
            lit->col2 = b;
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
        if (!wholeCols) {
            const quint32 a = cells.rowPlaceOf(lit->row1), b = cells.rowPlaceOf(lit->row2);
            if (a == CellStore::kNoLine || b == CellStore::kNoLine) return MapResult::Gone;
            lit->row1 = a;
            lit->row2 = b;
        }
        if (!wholeRows) {
            const quint32 a = cells.colPlaceOf(lit->col1), b = cells.colPlaceOf(lit->col2);
            if (a == CellStore::kNoLine || b == CellStore::kNoLine) return MapResult::Gone;
            lit->col1 = a;
            lit->col2 = b;
        }
        return MapResult::Changed;
    };
    QString out;
    if (!mapReferences(sheets, ownSheet, held, limits, toPlaces, &out)) return held;
    return out;
}

} // namespace jp::office::sheet
