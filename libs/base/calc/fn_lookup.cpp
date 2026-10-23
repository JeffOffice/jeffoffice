// The lookup and reference functions. A lookup in sorted data (the approximate mode of VLOOKUP, HLOOKUP, MATCH, and LOOKUP) halves the range, as the original does,
// so what it finds in data that is not in order is what the halving meets; a cell of another type than the value looked for is passed over.
#include "calc/machine.h"

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

// ---- comparing for a lookup

// -1, 0, or 1 as the value looked for is before, equal to, or after the cell's; 2 if they are of different kinds (or the cell is an error or empty).
int compareForLookup(const Value &key, const Value &cell)
{
    if (key.isNumber() && cell.isNumber()) {
        // exactly: a lookup does not take two numbers for the same because they agree to 15 digits, as the = operator does
        if (key.number() == cell.number()) return 0;
        return key.number() < cell.number() ? -1 : 1;
    }
    if (key.isText() && cell.isText()) return compareText(key.text(), cell.text());
    if (key.isBool() && cell.isBool()) return key.boolean() == cell.boolean() ? 0 : (key.boolean() ? 1 : -1);
    return 2;
}

// Does the cell equal the value looked for (for an exact match: a text with ? and * in it is a pattern)?
bool matchesExactly(const Value &key, const Value &cell, bool wildcards)
{
    if (wildcards && key.isText() && cell.isText()) return wildcardMatch(cell.text(), 0, key.text(), 0);
    return compareForLookup(key, cell) == 0;
}

// The position of the last cell of a column or row of n not after (ascending) or not before (descending) the value, found by halving; -1 for none, and -2 if the
// work gave out. `at` gives cell i. Each cell looked at counts as a step.
template <class At>
qint64 searchSorted(Call &c, const Value &key, qint64 n, bool descending, At at)
{
    qint64 lo = 0, hi = n - 1, result = -1;
    Machine &m = c.machine();
    while (lo <= hi) {
        qint64 mid = lo + (hi - lo) / 2;
        // a cell of another kind is passed over: the next one that can be compared, up to hi, else down from mid
        qint64 j = mid;
        int order = 2;
        while (j <= hi && (order = compareForLookup(key, at(j))) == 2) {
            ++j;
            if (!m.charge(1)) return -2;
        }
        if (j > hi) {
            j = mid - 1;
            while (j >= lo && (order = compareForLookup(key, at(j))) == 2) {
                --j;
                if (!m.charge(1)) return -2;
            }
            if (j < lo) break;
            // everything after j in the range is passed over
            const bool notAfter = descending ? order <= 0 : order >= 0;
            if (notAfter) {
                result = j;
                break;
            }
            hi = j - 1;
            continue;
        }
        if (!m.charge(1)) return -2;
        const bool notAfter = descending ? order <= 0 : order >= 0;   // the cell is not after (not before) the value
        if (notAfter) {
            result = j;
            lo = j + 1;
        } else {
            hi = mid - 1;
        }
    }
    return result;
}

// The first cell of a column or row of n equal to the value, in the order of the sheet; -1 for none.
template <class At>
qint64 searchExact(Call &c, const Value &key, qint64 n, At at)
{
    const bool wildcards = key.isText() && hasWildcards(key.text());
    for (qint64 i = 0; i < n; ++i) {
        if (!c.machine().charge(1)) return -2;
        if (matchesExactly(key, at(i), wildcards)) return i;
    }
    return -1;
}

// The value looked for: an empty one is 0; an error is the result.
bool lookupKey(Call &c, int i, Value *key)
{
    *key = c.scalar(i);
    if (key->isError()) {
        c.error(key->errorKind());
        return false;
    }
    if (key->isEmpty()) *key = Value::number(0);
    return true;
}

// ---- VLOOKUP, HLOOKUP

Value tableLookup(Call &c, bool vertical)
{
    Value key;
    Table t;
    double index;
    if (!lookupKey(c, 0, &key) || !c.table(1, &t) || !c.number(2, &index)) return c.failure();
    bool exact = false;
    if (c.argc() > 3) {
        if (c.item(3).isMissing()) {
            exact = true;
        } else {
            bool approx;
            if (!c.boolean(3, &approx)) return c.failure();
            exact = !approx;
        }
    }
    index = std::trunc(index);
    if (index < 1) return c.error(ErrorKind::Value);
    const qint64 span = vertical ? t.fullCols() : t.fullRows();   // the columns of the table (rows for HLOOKUP)
    if (index > double(span)) return c.error(ErrorKind::Ref);
    const qint64 n = vertical ? t.rows() : t.cols();
    auto at = [&](qint64 i) { return vertical ? t.at(i, 0) : t.at(0, i); };
    qint64 found;
    if (exact) {
        found = searchExact(c, key, n, at);
        if (found == -2) return c.error(ErrorKind::Value);
    } else {
        found = searchSorted(c, key, n, false, at);
        if (found == -2) return c.error(ErrorKind::Value);
    }
    if (found < 0) return c.error(ErrorKind::NA);
    return vertical ? t.at(found, qint64(index) - 1) : t.at(qint64(index) - 1, found);
}
Value fnVLookup(Call &c) { return tableLookup(c, true); }
Value fnHLookup(Call &c) { return tableLookup(c, false); }

// ---- MATCH, LOOKUP

Value fnMatch(Call &c)
{
    Value key;
    Table t;
    double type;
    if (!lookupKey(c, 0, &key)) return c.failure();
    if (!c.table(1, &t) || !c.numberOr(2, 1, &type)) return c.failure();
    if (c.argc() > 2 && c.item(2).isMissing()) type = 0;   // (an empty argument is not an argument left off: it is 0)
    if (t.fullRows() != 1 && t.fullCols() != 1) return c.error(ErrorKind::NA);
    const bool column = t.fullCols() == 1;
    const qint64 n = column ? t.rows() : t.cols();
    auto at = [&](qint64 i) { return column ? t.at(i, 0) : t.at(0, i); };
    qint64 found;
    if (type == 0) {
        found = searchExact(c, key, n, at);
        if (found == -2) return c.error(ErrorKind::Value);
    } else {
        found = searchSorted(c, key, n, type < 0, at);
        if (found == -2) return c.error(ErrorKind::Value);
    }
    if (found < 0) return c.error(ErrorKind::NA);
    return Value::number(double(found + 1));
}

Value fnLookup(Call &c)
{
    Value key;
    Table t;
    if (!lookupKey(c, 0, &key) || !c.table(1, &t)) return c.failure();
    Table result;
    qint64 n;
    bool vectorColumn;
    const bool hasResult = c.argc() > 2 && !c.missing(2);
    if (hasResult) {
        if (!c.table(2, &result, false)) return c.failure();
        if (t.fullRows() != 1 && t.fullCols() != 1) return c.error(ErrorKind::NA);
        vectorColumn = t.fullCols() == 1;
        n = vectorColumn ? t.rows() : t.cols();
    } else if (t.fullRows() == 1 || t.fullCols() == 1) {
        vectorColumn = t.fullCols() == 1;
        n = vectorColumn ? t.rows() : t.cols();
    } else {
        // an array: its first row or column is searched, and the value is in the last row or column
        vectorColumn = t.fullCols() <= t.fullRows();
        n = vectorColumn ? t.rows() : t.cols();
    }
    auto at = [&](qint64 i) { return vectorColumn ? t.at(i, 0) : t.at(0, i); };
    const qint64 found = searchSorted(c, key, n, false, at);
    if (found == -2) return c.error(ErrorKind::Value);
    if (found < 0) return c.error(ErrorKind::NA);
    if (hasResult) {
        const qint64 count = result.fullRows() * result.fullCols();
        if (found >= count) return c.error(ErrorKind::NA);
        return result.fullCols() == 1 ? result.at(found, 0) : result.at(0, found);
    }
    if (t.fullRows() == 1 || t.fullCols() == 1) return at(found);
    return vectorColumn ? t.at(found, t.fullCols() - 1) : t.at(t.fullRows() - 1, found);
}

// ---- INDEX, CHOOSE, OFFSET, INDIRECT

Value fnIndex(Call &c)
{
    Machine &m = c.machine();
    double rowArg = 0, colArg = 0;
    const bool haveRow = c.argc() > 1 && !c.missing(1);
    const bool haveCol = c.argc() > 2 && !c.missing(2);
    if ((haveRow && !c.number(1, &rowArg)) || (haveCol && !c.number(2, &colArg))) return c.failure();
    rowArg = std::trunc(rowArg);
    colArg = std::trunc(colArg);
    if (rowArg < 0 || colArg < 0) return c.error(ErrorKind::Value);
    const Item &first = c.item(0);
    if (first.isMissing()) return c.error(ErrorKind::Value);
    double areaArg = 1;
    if (c.argc() > 3 && !c.missing(3) && !c.number(3, &areaArg)) return c.failure();
    qint64 rows, cols;
    RefArea area;
    Value array;
    const bool isRef = first.isRef();
    if (isRef) {
        const int ai = int(std::trunc(areaArg));
        if (ai < 1 || ai > first.areaCount()) return c.error(ErrorKind::Ref);
        area = first.areaAt(ai - 1);
        if (area.sheet1 != area.sheet2) return c.error(ErrorKind::Value);
        rows = area.rows();
        cols = area.cols();
    } else {
        array = first.value;
        if (array.isError()) return array;
        rows = array.rows();
        cols = array.cols();
    }
    // with one index and a single row or column of cells the index counts along it
    if (!haveCol && c.argc() <= 3) {
        if (rows == 1 && cols > 1) {
            colArg = rowArg;
            rowArg = 0;
            if (!haveRow) colArg = 0;
        } else if (cols == 1 || (rows == 1 && cols == 1)) {
            // rowArg counts down the column
        }
    }
    if (rowArg > double(rows) || colArg > double(cols)) return c.error(ErrorKind::Ref);
    const qint64 r = qint64(rowArg), col = qint64(colArg);
    if (isRef) {
        RefArea out = area;
        if (r > 0) out.row1 = out.row2 = quint32(qint64(area.row1) + r - 1);
        if (col > 0) out.col1 = out.col2 = quint32(qint64(area.col1) + col - 1);
        return c.returnRef(Item::ofRef(out));
    }
    if (r > 0 && col > 0) return array.at(int(r - 1), int(col - 1));
    const int outRows = r > 0 ? 1 : int(rows), outCols = col > 0 ? 1 : int(cols);
    if (outRows == 1 && outCols == 1) return array.at(r > 0 ? int(r - 1) : 0, col > 0 ? int(col - 1) : 0);
    Value out = Value::array(outRows, outCols);
    for (int i = 0; i < outRows; ++i)
        for (int j = 0; j < outCols; ++j) out.set(i, j, array.at(r > 0 ? int(r - 1) : i, col > 0 ? int(col - 1) : j));
    Q_UNUSED(m);
    return out;
}

Value fnOffset(Call &c)
{
    double dr, dc, h = 0, w = 0;
    if (!c.number(1, &dr) || !c.number(2, &dc)) return c.failure();
    const Item &it = c.item(0);
    if (it.more) return c.error(ErrorKind::Value);
    RefArea a = it.area;
    const bool haveH = c.argc() > 3 && !c.missing(3), haveW = c.argc() > 4 && !c.missing(4);
    if ((haveH && !c.number(3, &h)) || (haveW && !c.number(4, &w))) return c.failure();
    h = haveH ? std::trunc(h) : double(a.rows());
    w = haveW ? std::trunc(w) : double(a.cols());
    dr = std::trunc(dr);
    dc = std::trunc(dc);
    if (h < 1 || w < 1) return c.error(ErrorKind::Ref);
    const double r1 = double(a.row1) + dr, c1 = double(a.col1) + dc;
    if (r1 < 0 || c1 < 0 || r1 + h - 1 >= double(kGridRows) || c1 + w - 1 >= double(kGridCols)) return c.error(ErrorKind::Ref);
    a.row1 = quint32(r1);
    a.col1 = quint32(c1);
    a.row2 = quint32(r1 + h - 1);
    a.col2 = quint32(c1 + w - 1);
    return c.returnRef(Item::ofRef(a));
}

Value fnIndirect(Call &c)
{
    QString text;
    bool a1;
    if (!c.text(0, &text) || !c.booleanOr(1, true, &a1)) return c.failure();
    text = text.trimmed();
    if (text.isEmpty()) return c.error(ErrorKind::Ref);
    const Item r = c.machine().evaluateText(text, !a1, false, c.context().sheet);
    if (!r.isRef()) {
        if (r.kind == Item::Kind::Value && r.value.isError() && r.value.errorKind() != ErrorKind::Name) return r.value;
        return c.error(ErrorKind::Ref);
    }
    return c.returnRef(r);
}

// ---- where things are

Value fnRow(Call &c)
{
    if (c.missing(0)) return Value::number(double(c.context().row) + 1);
    const Item &it = c.item(0);
    const RefArea &a = it.area;
    if (c.arrayMode() && a.rows() > 1) {
        if (a.rows() > kMaxArrayCells) return c.error(ErrorKind::Value);
        Value out = Value::array(int(a.rows()), 1);
        for (qint64 i = 0; i < a.rows(); ++i) out.set(int(i), 0, Value::number(double(a.row1) + double(i) + 1));
        return out;
    }
    return Value::number(double(a.row1) + 1);
}

Value fnColumn(Call &c)
{
    if (c.missing(0)) return Value::number(double(c.context().col) + 1);
    const Item &it = c.item(0);
    const RefArea &a = it.area;
    if (c.arrayMode() && a.cols() > 1) {
        Value out = Value::array(1, int(a.cols()));
        for (qint64 i = 0; i < a.cols(); ++i) out.set(0, int(i), Value::number(double(a.col1) + double(i) + 1));
        return out;
    }
    return Value::number(double(a.col1) + 1);
}

Value fnRows(Call &c)
{
    const Item &it = c.item(0);
    if (it.isRef()) return Value::number(double(it.area.rows()));
    if (it.kind == Item::Kind::Value) return it.value.isError() ? it.value : Value::number(double(it.value.rows()));
    return c.error(ErrorKind::Value);
}

Value fnColumns(Call &c)
{
    const Item &it = c.item(0);
    if (it.isRef()) return Value::number(double(it.area.cols()));
    if (it.kind == Item::Kind::Value) return it.value.isError() ? it.value : Value::number(double(it.value.cols()));
    return c.error(ErrorKind::Value);
}

Value fnAddress(Call &c)
{
    double row, col, abs = 1;
    bool a1;
    if (!c.number(0, &row) || !c.number(1, &col) || !c.numberOr(2, 1, &abs) || !c.booleanOr(3, true, &a1)) return c.failure();
    row = std::trunc(row);
    col = std::trunc(col);
    const int mode = int(std::trunc(abs));
    if (row < 1 || row > kGridRows || col < 1 || col > kGridCols || mode < 1 || mode > 4) return c.error(ErrorKind::Value);
    const bool rowAbs = mode == 1 || mode == 2, colAbs = mode == 1 || mode == 3;
    QString s;
    if (a1) {
        s = QString(colAbs ? "$" : "") + columnLetters(quint32(col) - 1) + (rowAbs ? "$" : "") + QString::number(qint64(row));
    } else {
        s = QStringLiteral("R") + (rowAbs ? QString::number(qint64(row)) : QStringLiteral("[%1]").arg(qint64(row))) + QStringLiteral("C")
            + (colAbs ? QString::number(qint64(col)) : QStringLiteral("[%1]").arg(qint64(col)));
    }
    if (c.argc() > 4 && !c.missing(4)) {
        QString sheet;
        if (!c.text(4, &sheet)) return c.failure();
        bool quote = sheet.isEmpty();
        for (const QChar ch : sheet)
            if (!ch.isLetterOrNumber() && ch != QLatin1Char('_') && ch != QLatin1Char('.')) quote = true;
        if (!sheet.isEmpty() && sheet[0].isDigit()) quote = true;
        if (quote) {
            sheet.replace(QLatin1Char('\''), QLatin1String("''"));
            sheet = QLatin1Char('\'') + sheet + QLatin1Char('\'');
        }
        s = sheet + QLatin1Char('!') + s;
    }
    return Value::text(s);
}

// CELL(info_type, [reference]): the position of a cell and what it holds. What depends on the file or on the program that shows it (the file's name, the format, the
// color, the width of the column) is not worked out: the evaluation says so, and the yardstick lists the cell apart.
Value fnCell(Call &c)
{
    QString kind;
    if (!c.text(0, &kind)) return c.failure();
    kind = kind.toLower();
    RefArea at;
    at.sheet1 = at.sheet2 = c.context().sheet;
    at.row1 = at.row2 = c.context().row;
    at.col1 = at.col2 = c.context().col;
    if (c.argc() > 1 && !c.missing(1)) {
        if (!c.item(1).isRef()) return c.error(ErrorKind::Value);
        at = c.item(1).area;
    }
    if (kind == QLatin1String("address")) {
        QString s = QStringLiteral("$") + columnLetters(at.col1) + QStringLiteral("$") + QString::number(qint64(at.row1) + 1);
        if (at.sheet1 != c.context().sheet) {
            // another sheet: its name is not known to the engine, so only the cell
        }
        return Value::text(s);
    }
    if (kind == QLatin1String("col")) return Value::number(double(at.col1) + 1);
    if (kind == QLatin1String("row")) return Value::number(double(at.row1) + 1);
    if (kind == QLatin1String("contents")) return c.machine().cell(at.sheet1, at.row1, at.col1);
    if (kind == QLatin1String("type")) {
        const Value v = c.machine().cell(at.sheet1, at.row1, at.col1);
        return Value::text(v.isEmpty() ? QStringLiteral("b") : v.isText() ? QStringLiteral("l") : QStringLiteral("v"));
    }
    for (const char *later : {"filename", "format", "color", "parentheses", "prefix", "protect", "width"})
        if (kind == QLatin1String(later)) {
            c.machine().unsupported |= 4;
            return c.error(ErrorKind::Value);
        }
    return c.error(ErrorKind::Value);
}

Value fnTranspose(Call &c)
{
    Value a;
    if (!c.array(0, &a)) return c.failure();
    Value out = Value::array(a.cols(), a.rows());
    for (int i = 0; i < a.rows(); ++i)
        for (int j = 0; j < a.cols(); ++j) out.set(j, i, a.at(i, j));
    return out;
}

} // namespace

void registerLookupFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"VLOOKUP", 3, 4, "vrvv", 0, fnVLookup});
    r.push_back({"HLOOKUP", 3, 4, "vrvv", 0, fnHLookup});
    r.push_back({"MATCH", 2, 3, "vrv", 0, fnMatch});
    r.push_back({"LOOKUP", 2, 3, "vaa", 0, fnLookup});
    r.push_back({"INDEX", 2, 4, "avvv", 0, fnIndex});
    r.push_back({"OFFSET", 3, 5, "xvvvv", kFnVolatile, fnOffset});
    r.push_back({"INDIRECT", 1, 2, "vv", kFnVolatile, fnIndirect});
    r.push_back({"ROW", 0, 1, "x", 0, fnRow});
    r.push_back({"COLUMN", 0, 1, "x", 0, fnColumn});
    r.push_back({"ROWS", 1, 1, "n", 0, fnRows});
    r.push_back({"COLUMNS", 1, 1, "n", 0, fnColumns});
    r.push_back({"ADDRESS", 2, 5, "vvvvv", 0, fnAddress});
    r.push_back({"CELL", 1, 2, "vn", 0, fnCell});
    r.push_back({"TRANSPOSE", 1, 1, "a", 0, fnTranspose});
}

} // namespace jp::calc
