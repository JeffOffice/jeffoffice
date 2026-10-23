// The logical and information functions.
#include "calc/machine.h"

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

// ---- IF and the others that choose, in the form that works out every argument (an array formula, where the condition may be an array)

Value pick(Call &c, const Item &item)
{
    if (item.isMissing()) return Value::number(0);
    return c.returnRef(item);
}

// Calls f(r, c) with the condition's element for an array; returns the array it makes. `parts` are the values the elements are taken from (stretched).
template <class F>
Value zipN(Call &c, const std::vector<Value> &parts, int rows, int cols, F f)
{
    if (qint64(rows) * cols > kMaxArrayCells) {
        c.machine().tooLarge();
        return c.error(ErrorKind::Value);
    }
    if (!c.machine().charge(qint64(rows) * cols)) return c.error(ErrorKind::Value);
    Value r = Value::array(rows, cols);
    std::vector<const Value *> cell(parts.size());
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) {
            for (size_t k = 0; k < parts.size(); ++k) cell[k] = &parts[k].at(parts[k].rows() == 1 ? 0 : i, parts[k].cols() == 1 ? 0 : j);
            r.set(i, j, f(cell));
        }
    return r;
}

Value fnIf(Call &c)
{
    Machine &m = c.machine();
    const Value cond = m.scalar(c.item(0), true);
    const Item none = Item::ofValue(Value::boolean(false));
    if (!cond.isArray()) {
        bool truth = false;
        ErrorKind e = ErrorKind::Value;
        if (!coerceBool(cond, &truth, &e)) return c.error(e);
        if (truth) return pick(c, c.item(1));
        return c.argc() > 2 ? pick(c, c.item(2)) : c.returnRef(none);
    }
    const Value a = c.missing(1) ? Value::number(0) : m.scalar(c.item(1), true);
    const Value b = c.argc() > 2 ? (c.missing(2) ? Value::number(0) : m.scalar(c.item(2), true)) : Value::boolean(false);
    const int rows = std::max({cond.rows(), a.rows(), b.rows()}), cols = std::max({cond.cols(), a.cols(), b.cols()});
    return zipN(c, {cond, a, b}, rows, cols, [&](const std::vector<const Value *> &v) {
        bool truth = false;
        ErrorKind e = ErrorKind::Value;
        if (!coerceBool(*v[0], &truth, &e)) return Value::error(e);
        return truth ? *v[1] : *v[2];
    });
}

Value fnIfError(Call &c, bool naOnly)
{
    Machine &m = c.machine();
    const Value v = m.scalar(c.item(0), true);
    auto caught = [&](const Value &x) { return x.isError() && (!naOnly || x.errorKind() == ErrorKind::NA); };
    if (!v.isArray()) {
        if (!caught(v)) return v;
        return pick(c, c.item(1));
    }
    const Value alt = c.missing(1) ? Value::number(0) : m.scalar(c.item(1), true);
    const int rows = std::max(v.rows(), alt.rows()), cols = std::max(v.cols(), alt.cols());
    return zipN(c, {v, alt}, rows, cols, [&](const std::vector<const Value *> &x) { return caught(*x[0]) ? *x[1] : *x[0]; });
}
Value fnIfErrorF(Call &c) { return fnIfError(c, false); }
Value fnIfNaF(Call &c) { return fnIfError(c, true); }

Value fnChoose(Call &c)
{
    Machine &m = c.machine();
    const Value index = m.scalar(c.item(0), true);
    const int n = c.argc() - 1;
    auto pickIndex = [&](const Value &x, int *k) {
        double d;
        ErrorKind e = ErrorKind::Value;
        if (!coerceNumber(x, c.context().dateSystem, &d, &e)) return e;
        d = std::trunc(d);
        if (!(d >= 1) || d > n) return ErrorKind::Value;
        *k = int(d);
        return ErrorKind::Null;
    };
    if (!index.isArray()) {
        int k = 0;
        const ErrorKind e = pickIndex(index, &k);
        if (e != ErrorKind::Null) return c.error(e);
        return pick(c, c.item(k));
    }
    std::vector<Value> parts{index};
    int rows = index.rows(), cols = index.cols();
    for (int k = 1; k <= n; ++k) {
        parts.push_back(c.missing(k) ? Value::number(0) : m.scalar(c.item(k), true));
        rows = std::max(rows, parts.back().rows());
        cols = std::max(cols, parts.back().cols());
    }
    return zipN(c, parts, rows, cols, [&](const std::vector<const Value *> &v) {
        int k = 0;
        const ErrorKind e = pickIndex(*v[0], &k);
        return e != ErrorKind::Null ? Value::error(e) : *v[size_t(k)];
    });
}

// ---- AND, OR, XOR

// Goes over the arguments: a typed value counts if it is a boolean, a number, or the text TRUE or FALSE (any other text is #VALUE!); in a reference or an array a
// text or an empty cell is left out. An error stops it. `seen` says whether any value counted.
bool eachLogical(Call &c, const std::function<void(bool)> &f, Value *error, bool *seen)
{
    *seen = false;
    for (int i = 0; i < c.argc(); ++i) {
        bool ok = true;
        const bool done = c.each(i, [&](const Value &v, bool typed) {
            if (v.isError()) {
                *error = v;
                ok = false;
                return false;
            }
            if (v.isBool() || v.isNumber()) {
                f(v.number() != 0);
                *seen = true;
            } else if (v.isText() && typed) {
                bool b;
                ErrorKind e = ErrorKind::Value;
                if (!coerceBool(v, &b, &e)) {
                    *error = Value::error(e);
                    ok = false;
                    return false;
                }
                f(b);
                *seen = true;
            }
            return true;
        });
        if (!done || !ok) return false;
    }
    return true;
}

Value fnAnd(Call &c)
{
    bool all = true, seen = false;
    Value error;
    if (!eachLogical(c, [&](bool b) { all = all && b; }, &error, &seen)) return error.isError() ? error : c.error(ErrorKind::Value);
    return seen ? Value::boolean(all) : c.error(ErrorKind::Value);
}

Value fnOr(Call &c)
{
    bool any = false, seen = false;
    Value error;
    if (!eachLogical(c, [&](bool b) { any = any || b; }, &error, &seen)) return error.isError() ? error : c.error(ErrorKind::Value);
    return seen ? Value::boolean(any) : c.error(ErrorKind::Value);
}

Value fnXor(Call &c)
{
    int count = 0;
    bool seen = false;
    Value error;
    if (!eachLogical(c, [&](bool b) { count += b ? 1 : 0; }, &error, &seen)) return error.isError() ? error : c.error(ErrorKind::Value);
    return seen ? Value::boolean(count % 2 == 1) : c.error(ErrorKind::Value);
}

Value fnNot(Call &c)
{
    bool b;
    if (!c.boolean(0, &b)) return c.failure();
    return Value::boolean(!b);
}

Value fnTrue(Call &) { return Value::boolean(true); }
Value fnFalse(Call &) { return Value::boolean(false); }

// ---- information

Value fnIsNumber(Call &c) { return Value::boolean(c.scalar(0).isNumber()); }
Value fnIsText(Call &c) { return Value::boolean(c.scalar(0).isText()); }
Value fnIsNonText(Call &c) { return Value::boolean(!c.scalar(0).isText()); }
Value fnIsLogical(Call &c) { return Value::boolean(c.scalar(0).isBool()); }
Value fnIsBlank(Call &c) { return Value::boolean(c.scalar(0).isEmpty()); }
Value fnIsError(Call &c) { return Value::boolean(c.scalar(0).isError()); }
Value fnIsErr(Call &c)
{
    const Value v = c.scalar(0);
    return Value::boolean(v.isError() && v.errorKind() != ErrorKind::NA);
}
Value fnIsNa(Call &c)
{
    const Value v = c.scalar(0);
    return Value::boolean(v.isError() && v.errorKind() == ErrorKind::NA);
}
Value fnIsRef(Call &c) { return Value::boolean(!c.missing(0) && c.item(0).isRef()); }

Value fnIsEven(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    return Value::boolean(std::fmod(std::floor(std::fabs(x)), 2.0) == 0);
}
Value fnIsOdd(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    return Value::boolean(std::fmod(std::floor(std::fabs(x)), 2.0) == 1);
}

Value fnNa(Call &) { return Value::error(ErrorKind::NA); }

Value fnN(Call &c)
{
    const Value v = c.scalar(0);
    switch (v.kind()) {
    case Value::Kind::Number:
    case Value::Kind::Bool: return Value::number(v.number());
    case Value::Kind::Error: return v;
    default: return Value::number(0);
    }
}

Value fnType(Call &c)
{
    if (c.missing(0)) return c.error(ErrorKind::Value);
    const Item &it = c.item(0);
    if (it.isRef() && (it.more || !it.area.single())) return Value::number(64);
    const Value v = c.machine().scalar(it, true);
    switch (v.kind()) {
    case Value::Kind::Number: case Value::Kind::Empty: return Value::number(1);
    case Value::Kind::Text: return Value::number(2);
    case Value::Kind::Bool: return Value::number(4);
    case Value::Kind::Error: return Value::number(16);
    case Value::Kind::Array: return Value::number(64);
    }
    return Value::number(1);
}

Value fnErrorType(Call &c)
{
    const Value v = c.scalar(0);
    if (!v.isError()) return c.error(ErrorKind::NA);
    switch (v.errorKind()) {
    case ErrorKind::Null: return Value::number(1);
    case ErrorKind::Div0: return Value::number(2);
    case ErrorKind::Value: return Value::number(3);
    case ErrorKind::Ref: return Value::number(4);
    case ErrorKind::Name: return Value::number(5);
    case ErrorKind::Num: return Value::number(6);
    case ErrorKind::NA: return Value::number(7);
    case ErrorKind::GettingData: return Value::number(8);
    default: return c.error(ErrorKind::NA);
    }
}

} // namespace

void registerLogicFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"IF", 2, 3, "nnn", kFnLazy, fnIf});
    r.push_back({"IFERROR", 2, 2, "nn", kFnLazy, fnIfErrorF});
    r.push_back({"IFNA", 2, 2, "nn", kFnLazy, fnIfNaF});
    r.push_back({"CHOOSE", 2, 254, "nn", kFnLazy, fnChoose});
    r.push_back({"AND", 1, 255, "r", 0, fnAnd});
    r.push_back({"OR", 1, 255, "r", 0, fnOr});
    r.push_back({"XOR", 1, 255, "r", 0, fnXor});
    r.push_back({"NOT", 1, 1, "v", 0, fnNot});
    r.push_back({"TRUE", 0, 0, "", 0, fnTrue});
    r.push_back({"FALSE", 0, 0, "", 0, fnFalse});
    r.push_back({"ISNUMBER", 1, 1, "v", 0, fnIsNumber});
    r.push_back({"ISTEXT", 1, 1, "v", 0, fnIsText});
    r.push_back({"ISNONTEXT", 1, 1, "v", 0, fnIsNonText});
    r.push_back({"ISLOGICAL", 1, 1, "v", 0, fnIsLogical});
    r.push_back({"ISBLANK", 1, 1, "v", 0, fnIsBlank});
    r.push_back({"ISERROR", 1, 1, "v", 0, fnIsError});
    r.push_back({"ISERR", 1, 1, "v", 0, fnIsErr});
    r.push_back({"ISNA", 1, 1, "v", 0, fnIsNa});
    r.push_back({"ISREF", 1, 1, "n", 0, fnIsRef});
    r.push_back({"ISEVEN", 1, 1, "v", 0, fnIsEven});
    r.push_back({"ISODD", 1, 1, "v", 0, fnIsOdd});
    r.push_back({"NA", 0, 0, "", 0, fnNa});
    r.push_back({"N", 1, 1, "v", 0, fnN});
    r.push_back({"TYPE", 1, 1, "n", 0, fnType});
    r.push_back({"ERROR.TYPE", 1, 1, "v", 0, fnErrorType});
}

} // namespace jp::calc
