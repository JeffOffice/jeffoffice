// The lookup and array functions of the newer kind (XLOOKUP, XMATCH, FILTER, SORT, SORTBY, UNIQUE), and AREAS, SHEET, and SHEETS.
#include "calc/machine.h"

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

// An empty cell that is given back in an array is the number 0.
Value shown(const Value &v) { return v.isEmpty() ? Value::number(0) : v; }

// ---- XMATCH and XLOOKUP

// A one-dimensional lookup array as a list; false (with the error) when it is a block of rows and columns.
bool lineOf(Call &c, int arg, std::vector<Value> *out)
{
    Value a;
    if (!c.array(arg, &a)) return false;
    if (a.rows() != 1 && a.cols() != 1) {
        c.error(ErrorKind::Value);
        return false;
    }
    const int n = a.rows() * a.cols();
    out->reserve(size_t(n));
    for (int i = 0; i < n; ++i) out->push_back(a.rows() == 1 ? a.at(0, i) : a.at(i, 0));
    return true;
}

bool sameKind(const Value &a, const Value &b)
{
    if (a.isEmpty() || b.isEmpty()) return a.isEmpty() && b.isEmpty();
    return a.kind() == b.kind();
}

// The place (from 0) of the match, or -1.
int findMatch(Call &c, const Value &what, const std::vector<Value> &line, int matchMode, int searchMode)
{
    const int n = int(line.size());
    if (!c.machine().charge(n / 4 + 1)) return -2;
    auto equal = [&](const Value &v) {
        if (what.isEmpty()) return v.isEmpty();
        if (matchMode == 2 && what.isText() && v.isText()) return wildcardMatch(v.text(), 0, what.text(), 0);
        return sameKind(what, v) && !v.isError() && compareValues(what, v) == 0;
    };
    auto closer = [&](int best, int i, bool larger) {
        // among the values of the kind of `what` that are not equal, the nearest one in the direction asked
        const Value &v = line[size_t(i)];
        if (!sameKind(what, v) || v.isError() || v.isEmpty()) return best;
        const int order = compareValues(v, what);
        if (larger ? order <= 0 : order >= 0) return best;
        if (best < 0) return i;
        const int against = compareValues(v, line[size_t(best)]);
        return larger ? (against < 0 ? i : best) : (against > 0 ? i : best);
    };
    if (searchMode == 2 || searchMode == -2) {
        // binary search on a list sorted ascending (2) or descending (-2)
        int lo = 0, hi = n - 1, found = -1;
        const int dir = searchMode == 2 ? 1 : -1;
        while (lo <= hi) {
            const int mid = (lo + hi) / 2;
            const Value &v = line[size_t(mid)];
            int order = 0;
            if (sameKind(what, v) && !v.isError() && !v.isEmpty()) order = compareValues(v, what) * dir;
            else order = (v.isText() ? 1 : v.isNumber() ? 0 : 2) - (what.isText() ? 1 : 0) > 0 ? 1 : -1;
            if (order == 0 && equal(v)) {
                found = mid;
                break;
            }
            if (order < 0) lo = mid + 1;
            else hi = mid - 1;
        }
        if (found >= 0 || matchMode == 0 || matchMode == 2) return found;
        // the neighbors of where it would go
        int best = -1;
        for (int i = std::max(0, lo - 1); i <= std::min(n - 1, lo); ++i) best = closer(best, i, matchMode > 0);
        return best;
    }
    const bool forward = searchMode >= 0;
    int best = -1;
    for (int k = 0; k < n; ++k) {
        const int i = forward ? k : n - 1 - k;
        if (equal(line[size_t(i)])) return i;
        if (matchMode == 1 || matchMode == -1) best = closer(best, i, matchMode == 1);
    }
    return best;
}

bool lookupModes(Call &c, int matchArg, int searchArg, int *match, int *search)
{
    double m = 0, s = 1;
    if (!c.numberOr(matchArg, 0, &m) || !c.numberOr(searchArg, 1, &s)) return false;
    m = std::trunc(m);
    s = std::trunc(s);
    if ((m < -1 || m > 2) || (s != 1 && s != -1 && s != 2 && s != -2)) {
        c.error(ErrorKind::Value);
        return false;
    }
    *match = int(m);
    *search = int(s);
    return true;
}

Value fnXMatch(Call &c)
{
    int match, search;
    std::vector<Value> line;
    const Value what = c.scalar(0);
    if (what.isError()) return what;
    if (!lineOf(c, 1, &line) || !lookupModes(c, 2, 3, &match, &search)) return c.failure();
    const int at = findMatch(c, what, line, match, search);
    if (at == -2) return c.error(ErrorKind::Value);
    return at < 0 ? c.error(ErrorKind::NA) : Value::number(at + 1);
}

Value fnXLookup(Call &c)
{
    int match, search;
    std::vector<Value> line;
    const Value what = c.scalar(0);
    if (what.isError()) return what;
    if (!lineOf(c, 1, &line) || !lookupModes(c, 4, 5, &match, &search)) return c.failure();
    Value result;
    if (!c.array(2, &result)) return c.failure();
    Value keys;
    c.array(1, &keys);
    const bool byRow = keys.cols() == 1;   // the lookup array runs down: the answer is a row of the return array
    if ((byRow ? result.rows() : result.cols()) != int(line.size())) return c.error(ErrorKind::Value);
    const int at = findMatch(c, what, line, match, search);
    if (at == -2) return c.error(ErrorKind::Value);
    if (at < 0) {
        if (!c.missing(3)) return c.scalar(3);
        return c.error(ErrorKind::NA);
    }
    const int rows = byRow ? 1 : result.rows(), cols = byRow ? result.cols() : 1;
    if (rows == 1 && cols == 1) return shown(byRow ? result.at(at, 0) : result.at(0, at));
    Value out = c.machine().newArray(rows, cols);
    if (!out.isArray()) return out;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) out.set(i, j, shown(byRow ? result.at(at, j) : result.at(i, at)));
    return out;
}

// ---- reading and writing rows (or columns) of an array

struct Lines {
    Value array;
    bool byCol = false;
    int count() const { return byCol ? array.cols() : array.rows(); }
    int length() const { return byCol ? array.rows() : array.cols(); }
    const Value &at(int line, int k) const { return byCol ? array.at(k, line) : array.at(line, k); }
};

Value assemble(Call &c, const Lines &in, const std::vector<int> &order)
{
    const int lines = int(order.size());
    if (!c.machine().charge(qint64(lines) * in.length())) return c.error(ErrorKind::Value);
    Value out = c.machine().newArray(in.byCol ? in.length() : lines, in.byCol ? lines : in.length());
    if (!out.isArray()) return out;
    for (int i = 0; i < lines; ++i)
        for (int k = 0; k < in.length(); ++k) {
            if (in.byCol) out.set(k, i, shown(in.at(order[size_t(i)], k)));
            else out.set(i, k, shown(in.at(order[size_t(i)], k)));
        }
    return out;
}

bool truthy(const Value &v, bool *out)
{
    if (v.isBool() || v.isNumber()) {
        *out = v.number() != 0;
        return true;
    }
    return false;
}

Value fnFilter(Call &c)
{
    Lines in;
    Value include;
    if (!c.array(0, &in.array) || !c.array(1, &include)) return c.failure();
    // the test runs down (a column) to filter rows, or across to filter columns
    if (include.cols() == 1 && include.rows() == in.array.rows()) in.byCol = false;
    else if (include.rows() == 1 && include.cols() == in.array.cols()) in.byCol = true;
    else return c.error(ErrorKind::Value);
    std::vector<int> keep;
    const int n = in.byCol ? include.cols() : include.rows();
    for (int i = 0; i < n; ++i) {
        const Value &v = in.byCol ? include.at(0, i) : include.at(i, 0);
        if (v.isError()) return v;
        bool yes;
        if (v.isEmpty()) yes = false;
        else if (!truthy(v, &yes)) return c.error(ErrorKind::Value);
        if (yes) keep.push_back(i);
    }
    if (keep.empty()) {
        if (!c.missing(2)) return c.scalar(2);
        return c.error(ErrorKind::Calc);
    }
    return assemble(c, in, keep);
}

// The order of values for sorting: numbers, then text, then logical values, then errors, with empty cells last.
int sortRank(const Value &v) { return v.isEmpty() ? 4 : v.isError() ? 3 : v.isBool() ? 2 : v.isText() ? 1 : 0; }

int compareForSort(const Value &a, const Value &b)
{
    const int ra = sortRank(a), rb = sortRank(b);
    if (ra != rb) return ra < rb ? -1 : 1;
    if (ra == 4) return 0;
    if (ra == 3) return int(a.errorKind()) < int(b.errorKind()) ? -1 : int(a.errorKind()) > int(b.errorKind()) ? 1 : 0;
    return compareValues(a, b);
}

Value fnSort(Call &c)
{
    Lines in;
    Value indexes, orders;
    bool byCol = false;
    if (!c.array(0, &in.array) || !c.booleanOr(3, false, &byCol)) return c.failure();
    in.byCol = byCol;
    // the number of the column (or row) to sort by, and the order, each one number or a list of them (one key for each)
    if (c.missing(1)) indexes = Value::number(1);
    else if (!c.array(1, &indexes)) return c.failure();
    if (c.missing(2)) orders = Value::number(1);
    else if (!c.array(2, &orders)) return c.failure();
    const int keyCount = indexes.rows() * indexes.cols();
    const int orderCount = orders.rows() * orders.cols();
    if (orderCount != 1 && orderCount != keyCount) return c.error(ErrorKind::Value);
    struct Key {
        int column;
        double order;
    };
    std::vector<Key> keys;
    for (int i = 0; i < keyCount; ++i) {
        const Value &x = indexes.at(i / indexes.cols(), i % indexes.cols());
        const Value &o = orders.at(orderCount == 1 ? 0 : i / orders.cols(), orderCount == 1 ? 0 : i % orders.cols());
        if (x.isError()) return x;
        if (o.isError()) return o;
        if (!x.isNumber() || !o.isNumber()) return c.error(ErrorKind::Value);
        const double index = std::trunc(x.number());
        if (index < 1 || index > in.length() || (o.number() != 1 && o.number() != -1)) return c.error(ErrorKind::Value);
        keys.push_back({int(index) - 1, o.number()});
    }
    std::vector<int> rows(static_cast<size_t>(in.count()));
    for (size_t i = 0; i < rows.size(); ++i) rows[i] = int(i);
    if (!c.machine().chargeCount(double(rows.size()) * (std::log2(double(rows.size()) + 1) + 1) * double(keys.size()) * 2)) return c.error(ErrorKind::Value);
    std::stable_sort(rows.begin(), rows.end(), [&](int a, int b) {
        for (const Key &k : keys) {
            const Value &x = in.at(a, k.column), &y = in.at(b, k.column);
            // (empty cells stay last whatever the order)
            if (x.isEmpty() || y.isEmpty()) {
                if (x.isEmpty() && y.isEmpty()) continue;
                return !x.isEmpty();
            }
            const int r = compareForSort(x, y);
            if (r != 0) return k.order > 0 ? r < 0 : r > 0;
        }
        return false;
    });
    return assemble(c, in, rows);
}

Value fnSortBy(Call &c)
{
    Lines in;
    if (!c.array(0, &in.array)) return c.failure();
    struct By {
        Value values;
        double order = 1;
    };
    std::vector<By> keys;
    for (int i = 1; i < c.argc(); i += 2) {
        By b;
        if (!c.array(i, &b.values)) return c.failure();
        if (i + 1 < c.argc() && !c.missing(i + 1) && !c.number(i + 1, &b.order)) return c.failure();
        if (b.order != 1 && b.order != -1) return c.error(ErrorKind::Value);
        keys.push_back(b);
    }
    // the keys run down to sort rows, across to sort columns
    const bool down = keys[0].values.cols() == 1 && keys[0].values.rows() == in.array.rows();
    const bool across = keys[0].values.rows() == 1 && keys[0].values.cols() == in.array.cols();
    if (!down && !across) return c.error(ErrorKind::Value);
    in.byCol = !down;
    for (const By &b : keys) {
        if (down ? !(b.values.cols() == 1 && b.values.rows() == in.array.rows()) : !(b.values.rows() == 1 && b.values.cols() == in.array.cols())) return c.error(ErrorKind::Value);
    }
    std::vector<int> rows(size_t(in.count()));
    for (size_t i = 0; i < rows.size(); ++i) rows[i] = int(i);
    if (!c.machine().chargeCount(double(rows.size()) * (std::log2(double(rows.size()) + 1) + 1) * double(keys.size()) * 2)) return c.error(ErrorKind::Value);
    std::stable_sort(rows.begin(), rows.end(), [&](int a, int b) {
        for (const By &k : keys) {
            const Value &x = down ? k.values.at(a, 0) : k.values.at(0, a), &y = down ? k.values.at(b, 0) : k.values.at(0, b);
            if (x.isEmpty() || y.isEmpty()) {
                if (x.isEmpty() && y.isEmpty()) continue;
                return !x.isEmpty();
            }
            const int r = compareForSort(x, y);
            if (r != 0) return k.order > 0 ? r < 0 : r > 0;
        }
        return false;
    });
    return assemble(c, in, rows);
}

Value fnUnique(Call &c)
{
    Lines in;
    bool byCol = false, once = false;
    if (!c.array(0, &in.array) || !c.booleanOr(1, false, &byCol) || !c.booleanOr(2, false, &once)) return c.failure();
    in.byCol = byCol;
    const int n = in.count();
    if (!c.machine().chargeCount(double(n) * (std::log2(double(n) + 1) + 1) * (double(in.length()) + 1))) return c.error(ErrorKind::Value);
    auto same = [&](int a, int b) {
        for (int k = 0; k < in.length(); ++k) {
            const Value &x = in.at(a, k), &y = in.at(b, k);
            if (x.isEmpty() != y.isEmpty()) return false;
            if (x.isEmpty()) continue;
            if (x.kind() != y.kind()) return false;
            if (x.isError() ? x.errorKind() != y.errorKind() : compareValues(x, y) != 0) return false;
        }
        return true;
    };
    // (the lines are put in order of their first value to find the equal ones fast, and the first of each is kept in its place)
    std::vector<int> order(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) order[size_t(i)] = i;
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        for (int k = 0; k < in.length(); ++k) {
            const int r = compareForSort(in.at(a, k), in.at(b, k));
            if (r != 0) return r < 0;
        }
        return false;
    });
    std::vector<int> firstOf(static_cast<size_t>(n), -1), count(static_cast<size_t>(n), 0);
    for (int s = 0; s < n;) {
        int e = s + 1;
        while (e < n && same(order[size_t(s)], order[size_t(e)])) ++e;
        const int first = order[size_t(s)];   // (stable: the earliest of the equal lines)
        for (int k = s; k < e; ++k) firstOf[size_t(order[size_t(k)])] = first;
        count[size_t(first)] = e - s;
        s = e;
    }
    std::vector<int> keep;
    for (int i = 0; i < n; ++i)
        if (firstOf[size_t(i)] == i && (!once || count[size_t(i)] == 1)) keep.push_back(i);
    if (keep.empty()) return c.error(ErrorKind::Calc);
    return assemble(c, in, keep);
}

// ---- references and sheets

Value fnAreas(Call &c)
{
    const Item &it = c.item(0);
    if (it.isRef()) return Value::number(it.areaCount());
    if (it.kind == Item::Kind::Value && it.value.isError()) return it.value;
    return c.error(ErrorKind::Value);
}

Value fnSheet(Call &c)
{
    if (c.missing(0)) return Value::number(c.context().sheet + 1);
    const Item &it = c.item(0);
    if (it.isRef()) return Value::number(it.area.sheet1 + 1);
    if (it.kind == Item::Kind::Value && it.value.isError()) return it.value;
    QString name;
    if (!c.text(0, &name)) return c.failure();
    const int at = c.machine().source().sheetIndex(name);
    return at < 0 ? c.error(ErrorKind::NA) : Value::number(at + 1);
}

Value fnSheets(Call &c)
{
    if (c.missing(0)) return Value::number(c.machine().sheetCount());
    const Item &it = c.item(0);
    if (it.isRef()) return Value::number(double(it.area.sheet2 - it.area.sheet1 + 1));
    if (it.kind == Item::Kind::Value && it.value.isError()) return it.value;
    return c.error(ErrorKind::Value);
}

// The cell a reference argument names (the top left one of a block), or false with the error.
bool firstCellOf(Call &c, RefArea *area)
{
    const Item &it = c.item(0);
    if (!it.isRef()) {
        c.error(it.kind == Item::Kind::Value && it.value.isError() ? it.value.errorKind() : ErrorKind::Value);
        return false;
    }
    *area = it.area;
    return true;
}

Value fnIsFormula(Call &c)
{
    RefArea a;
    if (!firstCellOf(c, &a)) return c.failure();
    c.noteRead(a);
    return Value::boolean(c.machine().source().formulaInfo(a.sheet1, a.row1, a.col1, nullptr));
}

Value fnFormulaText(Call &c)
{
    RefArea a;
    if (!firstCellOf(c, &a)) return c.failure();
    c.noteRead(a);
    QString text;
    if (!c.machine().source().formulaInfo(a.sheet1, a.row1, a.col1, &text)) return c.error(ErrorKind::NA);
    return Value::text(QLatin1Char('=') + text);
}

} // namespace

void registerArrayFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"XMATCH", 2, 4, "vavv", kFnFuture, fnXMatch});
    r.push_back({"XLOOKUP", 3, 6, "vaavvv", kFnFuture, fnXLookup});
    r.push_back({"FILTER", 2, 3, "aav", kFnFuture, fnFilter});
    r.push_back({"SORT", 1, 4, "arrv", kFnFuture, fnSort});
    r.push_back({"SORTBY", 2, 255, "aar", kFnFuture, fnSortBy});
    r.push_back({"UNIQUE", 1, 3, "avv", kFnFuture, fnUnique});
    r.push_back({"AREAS", 1, 1, "x", 0, fnAreas});
    r.push_back({"ISFORMULA", 1, 1, "x", kFnFuture, fnIsFormula});
    r.push_back({"FORMULATEXT", 1, 1, "x", kFnFuture, fnFormulaText});
    r.push_back({"SHEET", 0, 1, "n", kFnFuture, fnSheet});
    r.push_back({"SHEETS", 0, 1, "n", kFnFuture, fnSheets});
}

} // namespace jp::calc
