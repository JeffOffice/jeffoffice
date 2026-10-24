#include "sheet/refshift.h"

#include <QRegularExpression>

#include <algorithm>

namespace jp::office::sheet {

using jp::calc::kGridCols;
using jp::calc::kGridRows;

StructEdit StructEdit::insertRows(int sheet, quint32 first, quint32 count)
{
    StructEdit e;
    e.kind = Kind::InsertRows;
    e.sheet = sheet;
    e.first = first;
    e.count = count;
    return e;
}
StructEdit StructEdit::deleteRows(int sheet, quint32 first, quint32 count)
{
    StructEdit e = insertRows(sheet, first, count);
    e.kind = Kind::DeleteRows;
    return e;
}
StructEdit StructEdit::insertColumns(int sheet, quint32 first, quint32 count)
{
    StructEdit e = insertRows(sheet, first, count);
    e.kind = Kind::InsertColumns;
    return e;
}
StructEdit StructEdit::deleteColumns(int sheet, quint32 first, quint32 count)
{
    StructEdit e = insertRows(sheet, first, count);
    e.kind = Kind::DeleteColumns;
    return e;
}
static StructEdit cellsEdit(StructEdit::Kind kind, int sheet, const CellRange &b)
{
    StructEdit e;
    e.kind = kind;
    e.sheet = sheet;
    e.row1 = b.row1;
    e.col1 = b.col1;
    e.row2 = b.row2;
    e.col2 = b.col2;
    return e;
}
StructEdit StructEdit::insertCellsDown(int sheet, const CellRange &b) { return cellsEdit(Kind::InsertCellsDown, sheet, b); }
StructEdit StructEdit::deleteCellsUp(int sheet, const CellRange &b) { return cellsEdit(Kind::DeleteCellsUp, sheet, b); }
StructEdit StructEdit::insertCellsRight(int sheet, const CellRange &b) { return cellsEdit(Kind::InsertCellsRight, sheet, b); }
StructEdit StructEdit::deleteCellsLeft(int sheet, const CellRange &b) { return cellsEdit(Kind::DeleteCellsLeft, sheet, b); }
StructEdit StructEdit::move(int sheet, const CellRange &b, int toSheet, quint32 toRow, quint32 toCol)
{
    StructEdit e = cellsEdit(Kind::Move, sheet, b);
    e.toSheet = toSheet;
    e.toRow = toRow;
    e.toCol = toCol;
    return e;
}
StructEdit StructEdit::offset(qint32 dRow, qint32 dCol)
{
    StructEdit e;
    e.kind = Kind::Offset;
    e.dRow = dRow;
    e.dCol = dCol;
    return e;
}

StructEdit StructEdit::inverse() const
{
    StructEdit e = *this;
    switch (kind) {
    case Kind::InsertRows: e.kind = Kind::DeleteRows; break;
    case Kind::DeleteRows: e.kind = Kind::InsertRows; break;
    case Kind::InsertColumns: e.kind = Kind::DeleteColumns; break;
    case Kind::DeleteColumns: e.kind = Kind::InsertColumns; break;
    case Kind::InsertCellsDown: e.kind = Kind::DeleteCellsUp; break;
    case Kind::DeleteCellsUp: e.kind = Kind::InsertCellsDown; break;
    case Kind::InsertCellsRight: e.kind = Kind::DeleteCellsLeft; break;
    case Kind::DeleteCellsLeft: e.kind = Kind::InsertCellsRight; break;
    case Kind::Move: {
        const quint32 rows = row2 - row1, cols = col2 - col1;
        e.sheet = toSheet;
        e.row1 = toRow;
        e.col1 = toCol;
        e.row2 = toRow + rows;
        e.col2 = toCol + cols;
        e.toSheet = sheet;
        e.toRow = row1;
        e.toCol = col1;
        break;
    }
    case Kind::Offset:
        e.dRow = -dRow;
        e.dCol = -dCol;
        break;
    }
    return e;
}

namespace {

// How a span [a, b] of rows (or columns) is after rows [first, first + count) are inserted there.
bool insertSpan(quint32 first, quint32 count, quint32 limit, quint32 *a, quint32 *b)
{
    const quint64 na = *a >= first ? quint64(*a) + count : *a, nb = *b >= first ? quint64(*b) + count : *b;
    if (na >= limit) return false;   // pushed off the edge
    *a = quint32(na);
    *b = quint32(std::min<quint64>(nb, limit - 1));
    return true;
}

// And after they are deleted: false if all of it was in them.
bool deleteSpan(quint32 first, quint32 count, quint32 *a, quint32 *b)
{
    const quint32 end = first + count;   // one past the last deleted
    if (*a >= first && *b < end) return false;
    const quint32 na = *a < first ? *a : (*a < end ? first : *a - count);
    const quint32 nb = *b < first ? *b : (*b < end ? first - 1 : *b - count);
    *a = na;
    *b = nb;
    return true;
}

} // namespace

RefFate adjustBlock(const StructEdit &e, const Block &in, Block *out)
{
    *out = in;
    using K = StructEdit::Kind;
    switch (e.kind) {
    case K::InsertRows:
    case K::DeleteRows: {
        if (in.sheet != e.sheet || in.wholeColumns) return RefFate::Same;
        Block b = in;
        const bool ok = e.kind == K::InsertRows ? insertSpan(e.first, e.count, kGridRows, &b.row1, &b.row2) : deleteSpan(e.first, e.count, &b.row1, &b.row2);
        if (!ok) return RefFate::Gone;
        *out = b;
        return b.row1 == in.row1 && b.row2 == in.row2 ? RefFate::Same : RefFate::Moved;
    }
    case K::InsertColumns:
    case K::DeleteColumns: {
        if (in.sheet != e.sheet || in.wholeRows) return RefFate::Same;
        Block b = in;
        const bool ok = e.kind == K::InsertColumns ? insertSpan(e.first, e.count, kGridCols, &b.col1, &b.col2) : deleteSpan(e.first, e.count, &b.col1, &b.col2);
        if (!ok) return RefFate::Gone;
        *out = b;
        return b.col1 == in.col1 && b.col2 == in.col2 ? RefFate::Same : RefFate::Moved;
    }
    case K::InsertCellsDown:
    case K::DeleteCellsUp: {
        // only a reference whole inside the columns of the block is changed (a reference across other columns could not stay a block)
        if (in.sheet != e.sheet || in.wholeColumns || in.wholeRows || in.col1 < e.col1 || in.col2 > e.col2) return RefFate::Same;
        Block b = in;
        const quint32 count = e.row2 - e.row1 + 1;
        const bool ok = e.kind == K::InsertCellsDown ? insertSpan(e.row1, count, kGridRows, &b.row1, &b.row2) : deleteSpan(e.row1, count, &b.row1, &b.row2);
        if (!ok) return RefFate::Gone;
        *out = b;
        return b.row1 == in.row1 && b.row2 == in.row2 ? RefFate::Same : RefFate::Moved;
    }
    case K::InsertCellsRight:
    case K::DeleteCellsLeft: {
        if (in.sheet != e.sheet || in.wholeColumns || in.wholeRows || in.row1 < e.row1 || in.row2 > e.row2) return RefFate::Same;
        Block b = in;
        const quint32 count = e.col2 - e.col1 + 1;
        const bool ok = e.kind == K::InsertCellsRight ? insertSpan(e.col1, count, kGridCols, &b.col1, &b.col2) : deleteSpan(e.col1, count, &b.col1, &b.col2);
        if (!ok) return RefFate::Gone;
        *out = b;
        return b.col1 == in.col1 && b.col2 == in.col2 ? RefFate::Same : RefFate::Moved;
    }
    case K::Move: {
        // a reference whole inside the block that moves goes with it
        if (in.sheet != e.sheet || in.wholeColumns || in.wholeRows) return RefFate::Same;
        if (in.row1 < e.row1 || in.row2 > e.row2 || in.col1 < e.col1 || in.col2 > e.col2) return RefFate::Same;
        const qint64 dRow = qint64(e.toRow) - e.row1, dCol = qint64(e.toCol) - e.col1;
        if (dRow == 0 && dCol == 0 && e.toSheet == e.sheet) return RefFate::Same;
        Block b = in;
        b.sheet = e.toSheet;
        b.row1 = quint32(qint64(in.row1) + dRow);
        b.row2 = quint32(qint64(in.row2) + dRow);
        b.col1 = quint32(qint64(in.col1) + dCol);
        b.col2 = quint32(qint64(in.col2) + dCol);
        if (b.row2 >= kGridRows || b.col2 >= kGridCols) return RefFate::Gone;
        *out = b;
        return RefFate::Moved;
    }
    case K::Offset:
        return RefFate::Same;
    }
    return RefFate::Same;
}

RefFate adjustCell(const StructEdit &e, int *sheet, quint32 *row, quint32 *col)
{
    Block in;
    in.sheet = *sheet;
    in.row1 = in.row2 = *row;
    in.col1 = in.col2 = *col;
    Block out;
    const RefFate fate = adjustBlock(e, in, &out);
    if (fate == RefFate::Moved) {
        *sheet = out.sheet;
        *row = out.row1;
        *col = out.col1;
        return fate;
    }
    if (fate == RefFate::Gone) return fate;
    // a cell that a Move writes over is gone (the cell that goes there came from the block)
    if (e.kind == StructEdit::Kind::Move && *sheet == e.toSheet) {
        const quint32 rows = e.row2 - e.row1 + 1, cols = e.col2 - e.col1 + 1;
        const bool inDestination = *row >= e.toRow && *row < e.toRow + rows && *col >= e.toCol && *col < e.toCol + cols;
        if (inDestination) return RefFate::Gone;
    }
    return RefFate::Same;
}

bool adjustRange(const StructEdit &e, int sheet, CellRange *range)
{
    if (!range->valid) return true;
    Block in;
    in.sheet = sheet;
    in.row1 = range->row1;
    in.col1 = range->col1;
    in.row2 = range->row2;
    in.col2 = range->col2;
    in.wholeColumns = range->row1 == 0 && range->row2 >= kGridRows - 1;
    in.wholeRows = range->col1 == 0 && range->col2 >= kGridCols - 1;
    Block out;
    const RefFate fate = adjustBlock(e, in, &out);
    if (fate == RefFate::Gone) return false;
    if (fate == RefFate::Moved) {
        range->row1 = out.row1;
        range->col1 = out.col1;
        range->row2 = out.row2;
        range->col2 = out.col2;
    }
    return true;
}

QString adjustSqref(const QString &sqref, const StructEdit &edit, int sheet)
{
    QStringList out;
    for (const QString &part : sqref.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
        CellRange r;
        if (!parseRangeRef(part, &r)) {
            out << part;   // (not a range this reads: kept)
            continue;
        }
        if (!adjustRange(edit, sheet, &r)) continue;
        out << (r.row1 == r.row2 && r.col1 == r.col2 ? cellName(r.row1, r.col1) : cellName(r.row1, r.col1) + QLatin1Char(':') + cellName(r.row2, r.col2));
    }
    return out.join(QLatin1Char(' '));
}

namespace {

int sheetIndexOf(const QStringList &names, const QString &name)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].compare(name, Qt::CaseInsensitive) == 0) return i;
    return -1;
}

// A sheet's name as a formula writes it in front of a reference: in apostrophes when it has anything but letters, digits, and underscores (or starts with a digit).
QString sheetPrefix(const QString &name)
{
    bool plain = !name.isEmpty() && !name.front().isDigit();
    for (const QChar c : name)
        if (!(c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'))) plain = false;
    if (plain) {
        // (a name that reads as a cell, or as R1C1, is quoted too)
        static const QRegularExpression cell(QStringLiteral("^[A-Za-z]{1,3}[0-9]+$|^[Rr][0-9]*[Cc][0-9]*$"));
        if (cell.match(name).hasMatch()) plain = false;
    }
    if (plain) return name + QLatin1Char('!');
    QString q = name;
    q.replace(QLatin1Char('\''), QLatin1String("''"));
    return QLatin1Char('\'') + q + QLatin1String("'!");
}

} // namespace

QString adjustFormula(const QString &formula, int ownSheet, const QStringList &sheetNames, const StructEdit &edit, bool *changed, bool definedName)
{
    if (changed) *changed = false;
    if (edit.kind == StructEdit::Kind::Offset) return offsetFormula(formula, edit.dRow, edit.dCol, changed);
    jp::calc::ParseOptions po;
    po.definedName = definedName;
    std::vector<jp::calc::ReferenceAt> refs;
    if (!jp::calc::findReferences(formula, po, &refs) || refs.empty()) return formula;
    QString out;
    int at = 0;
    bool any = false;
    for (const jp::calc::ReferenceAt &r : refs) {
        int first = ownSheet, last = ownSheet;
        if (!r.sheet1.isEmpty()) {
            first = sheetIndexOf(sheetNames, r.sheet1);
            last = r.sheet2.isEmpty() ? first : sheetIndexOf(sheetNames, r.sheet2);
            if (first < 0 || last < 0) continue;   // (another workbook, or a sheet this does not have)
        }
        if (first > last) std::swap(first, last);
        // a reference through several sheets is changed when the edit is to one of them
        if (edit.sheet < first || edit.sheet > last) continue;
        const int target = edit.sheet;
        Block in;
        in.sheet = target;
        in.row1 = r.ref.row1;
        in.col1 = r.ref.col1;
        in.row2 = r.ref.row2;
        in.col2 = r.ref.col2;
        in.wholeColumns = r.ref.flags & jp::calc::kRefWholeCols;
        in.wholeRows = r.ref.flags & jp::calc::kRefWholeRows;
        Block moved;
        const RefFate fate = adjustBlock(edit, in, &moved);
        if (fate == RefFate::Same) continue;
        any = true;
        // a reference to cells that moved to another sheet names that sheet now (one through several sheets stays as it is)
        const bool otherSheet = fate == RefFate::Moved && moved.sheet != in.sheet && first == last && moved.sheet >= 0 && moved.sheet < sheetNames.size();
        out += formula.mid(at, (otherSheet ? r.start : r.body) - at);
        if (fate == RefFate::Gone) {
            out += QStringLiteral("#REF!");
        } else {
            jp::calc::RefLit lit = r.ref;
            lit.row1 = moved.row1;
            lit.col1 = moved.col1;
            lit.row2 = moved.row2;
            lit.col2 = moved.col2;
            if (otherSheet && moved.sheet != ownSheet) out += sheetPrefix(sheetNames[moved.sheet]);   // (a reference that names no sheet is to the formula's own)
            // (a range that closed up to one cell is written as one cell)
            if (lit.row1 == lit.row2 && lit.col1 == lit.col2 && (r.ref.row1 != r.ref.row2 || r.ref.col1 != r.ref.col2)) lit.flags &= quint8(~jp::calc::kRefArea);
            out += jp::calc::referenceCellsText(lit);
        }
        at = r.end;
    }
    if (!any) return formula;
    out += formula.mid(at);
    if (changed) *changed = true;
    return out;
}

QString offsetFormula(const QString &formula, qint32 dRow, qint32 dCol, bool *changed)
{
    if (changed) *changed = false;
    if ((dRow == 0 && dCol == 0) || formula.isEmpty()) return formula;
    jp::calc::ParseOptions po;
    std::vector<jp::calc::ReferenceAt> refs;
    if (!jp::calc::findReferences(formula, po, &refs) || refs.empty()) return formula;
    QString out;
    int at = 0;
    bool any = false;
    for (const jp::calc::ReferenceAt &r : refs) {
        jp::calc::RefLit lit = r.ref;
        const quint8 f = lit.flags;
        const bool wholeCols = f & jp::calc::kRefWholeCols, wholeRows = f & jp::calc::kRefWholeRows;
        qint64 r1 = lit.row1, r2 = lit.row2, c1 = lit.col1, c2 = lit.col2;
        if (!wholeCols) {
            if (!(f & jp::calc::kRefRow1Abs)) r1 += dRow;
            if (!(f & jp::calc::kRefRow2Abs)) r2 += dRow;
        }
        if (!wholeRows) {
            if (!(f & jp::calc::kRefCol1Abs)) c1 += dCol;
            if (!(f & jp::calc::kRefCol2Abs)) c2 += dCol;
        }
        if (r1 == lit.row1 && r2 == lit.row2 && c1 == lit.col1 && c2 == lit.col2) continue;
        any = true;
        out += formula.mid(at, r.body - at);
        // (a corner that goes off the grid makes the reference #REF!; the corners may cross, and are put in order)
        if (r1 < 0 || r2 < 0 || c1 < 0 || c2 < 0 || r1 >= qint64(kGridRows) || r2 >= qint64(kGridRows) || c1 >= qint64(kGridCols) || c2 >= qint64(kGridCols)) {
            out += QStringLiteral("#REF!");
        } else {
            lit.row1 = quint32(r1);
            lit.row2 = quint32(r2);
            lit.col1 = quint32(c1);
            lit.col2 = quint32(c2);
            if (lit.row1 > lit.row2) {
                std::swap(lit.row1, lit.row2);
                const quint8 a = (lit.flags & jp::calc::kRefRow1Abs) ? 1 : 0, b = (lit.flags & jp::calc::kRefRow2Abs) ? 1 : 0;
                lit.flags = quint8((lit.flags & ~(jp::calc::kRefRow1Abs | jp::calc::kRefRow2Abs)) | (b ? jp::calc::kRefRow1Abs : 0) | (a ? jp::calc::kRefRow2Abs : 0));
            }
            if (lit.col1 > lit.col2) {
                std::swap(lit.col1, lit.col2);
                const quint8 a = (lit.flags & jp::calc::kRefCol1Abs) ? 1 : 0, b = (lit.flags & jp::calc::kRefCol2Abs) ? 1 : 0;
                lit.flags = quint8((lit.flags & ~(jp::calc::kRefCol1Abs | jp::calc::kRefCol2Abs)) | (b ? jp::calc::kRefCol1Abs : 0) | (a ? jp::calc::kRefCol2Abs : 0));
            }
            out += jp::calc::referenceCellsText(lit);
        }
        at = r.end;
    }
    if (!any) return formula;
    out += formula.mid(at);
    if (changed) *changed = true;
    return out;
}

} // namespace jp::office::sheet
