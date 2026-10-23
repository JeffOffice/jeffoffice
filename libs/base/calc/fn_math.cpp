// The number functions: sums and extremes, counts, rounding, powers, logarithms, and the rest of the arithmetic.
#include "calc/machine.h"

#include <QByteArray>
#include <QString>

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

// ---- the digits of a number as the original rounds them: its 15 significant digits as decimal text

struct Digits {
    QByteArray d;   // 15 digits
    int exp = 0;    // the value is d[0].d[1]d[2]... x 10^exp
    bool negative = false;
};

Digits digitsOf(double x)
{
    Digits r;
    r.negative = x < 0;
    const QByteArray t = QString::number(std::fabs(x), 'e', 14).toLatin1();   // d.dddddddddddddde+XX
    const int e = t.indexOf('e');
    r.d = t.left(1) + t.mid(2, e - 2);
    r.exp = t.mid(e + 1).toInt();
    return r;
}

// The digits (an integer written out) times 10^exp10; false if that is no number a double holds.
bool fromDigits(const QByteArray &digits, int exp10, bool negative, double *out)
{
    bool ok = false;
    const double v = (digits + "e" + QByteArray::number(exp10)).toDouble(&ok);
    if (!ok || !std::isfinite(v)) return false;
    *out = negative ? -v : v;
    return true;
}

enum class RoundMode { Nearest, Up, Down };

// x rounded to `places` decimal places (negative: to tens, hundreds, ...) from its 15 significant digits: half away from zero, or always away from zero, or always
// toward it. False if the result is no number.
bool roundDecimal(double x, int places, RoundMode mode, double *out)
{
    if (x == 0 || !std::isfinite(x)) {
        *out = x;
        return std::isfinite(x);
    }
    places = std::max(-400, std::min(400, places));
    const Digits g = digitsOf(x);
    const int keep = g.exp + places + 1;   // how many of the 15 digits stay: the digit k has the place value 10^(exp - k)
    if (keep >= 15) return fromDigits(g.d, g.exp - 14, g.negative, out);
    QByteArray kept;
    bool anyRest = true, half = false;
    if (keep >= 0) {
        kept = g.d.left(keep);
        const QByteArray rest = g.d.mid(keep);
        anyRest = false;
        for (const char ch : rest)
            if (ch != '0') anyRest = true;
        half = !rest.isEmpty() && rest[0] >= '5';
    }
    bool increment = false;
    switch (mode) {
    case RoundMode::Nearest: increment = half; break;
    case RoundMode::Up: increment = anyRest; break;
    case RoundMode::Down: break;
    }
    if (kept.isEmpty()) kept = "0";
    if (increment) {
        int i = int(kept.size()) - 1;
        for (; i >= 0; --i) {
            if (kept[i] == '9') {
                kept[i] = '0';
            } else {
                kept[i] = char(kept[i] + 1);
                break;
            }
        }
        if (i < 0) kept.prepend('1');
    }
    return fromDigits(kept, -places, g.negative, out);
}

// ---- going over numbers

enum NumberMode {
    kSkipText = 0,    // SUM, MAX, ...: in a reference or array only numbers count
    kTextIsZero = 1,  // MAXA, AVERAGEA: text counts as 0 and true as 1
    kIgnoreErrors = 2 // COUNT: an error is no number and no problem
};

// Gives f each number in the arguments from `first` on. A typed text is read as a number or is #VALUE!; typed true and false are 1 and 0; an argument left out is 0.
// In a reference or an array only numbers count (and with kTextIsZero a text is 0 and true and false 1). An error stops it (it is `*error`), unless kIgnoreErrors.
template <class F>
bool eachNumber(Call &c, int first, int mode, Value *error, F f)
{
    for (int i = first; i < c.argc(); ++i) {
        bool ok = true;
        const bool done = c.each(i, [&](const Value &v, bool typed) {
            switch (v.kind()) {
            case Value::Kind::Number: f(v.number()); return true;
            case Value::Kind::Bool:
                if (typed || (mode & kTextIsZero)) f(v.number());
                return true;
            case Value::Kind::Empty:
                if (typed) f(0);
                return true;
            case Value::Kind::Text: {
                double x;
                if (typed) {
                    if (textToNumber(v.text(), c.context().dateSystem, &x)) {
                        f(x);
                        return true;
                    }
                    if (mode & kIgnoreErrors) return true;
                    *error = Value::error(ErrorKind::Value);
                    ok = false;
                    return false;
                }
                if (mode & kTextIsZero) f(0);
                return true;
            }
            case Value::Kind::Error:
                if (mode & kIgnoreErrors) return true;
                *error = v;
                ok = false;
                return false;
            case Value::Kind::Array: return true;
            }
            return true;
        });
        if (!done) {
            if (ok) *error = Value::error(ErrorKind::Value);   // the work gave out
            return false;
        }
    }
    return true;
}

Value fnSum(Call &c)
{
    double sum = 0;
    Value error;
    if (!eachNumber(c, 0, kSkipText, &error, [&](double x) { sum = snappedSum(sum, x); })) return error;
    return numberResult(sum);
}

Value fnProduct(Call &c)
{
    double product = 1;
    bool any = false;
    Value error;
    if (!eachNumber(c, 0, kSkipText, &error, [&](double x) {
            product *= x;
            any = true;
        }))
        return error;
    return numberResult(any ? product : 0);
}

Value fnSumsq(Call &c)
{
    double sum = 0;
    Value error;
    if (!eachNumber(c, 0, kSkipText, &error, [&](double x) { sum += x * x; })) return error;
    return numberResult(sum);
}

Value extreme(Call &c, bool greatest, int mode)
{
    double best = 0;
    bool any = false;
    Value error;
    if (!eachNumber(c, 0, mode, &error, [&](double x) {
            if (!any || (greatest ? x > best : x < best)) best = x;
            any = true;
        }))
        return error;
    return Value::number(any ? best : 0);
}
Value fnMax(Call &c) { return extreme(c, true, kSkipText); }
Value fnMin(Call &c) { return extreme(c, false, kSkipText); }
Value fnMaxA(Call &c) { return extreme(c, true, kTextIsZero); }
Value fnMinA(Call &c) { return extreme(c, false, kTextIsZero); }

Value average(Call &c, int mode)
{
    double sum = 0;
    qint64 n = 0;
    Value error;
    if (!eachNumber(c, 0, mode, &error, [&](double x) {
            sum = snappedSum(sum, x);
            ++n;
        }))
        return error;
    if (n == 0) return Value::error(ErrorKind::Div0);
    return numberResult(sum / double(n));
}
Value fnAverage(Call &c) { return average(c, kSkipText); }
Value fnAverageA(Call &c) { return average(c, kTextIsZero); }

Value fnCount(Call &c)
{
    qint64 n = 0;
    Value error;
    if (!eachNumber(c, 0, kSkipText | kIgnoreErrors, &error, [&](double) { ++n; })) return error;
    return Value::number(double(n));
}

Value fnCountA(Call &c)
{
    qint64 n = 0;
    for (int i = 0; i < c.argc(); ++i) {
        if (c.item(i).isMissing()) continue;
        const bool done = c.each(i, [&](const Value &, bool) {
            ++n;
            return true;
        });
        if (!done) return c.error(ErrorKind::Value);
    }
    return Value::number(double(n));
}

Value fnCountBlank(Call &c)
{
    const Item &it = c.item(0);
    if (!it.isRef() || it.more) {
        if (it.kind == Item::Kind::Value && it.value.isArray()) {
            qint64 n = 0;
            for (int r = 0; r < it.value.rows(); ++r)
                for (int col = 0; col < it.value.cols(); ++col) {
                    const Value &v = it.value.at(r, col);
                    if (v.isEmpty() || (v.isText() && v.text().isEmpty())) ++n;
                }
            return Value::number(double(n));
        }
        return c.error(ErrorKind::Value);
    }
    // the cells of the area, less those that hold something other than an empty text
    qint64 held = 0;
    const RefArea &a = it.area;
    for (int s = a.sheet1; s <= a.sheet2; ++s)
        c.machine().source().visit(s, a.row1, a.col1, a.row2, a.col2, [&](quint32, quint32, const Value &v) {
            if (!c.machine().charge(1)) return false;
            if (!v.isEmpty() && !(v.isText() && v.text().isEmpty())) ++held;
            return true;
        });
    if (c.machine().stopped()) return c.error(ErrorKind::Value);
    return Value::number(double(a.cells() - held));
}

// ---- functions of one number

Value fnAbs(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    return Value::number(std::fabs(x));
}

Value fnSqrt(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    if (x < 0) return c.error(ErrorKind::Num);
    return Value::number(std::sqrt(x));
}

Value fnExp(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    const double r = std::exp(x);
    return numberResult(r);
}

Value fnLn(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    if (x <= 0) return c.error(ErrorKind::Num);
    return Value::number(std::log(x));
}

Value fnLog10(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    if (x <= 0) return c.error(ErrorKind::Num);
    return Value::number(std::log10(x));
}

Value fnLog(Call &c)
{
    double x, base;
    if (!c.number(0, &x) || !c.numberOr(1, 10, &base)) return c.failure();
    if (x <= 0 || base <= 0) return c.error(ErrorKind::Num);
    if (base == 1) return c.error(ErrorKind::Div0);
    if (base == 10) return Value::number(std::log10(x));
    return numberResult(std::log(x) / std::log(base));
}

Value fnSign(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    return Value::number(x > 0 ? 1 : x < 0 ? -1 : 0);
}

Value fnInt(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    return Value::number(std::floor(x));
}

Value fnPi(Call &) { return Value::number(3.14159265358979323846); }

Value fnPower(Call &c)
{
    double x, y;
    if (!c.number(0, &x) || !c.number(1, &y)) return c.failure();
    if (x == 0 && y == 0) return c.error(ErrorKind::Num);
    if (x == 0 && y < 0) return c.error(ErrorKind::Div0);
    if (x < 0 && y != std::floor(y)) return c.error(ErrorKind::Num);
    return numberResult(powerOf(x, y));
}

Value fnMod(Call &c)
{
    double n, d;
    if (!c.number(0, &n) || !c.number(1, &d)) return c.failure();
    if (d == 0) return c.error(ErrorKind::Div0);
    double r = std::fmod(n, d);
    if (r != 0 && ((r < 0) != (d < 0))) r += d;
    return numberResult(r);
}

Value fnQuotient(Call &c)
{
    double n, d;
    if (!c.number(0, &n) || !c.number(1, &d)) return c.failure();
    if (d == 0) return c.error(ErrorKind::Div0);
    return numberResult(std::trunc(n / d));
}

Value rounding(Call &c, RoundMode mode)
{
    double x, p;
    if (!c.number(0, &x) || !c.number(1, &p)) return c.failure();
    p = std::trunc(p);
    if (std::fabs(p) > 1e6) p = p < 0 ? -1e6 : 1e6;
    double r;
    if (!roundDecimal(x, int(p), mode, &r)) return c.error(ErrorKind::Num);
    return Value::number(r);
}
Value fnRound(Call &c) { return rounding(c, RoundMode::Nearest); }
Value fnRoundUp(Call &c) { return rounding(c, RoundMode::Up); }
Value fnRoundDown(Call &c) { return rounding(c, RoundMode::Down); }

Value fnTrunc(Call &c)
{
    double x, p;
    if (!c.number(0, &x) || !c.numberOr(1, 0, &p)) return c.failure();
    p = std::trunc(p);
    if (std::fabs(p) > 1e6) p = p < 0 ? -1e6 : 1e6;
    double r;
    if (!roundDecimal(x, int(p), RoundMode::Down, &r)) return c.error(ErrorKind::Num);
    return Value::number(r);
}

Value fnCeiling(Call &c)
{
    double n, s;
    if (!c.number(0, &n) || !c.numberOr(1, 1, &s)) return c.failure();
    if (s == 0) return Value::number(0);
    if (n > 0 && s < 0) return c.error(ErrorKind::Num);
    return numberResult(std::ceil(roundTo15Digits(n / s)) * s);
}

Value fnFloor(Call &c)
{
    double n, s;
    if (!c.number(0, &n) || !c.numberOr(1, 1, &s)) return c.failure();
    if (s == 0) return c.error(ErrorKind::Div0);
    if (n > 0 && s < 0) return c.error(ErrorKind::Num);
    return numberResult(std::floor(roundTo15Digits(n / s)) * s);
}

Value fnEven(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    const double a = std::ceil(std::fabs(x) / 2) * 2;
    return numberResult(x < 0 ? -a : a);
}

Value fnOdd(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    double a = std::ceil(std::fabs(x));
    if (std::fmod(a, 2.0) == 0) a += 1;
    return numberResult(x < 0 ? -a : a);
}

Value fnFact(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    x = std::floor(x);
    if (x < 0 || x > 170) return c.error(ErrorKind::Num);
    double r = 1;
    for (int i = 2; i <= int(x); ++i) r *= i;
    return Value::number(r);
}

Value fnRand(Call &c)
{
    std::uniform_real_distribution<double> d(0.0, 1.0);
    return Value::number(d(c.machine().random()));
}

Value fnRandBetween(Call &c)
{
    double lo, hi;
    if (!c.number(0, &lo) || !c.number(1, &hi)) return c.failure();
    lo = std::ceil(lo);
    hi = std::floor(hi);
    if (lo > hi) return c.error(ErrorKind::Num);
    std::uniform_real_distribution<double> d(0.0, 1.0);
    return Value::number(std::min(hi, lo + std::floor(d(c.machine().random()) * (hi - lo + 1))));
}

Value gcdLcm(Call &c, bool lcm)
{
    double result = lcm ? 1 : 0;
    for (int i = 0; i < c.argc(); ++i) {
        bool failed = false;
        Value error;
        const bool done = c.each(i, [&](const Value &v, bool typed) {
            double x;
            ErrorKind e = ErrorKind::Value;
            if (!v.isNumber() && !(typed && coerceNumber(v, c.context().dateSystem, &x, &e))) {
                if (v.isError()) {
                    error = v;
                    failed = true;
                    return false;
                }
                if (typed) {
                    error = Value::error(e);
                    failed = true;
                    return false;
                }
                return true;
            }
            x = v.isNumber() ? v.number() : x;
            x = std::floor(x);
            if (x < 0 || x > 9007199254740991.0) {
                error = Value::error(ErrorKind::Num);
                failed = true;
                return false;
            }
            if (lcm) {
                if (x == 0) {
                    result = 0;
                } else if (result != 0) {
                    double a = result, b = x;
                    while (b != 0) {
                        const double t = std::fmod(a, b);
                        a = b;
                        b = t;
                    }
                    result = result / a * x;
                    // past what a number holds exactly it is no multiple to give (and an infinite one would never come to the end of the division of the next)
                    if (!(result <= 9007199254740991.0)) {
                        error = Value::error(ErrorKind::Num);
                        failed = true;
                        return false;
                    }
                }
            } else {
                double a = result, b = x;
                while (b != 0) {
                    const double t = std::fmod(a, b);
                    a = b;
                    b = t;
                }
                result = a;
            }
            return true;
        });
        if (!done) return failed ? error : c.error(ErrorKind::Value);
    }
    return numberResult(result);
}
Value fnGcd(Call &c) { return gcdLcm(c, false); }
Value fnLcm(Call &c) { return gcdLcm(c, true); }

Value fnCombin(Call &c)
{
    double n, k;
    if (!c.number(0, &n) || !c.number(1, &k)) return c.failure();
    n = std::trunc(n);
    k = std::trunc(k);
    if (n < 0 || k < 0 || k > n) return c.error(ErrorKind::Num);
    k = std::min(k, n - k);
    double r = 1;
    for (double i = 1; i <= k && std::isfinite(r); i += 1) r = r * (n - k + i) / i;
    return numberResult(std::round(r));
}

Value fnPermut(Call &c)
{
    double n, k;
    if (!c.number(0, &n) || !c.number(1, &k)) return c.failure();
    n = std::trunc(n);
    k = std::trunc(k);
    if (n < 0 || k < 0 || k > n) return c.error(ErrorKind::Num);
    double r = 1;
    for (double i = 0; i < k && std::isfinite(r); i += 1) r *= n - i;
    return numberResult(r);
}

Value fnMround(Call &c)
{
    double n, m;
    if (!c.number(0, &n) || !c.number(1, &m)) return c.failure();
    if (m == 0) return Value::number(0);
    if ((n > 0 && m < 0) || (n < 0 && m > 0)) return c.error(ErrorKind::Num);
    return numberResult(std::floor(std::fabs(roundTo15Digits(n / m)) + 0.5) * std::fabs(m) * (n < 0 ? -1 : 1));
}

double toRadians(double x) { return x * 3.14159265358979323846 / 180.0; }
double toDegrees(double x) { return x * 180.0 / 3.14159265358979323846; }

template <double (*F)(double)>
Value domain1(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    const double r = F(x);
    return std::isfinite(r) ? Value::number(r) : c.error(ErrorKind::Num);
}

double sinF(double x) { return std::sin(x); }
double cosF(double x) { return std::cos(x); }
double tanF(double x) { return std::tan(x); }
double asinF(double x) { return std::asin(x); }
double acosF(double x) { return std::acos(x); }
double atanF(double x) { return std::atan(x); }
double sinhF(double x) { return std::sinh(x); }
double coshF(double x) { return std::cosh(x); }
double tanhF(double x) { return std::tanh(x); }

Value fnAtan2(Call &c)
{
    double x, y;
    if (!c.number(0, &x) || !c.number(1, &y)) return c.failure();
    if (x == 0 && y == 0) return c.error(ErrorKind::Div0);
    return Value::number(std::atan2(y, x));
}

} // namespace

void registerMathFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"SUM", 1, 255, "r", 0, fnSum});
    r.push_back({"PRODUCT", 1, 255, "r", 0, fnProduct});
    r.push_back({"SUMSQ", 1, 255, "r", 0, fnSumsq});
    r.push_back({"MAX", 1, 255, "r", 0, fnMax});
    r.push_back({"MIN", 1, 255, "r", 0, fnMin});
    r.push_back({"MAXA", 1, 255, "r", 0, fnMaxA});
    r.push_back({"MINA", 1, 255, "r", 0, fnMinA});
    r.push_back({"AVERAGE", 1, 255, "r", 0, fnAverage});
    r.push_back({"AVERAGEA", 1, 255, "r", 0, fnAverageA});
    r.push_back({"COUNT", 1, 255, "r", 0, fnCount});
    r.push_back({"COUNTA", 1, 255, "r", 0, fnCountA});
    r.push_back({"COUNTBLANK", 1, 1, "n", 0, fnCountBlank});
    r.push_back({"ABS", 1, 1, "v", 0, fnAbs});
    r.push_back({"SQRT", 1, 1, "v", 0, fnSqrt});
    r.push_back({"EXP", 1, 1, "v", 0, fnExp});
    r.push_back({"LN", 1, 1, "v", 0, fnLn});
    r.push_back({"LOG", 1, 2, "vv", 0, fnLog});
    r.push_back({"LOG10", 1, 1, "v", 0, fnLog10});
    r.push_back({"SIGN", 1, 1, "v", 0, fnSign});
    r.push_back({"INT", 1, 1, "v", 0, fnInt});
    r.push_back({"PI", 0, 0, "", 0, fnPi});
    r.push_back({"POWER", 2, 2, "vv", 0, fnPower});
    r.push_back({"MOD", 2, 2, "vv", 0, fnMod});
    r.push_back({"QUOTIENT", 2, 2, "vv", 0, fnQuotient});
    r.push_back({"ROUND", 2, 2, "vv", 0, fnRound});
    r.push_back({"ROUNDUP", 2, 2, "vv", 0, fnRoundUp});
    r.push_back({"ROUNDDOWN", 2, 2, "vv", 0, fnRoundDown});
    r.push_back({"TRUNC", 1, 2, "vv", 0, fnTrunc});
    r.push_back({"CEILING", 1, 2, "vv", 0, fnCeiling});
    r.push_back({"FLOOR", 1, 2, "vv", 0, fnFloor});
    r.push_back({"EVEN", 1, 1, "v", 0, fnEven});
    r.push_back({"ODD", 1, 1, "v", 0, fnOdd});
    r.push_back({"FACT", 1, 1, "v", 0, fnFact});
    r.push_back({"RAND", 0, 0, "", kFnVolatile, fnRand});
    r.push_back({"RANDBETWEEN", 2, 2, "vv", kFnVolatile, fnRandBetween});
    r.push_back({"GCD", 1, 255, "r", 0, fnGcd});
    r.push_back({"LCM", 1, 255, "r", 0, fnLcm});
    r.push_back({"COMBIN", 2, 2, "vv", 0, fnCombin});
    r.push_back({"PERMUT", 2, 2, "vv", 0, fnPermut});
    r.push_back({"MROUND", 2, 2, "vv", 0, fnMround});
    r.push_back({"SIN", 1, 1, "v", 0, domain1<sinF>});
    r.push_back({"COS", 1, 1, "v", 0, domain1<cosF>});
    r.push_back({"TAN", 1, 1, "v", 0, domain1<tanF>});
    r.push_back({"ASIN", 1, 1, "v", 0, domain1<asinF>});
    r.push_back({"ACOS", 1, 1, "v", 0, domain1<acosF>});
    r.push_back({"ATAN", 1, 1, "v", 0, domain1<atanF>});
    r.push_back({"SINH", 1, 1, "v", 0, domain1<sinhF>});
    r.push_back({"COSH", 1, 1, "v", 0, domain1<coshF>});
    r.push_back({"TANH", 1, 1, "v", 0, domain1<tanhF>});
    r.push_back({"ATAN2", 2, 2, "vv", 0, fnAtan2});
    r.push_back({"RADIANS", 1, 1, "v", 0, domain1<toRadians>});
    r.push_back({"DEGREES", 1, 1, "v", 0, domain1<toDegrees>});
}

} // namespace jp::calc
