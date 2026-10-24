#include "sheet/refshift.h"

#include <cstring>

#include "calc/formula.h"

#include <QRegularExpression>

#include <algorithm>
#include <atomic>

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
        if (in.wholeColumns || in.wholeRows) return RefFate::Same;
        const bool inSource = in.sheet == e.sheet && in.row1 >= e.row1 && in.row2 <= e.row2 && in.col1 >= e.col1 && in.col2 <= e.col2;
        if (!inSource) {
            // cells written over by the cells that arrive: a reference to them (whole inside them, and not to cells that go with the block) has no cell to name, as in the programs people
            // know; one that reaches beyond them keeps its cells
            const quint32 toRow2 = e.toRow + (e.row2 - e.row1), toCol2 = e.toCol + (e.col2 - e.col1);
            if (!e.keepsWhatIsWrittenOver && in.sheet == e.toSheet && in.row1 >= e.toRow && in.row2 <= toRow2 && in.col1 >= e.toCol && in.col2 <= toCol2) return RefFate::Gone;
            return RefFate::Same;
        }
        // a reference whole inside the block that moves goes with it
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

bool adjustColumnRun(const StructEdit &e, quint32 *first, quint32 *last)
{
    if (e.kind == StructEdit::Kind::InsertColumns) {
        const quint32 count = e.count;
        if (*first >= e.first) {
            *first += count;
            *last += count;
        } else if (*last >= e.first) {
            *last += count;
        }
        if (*first >= kGridCols) return false;
        *last = std::min(*last, kGridCols - 1);
        return true;
    }
    if (e.kind == StructEdit::Kind::DeleteColumns) {
        const quint32 count = e.count, end = e.first + count;
        if (*last < e.first) return true;
        if (*first >= end) {
            *first -= count;
            *last -= count;
            return true;
        }
        // overlapped: what is left of it
        const quint32 before = *first < e.first ? e.first - *first : 0, after = *last >= end ? *last - end + 1 : 0;
        if (before + after == 0) return false;
        const quint32 start = *first < e.first ? *first : e.first;
        *first = start;
        *last = start + before + after - 1;
        return true;
    }
    return true;
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

namespace {

std::atomic<bool> g_fastRows{true};

bool isUpper(QChar c) { return c.unicode() >= 'A' && c.unicode() <= 'Z'; }
bool isWordChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'); }

// Rows inserted or deleted, in a formula that is only plain references to the cells of its own sheet (A1, $B$2, A1:C9), numbers, operators, brackets, and calls: the row numbers are
// changed where they stand. False (nothing decided) for anything else, or for a reference a deleted row holds, which the full reading handles.
bool fastRowEdit(const QString &f, const StructEdit &e, QString *out, bool *changed)
{
    const bool inserting = e.kind == StructEdit::Kind::InsertRows;
    const int n = int(f.size());
    QString result;
    int copied = 0;
    bool any = false;
    int i = 0;
    while (i < n) {
        const QChar c = f[i];
        const ushort u = c.unicode();
        if (u == '\'' || u == '"' || u == '!' || u == '[' || u == ']' || u == '#' || u == '{' || u == '}' || u == '_' || u == '\\' || u == '@' || u > 126) return false;
        if (c.isDigit()) {
            // a number (1, 2.5, 1E5): or the row numbers of a range of rows (3:5), which the full reading handles
            if (i > 0 && f[i - 1] == QLatin1Char(':')) return false;
            int j = i;
            while (j < n && (f[j].isDigit() || f[j] == QLatin1Char('.'))) ++j;
            if (j < n && (f[j] == QLatin1Char('E') || f[j] == QLatin1Char('e'))) {
                ++j;
                if (j < n && (f[j] == QLatin1Char('+') || f[j] == QLatin1Char('-'))) ++j;
                while (j < n && f[j].isDigit()) ++j;
            }
            if (j < n && f[j] == QLatin1Char(':')) return false;
            if (j < n && isWordChar(f[j])) return false;
            i = j;
            continue;
        }
        if (!(c.isLetter() || u == '$')) {
            ++i;
            continue;
        }
        // a word, or a reference
        const int start = i;
        int j = i;
        const bool dollar1 = f[j] == QLatin1Char('$');
        if (dollar1) ++j;
        const int letters = j;
        while (j < n && f[j].isLetter()) ++j;
        const int letterCount = j - letters;
        bool dollar2 = false;
        if (j < n && f[j] == QLatin1Char('$')) {
            dollar2 = true;
            ++j;
        }
        const int digits = j;
        while (j < n && f[j].isDigit()) ++j;
        const int digitCount = j - digits;
        const bool cellShaped = letterCount >= 1 && letterCount <= 3 && digitCount >= 1 && digitCount <= 7 && (!dollar1 || letters == start + 1);
        const bool endsWord = j >= n || !isWordChar(f[j]);
        const bool startsWord = start == 0 || !isWordChar(f[start - 1]);
        if (cellShaped && endsWord && startsWord && !(j < n && f[j] == QLatin1Char('('))) {
            for (int k = letters; k < letters + letterCount; ++k)
                if (!isUpper(f[k])) return false;
            // (a column past XFD is a name)
            qint64 col = 0;
            for (int k = letters; k < letters + letterCount; ++k) col = col * 26 + (f[k].unicode() - 'A' + 1);
            if (col <= qint64(kGridCols)) {
                const qint64 row0 = f.mid(digits, digitCount).toLongLong() - 1;
                if (row0 >= 0 && row0 < qint64(kGridRows)) {
                    qint64 now = row0;
                    if (inserting) {
                        if (row0 >= qint64(e.first)) now = row0 + e.count;
                    } else if (row0 >= qint64(e.first) + e.count) {
                        now = row0 - e.count;
                    } else if (row0 >= qint64(e.first)) {
                        return false;   // (in the rows deleted)
                    }
                    if (now >= qint64(kGridRows)) return false;
                    if (now != row0) {
                        result += f.mid(copied, digits - copied) + QString::number(now + 1);
                        copied = digits + digitCount;
                        any = true;
                    }
                }
                i = j;
                continue;
            }
        }
        // a word: a name, a function, TRUE, a column (A:C is left to the full reading)
        while (j < n && isWordChar(f[j])) ++j;
        if (start > 0 && f[start - 1] == QLatin1Char(':')) return false;
        if (j < n && f[j] == QLatin1Char(':')) return false;
        i = std::max(j, i + 1);
    }
    *changed = any;
    if (any) *out = result + f.mid(copied);
    return true;
}

} // namespace

void setFastRowEdits(bool on) { g_fastRows = on; }

QString adjustFormula(const QString &formula, int ownSheet, const QStringList &sheetNames, const StructEdit &edit, bool *changed, bool definedName)
{
    if (changed) *changed = false;
    if (edit.kind == StructEdit::Kind::Offset) return offsetFormula(formula, edit.dRow, edit.dCol, changed);
    if (g_fastRows.load(std::memory_order_relaxed) && !definedName && (edit.kind == StructEdit::Kind::InsertRows || edit.kind == StructEdit::Kind::DeleteRows)) {
        if (edit.sheet != ownSheet && !formula.contains(QLatin1Char('!')) && !formula.contains(QLatin1Char('['))) return formula;   // (its own sheet's cells are not the edit's)
        QString now;
        bool did = false;
        if (fastRowEdit(formula, edit, &now, &did)) {
            if (edit.sheet != ownSheet) return formula;
            if (!did) return formula;
            if (changed) *changed = true;
            return now;
        }
    }
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
        // a reference through several sheets names the same cells on each of them: an edit to one of them does not change it, as in the programs people know
        if (first != last) continue;
        const int target = first;
        Block in;
        in.sheet = target;
        in.row1 = r.ref.row1;
        in.col1 = r.ref.col1;
        in.row2 = r.ref.row2;
        in.col2 = r.ref.col2;
        in.wholeColumns = r.ref.flags & jp::calc::kRefWholeCols;
        in.wholeRows = r.ref.flags & jp::calc::kRefWholeRows;
        bool keepRow1 = false, keepRow2 = false, keepCol1 = false, keepCol2 = false;
        if (definedName) {
            // a name's reference that is not fixed (no "$" on its rows, say) is read from the cell the name is used in, and keeps its offset when rows (or columns) are inserted or
            // deleted; the fixed parts follow the cells
            using K = StructEdit::Kind;
            const bool rowEdit = edit.kind == K::InsertRows || edit.kind == K::DeleteRows || edit.kind == K::InsertCellsDown || edit.kind == K::DeleteCellsUp;
            const bool columnEdit = edit.kind == K::InsertColumns || edit.kind == K::DeleteColumns || edit.kind == K::InsertCellsRight || edit.kind == K::DeleteCellsLeft;
            const quint8 f = r.ref.flags;
            const bool area = (f & jp::calc::kRefArea) || r.ref.row1 != r.ref.row2 || r.ref.col1 != r.ref.col2;
            // (which ends of the edited axis are read from the cell: a fixed end follows the cells, a relative one keeps its offset)
            auto fixedEnds = [&](quint8 first, quint8 second, bool *keepFirst, bool *keepSecond) {
                const bool a = f & first, b = area ? bool(f & second) : a;
                *keepFirst = !a;
                *keepSecond = !b;
                return a || b;
            };
            if (rowEdit && !fixedEnds(jp::calc::kRefRow1Abs, jp::calc::kRefRow2Abs, &keepRow1, &keepRow2)) continue;
            if (columnEdit && !fixedEnds(jp::calc::kRefCol1Abs, jp::calc::kRefCol2Abs, &keepCol1, &keepCol2)) continue;
        }
        Block moved;
        RefFate fate = adjustBlock(edit, in, &moved);
        if (fate == RefFate::Moved) {
            if (keepRow1) moved.row1 = in.row1;
            if (keepRow2) moved.row2 = in.row2;
            if (keepCol1) moved.col1 = in.col1;
            if (keepCol2) moved.col2 = in.col2;
            if (moved.sheet == in.sheet && moved.row1 == in.row1 && moved.row2 == in.row2 && moved.col1 == in.col1 && moved.col2 == in.col2) fate = RefFate::Same;
        }
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

struct TextRewriter::Impl {
    struct Ref {
        jp::calc::ReferenceAt at;
        int sheet = -1;      // the place of the sheet it names (the formula's own if none is written); -1: not one of this workbook's
        bool skip = false;   // not changed by an edit: another workbook, or through several sheets
    };
    QString text;
    std::vector<Ref> refs;
    bool usable = false;
};

bool TextRewriter::Key::operator==(const Key &o) const { return std::memcmp(v, o.v, sizeof(v)) == 0; }

quint64 TextRewriter::Key::hash() const
{
    quint64 h = 1469598103934665603ull;
    for (const qint32 x : v) h = (h ^ quint32(x)) * 1099511628211ull;
    return h;
}

TextRewriter::TextRewriter(const QString &text, int ownSheet, const QStringList &sheetNames) : d(std::make_unique<Impl>())
{
    d->text = text;
    jp::calc::ParseOptions po;
    std::vector<jp::calc::ReferenceAt> refs;
    if (!jp::calc::findReferences(text, po, &refs) || refs.size() > size_t(kMaxRefs)) return;
    for (const jp::calc::ReferenceAt &r : refs) {
        Impl::Ref ref;
        ref.at = r;
        ref.sheet = ownSheet;
        if (!r.sheet1.isEmpty()) {
            const int first = sheetIndexOf(sheetNames, r.sheet1);
            const int last = r.sheet2.isEmpty() ? first : sheetIndexOf(sheetNames, r.sheet2);
            ref.sheet = first;
            ref.skip = first < 0 || last < 0 || first != last;
        }
        d->refs.push_back(std::move(ref));
    }
    d->usable = true;
}

TextRewriter::~TextRewriter() = default;
TextRewriter::TextRewriter(TextRewriter &&) noexcept = default;
TextRewriter &TextRewriter::operator=(TextRewriter &&) noexcept = default;

bool TextRewriter::usable() const { return d && d->usable; }

TextRewriter::Fate TextRewriter::fate(const StructEdit &edit, qint32 dRow, qint32 dCol, Key *key) const
{
    using K = StructEdit::Kind;
    *key = Key();
    if (!usable()) return Fate::Unsupported;
    if (edit.kind != K::InsertRows && edit.kind != K::DeleteRows && edit.kind != K::InsertColumns && edit.kind != K::DeleteColumns) return Fate::Unsupported;
    bool any = false;
    for (size_t i = 0; i < d->refs.size(); ++i) {
        const Impl::Ref &r = d->refs[i];
        if (r.skip || r.sheet != edit.sheet) continue;
        const jp::calc::RefLit &lit = r.at.ref;
        const quint8 f = lit.flags;
        const bool wholeCols = f & jp::calc::kRefWholeCols, wholeRows = f & jp::calc::kRefWholeRows;
        // (the cells it names for this cell: the parts that are not fixed move with the cell, as a copy of the formula has them)
        qint64 r1 = lit.row1, r2 = lit.row2, c1 = lit.col1, c2 = lit.col2;
        if (!wholeCols) {
            if (!(f & jp::calc::kRefRow1Abs)) r1 += dRow;
            if (!(f & jp::calc::kRefRow2Abs)) r2 += dRow;
        }
        if (!wholeRows) {
            if (!(f & jp::calc::kRefCol1Abs)) c1 += dCol;
            if (!(f & jp::calc::kRefCol2Abs)) c2 += dCol;
        }
        if (r1 < 0 || r2 < 0 || c1 < 0 || c2 < 0 || r1 >= qint64(kGridRows) || r2 >= qint64(kGridRows) || c1 >= qint64(kGridCols) || c2 >= qint64(kGridCols) || r1 > r2 || c1 > c2) return Fate::Unsupported;
        Block in;
        in.sheet = r.sheet;
        in.row1 = quint32(r1);
        in.row2 = quint32(r2);
        in.col1 = quint32(c1);
        in.col2 = quint32(c2);
        in.wholeColumns = wholeCols;
        in.wholeRows = wholeRows;
        Block moved;
        const RefFate fate = adjustBlock(edit, in, &moved);
        if (fate == RefFate::Same) continue;
        qint32 *k = &key->v[i * 5];
        any = true;
        if (fate == RefFate::Gone) {
            k[0] = 2;
            continue;
        }
        if (moved.sheet != in.sheet) return Fate::Unsupported;
        k[0] = 1;
        k[1] = qint32(qint64(moved.row1) - qint64(in.row1));
        k[2] = qint32(qint64(moved.row2) - qint64(in.row2));
        k[3] = qint32(qint64(moved.col1) - qint64(in.col1));
        k[4] = qint32(qint64(moved.col2) - qint64(in.col2));
    }
    return any ? Fate::Changed : Fate::Same;
}

QString TextRewriter::apply(const Key &key) const
{
    if (!usable()) return QString();
    QString out;
    int at = 0;
    for (size_t i = 0; i < d->refs.size(); ++i) {
        const qint32 *k = &key.v[i * 5];
        if (k[0] == 0) continue;
        const jp::calc::ReferenceAt &a = d->refs[i].at;
        out += d->text.mid(at, a.body - at);
        at = a.end;
        if (k[0] == 2) {
            out += QStringLiteral("#REF!");
            continue;
        }
        jp::calc::RefLit lit = a.ref;
        const qint64 r1 = qint64(lit.row1) + k[1], r2 = qint64(lit.row2) + k[2], c1 = qint64(lit.col1) + k[3], c2 = qint64(lit.col2) + k[4];
        if (r1 < 0 || r2 < r1 || r2 >= qint64(kGridRows) || c1 < 0 || c2 < c1 || c2 >= qint64(kGridCols)) return QString();
        lit.row1 = quint32(r1);
        lit.row2 = quint32(r2);
        lit.col1 = quint32(c1);
        lit.col2 = quint32(c2);
        // (a range that closed up to one cell is written as one cell)
        if (lit.row1 == lit.row2 && lit.col1 == lit.col2 && (a.ref.row1 != a.ref.row2 || a.ref.col1 != a.ref.col2)) lit.flags &= quint8(~jp::calc::kRefArea);
        out += jp::calc::referenceCellsText(lit);
    }
    out += d->text.mid(at);
    return out;
}

QString adjustMovedFormula(const QString &formula, const QStringList &sheetNames, const StructEdit &move, bool *changed)
{
    StructEdit own = move;
    own.keepsWhatIsWrittenOver = true;
    if (move.kind != StructEdit::Kind::Move || move.toSheet == move.sheet || move.sheet < 0 || move.sheet >= sheetNames.size() || move.toSheet < 0 || move.toSheet >= sheetNames.size())
        return adjustFormula(formula, move.sheet, sheetNames, own, changed);
    jp::calc::ParseOptions po;
    std::vector<jp::calc::ReferenceAt> refs;
    QString text = formula;
    bool namesDestination = false;
    if (jp::calc::findReferences(formula, po, &refs) && !refs.empty()) {
        // (references that name no sheet and are not to cells that go with the block are to the sheet it came from: they say so)
        const QString prefix = sheetPrefix(sheetNames[move.sheet]);
        QString out;
        int at = 0;
        for (const jp::calc::ReferenceAt &r : refs) {
            if (!r.sheet1.isEmpty()) {
                if (sheetIndexOf(sheetNames, r.sheet1) == move.toSheet || (!r.sheet2.isEmpty() && sheetIndexOf(sheetNames, r.sheet2) == move.toSheet)) namesDestination = true;
                continue;
            }
            const bool whole = r.ref.flags & (jp::calc::kRefWholeCols | jp::calc::kRefWholeRows);
            if (!whole && r.ref.row1 >= move.row1 && r.ref.row2 <= move.row2 && r.ref.col1 >= move.col1 && r.ref.col2 <= move.col2) continue;   // (it goes with the cells)
            out += formula.mid(at, r.body - at) + prefix;
            at = r.body;
        }
        out += formula.mid(at);
        text = out;
    }
    QString moved = adjustFormula(text, move.sheet, sheetNames, own);
    if (!namesDestination) {
        // (the references to cells that went with the block name the sheet they went to, which is the formula's own now: they need not say so)
        refs.clear();
        if (jp::calc::findReferences(moved, po, &refs) && !refs.empty()) {
            QString out;
            int at = 0;
            for (const jp::calc::ReferenceAt &r : refs) {
                if (r.sheet1.isEmpty() || !r.sheet2.isEmpty() || sheetIndexOf(sheetNames, r.sheet1) != move.toSheet) continue;
                out += moved.mid(at, r.start - at);
                at = r.body;
            }
            out += moved.mid(at);
            moved = out;
        }
    }
    if (changed) *changed = moved != formula;
    return moved;
}

QString adjustRuleFormula(const QString &formula, int ownSheet, const QStringList &sheetNames, const StructEdit &edit, const CellRange &firstRange, bool *changed)
{
    qint32 dRow = 0, dCol = 0;
    if (firstRange.valid && edit.sheet == ownSheet && edit.kind != StructEdit::Kind::Move && edit.kind != StructEdit::Kind::Offset) {
        CellRange now = firstRange;
        if (adjustRange(edit, ownSheet, &now)) {
            // (where the cell that is the first now was before: the edit undone, for the cell)
            int sheet = ownSheet;
            quint32 row = now.row1, col = now.col1;
            if (adjustCell(edit.inverse(), &sheet, &row, &col) != RefFate::Gone) {
                dRow = qint32(row) - qint32(firstRange.row1);
                dCol = qint32(col) - qint32(firstRange.col1);
            }
        }
    }
    QString text = dRow || dCol ? offsetFormula(formula, dRow, dCol) : formula;
    text = adjustFormula(text, ownSheet, sheetNames, edit);
    if (changed) *changed = text != formula;
    return text;
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
