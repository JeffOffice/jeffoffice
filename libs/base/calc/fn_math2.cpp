// More number functions: the hyperbolic and reciprocal trigonometry, Roman and other bases, matrices, series sums, rounding to a multiple that ignores signs,
// AGGREGATE, and the functions that make arrays of numbers.
#include "calc/machine.h"

#include <QString>

#include <algorithm>
#include <cmath>
#include <random>

namespace jp::calc {

namespace {

constexpr double kPi = 3.14159265358979323846;

template <double (*F)(double)>
Value domain(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    const double r = F(x);
    return std::isfinite(r) ? Value::number(r) : c.error(ErrorKind::Num);
}

// 1 / f(x), with #DIV/0! where f(x) is zero.
template <double (*F)(double)>
Value reciprocal(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    if (std::fabs(x) >= 134217728.0) return c.error(ErrorKind::Num);   // (2^27, as the trigonometric functions of the original)
    const double d = F(x);
    if (d == 0) return c.error(ErrorKind::Div0);
    return numberResult(1 / d);
}

double sinD(double x) { return std::sin(x); }
double cosD(double x) { return std::cos(x); }
double tanD(double x) { return std::tan(x); }
double sinhD(double x) { return std::sinh(x); }
double coshD(double x) { return std::cosh(x); }
double tanhD(double x) { return std::tanh(x); }

double acoshF(double x) { return x < 1 ? NAN : std::acosh(x); }
double asinhF(double x) { return std::asinh(x); }
double atanhF(double x) { return std::fabs(x) >= 1 ? NAN : std::atanh(x); }
double acotF(double x) { return kPi / 2 - std::atan(x); }
double acothF(double x) { return std::fabs(x) <= 1 ? NAN : 0.5 * std::log((x + 1) / (x - 1)); }
double sqrtPiF(double x) { return x < 0 ? NAN : std::sqrt(x * kPi); }

Value fnFactDouble(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    x = std::trunc(x);
    if (x < -1 || x > 300) return c.error(ErrorKind::Num);
    double r = 1;
    for (int i = int(x); i > 1; i -= 2) r *= i;
    return numberResult(r);
}

Value fnMultinomial(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    double sum = 0, denominator = 1;
    for (double v : x) {
        v = std::trunc(v);
        if (v < 0) return c.error(ErrorKind::Num);
        sum += v;
        if (sum > 170) return c.error(ErrorKind::Num);
        for (int i = 2; i <= int(v); ++i) denominator *= i;
    }
    double top = 1;
    for (int i = 2; i <= int(sum); ++i) top *= i;
    return numberResult(std::round(top / denominator));
}

// ---- Roman numbers

QString romanOf(int value, int mode)
{
    static const char chars[] = "MDCLXVI";
    static const int values[] = {1000, 500, 100, 50, 10, 5, 1};
    const int last = 6;
    QString out;
    for (int i = 0; i <= last / 2; ++i) {
        int index = 2 * i;
        const int digit = value / values[index];
        if (digit % 5 == 4) {
            const int index2 = digit == 4 ? index - 1 : index - 2;
            int steps = 0;
            while (steps < mode && index < last) {
                ++steps;
                if (values[index2] - values[index + 1] <= value) ++index;
                else steps = mode;
            }
            out += QLatin1Char(chars[index]);
            out += QLatin1Char(chars[index2]);
            value += values[index];
            value -= values[index2];
        } else {
            if (digit > 4) out += QLatin1Char(chars[index - 1]);
            for (int k = digit % 5; k > 0; --k) out += QLatin1Char(chars[index]);
            value %= values[index];
        }
    }
    return out;
}

Value fnRoman(Call &c)
{
    double n, form = 0;
    if (!c.number(0, &n)) return c.failure();
    if (!c.missing(1)) {
        const Value v = c.scalar(1);
        if (v.isBool()) form = v.boolean() ? 0 : 4;
        else if (!c.number(1, &form)) return c.failure();
    }
    n = std::trunc(n);
    form = std::trunc(form);
    if (n < 0 || n > 3999 || form < 0 || form > 4) return c.error(ErrorKind::Value);
    return Value::text(romanOf(int(n), int(form)));
}

Value fnArabic(Call &c)
{
    QString t;
    if (!c.text(0, &t)) return c.failure();
    t = t.trimmed().toUpper();
    if (t.isEmpty()) return Value::number(0);
    bool negative = false;
    if (t.startsWith(QLatin1Char('-'))) {
        negative = true;
        t.remove(0, 1);
    }
    auto digit = [](QChar ch) {
        switch (ch.unicode()) {
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default: return 0;
        }
    };
    if (t.isEmpty() || t.size() > 255) return c.error(ErrorKind::Value);
    double total = 0;
    int highest = 0;
    for (int i = int(t.size()) - 1; i >= 0; --i) {
        const int d = digit(t[i]);
        if (d == 0) return c.error(ErrorKind::Value);
        if (d >= highest) {
            total += d;
            highest = d;
        } else total -= d;
    }
    return Value::number(negative ? -total : total);
}

Value fnBase(Call &c)
{
    double n, radix, minLength = 0;
    if (!c.number(0, &n) || !c.number(1, &radix) || !c.numberOr(2, 0, &minLength)) return c.failure();
    n = std::trunc(n);
    radix = std::trunc(radix);
    minLength = std::trunc(minLength);
    if (n < 0 || n >= 9007199254740992.0 || radix < 2 || radix > 36 || minLength < 0 || minLength > 255) return c.error(ErrorKind::Num);
    QString s = QString::number(qulonglong(n), int(radix)).toUpper();
    while (s.size() < int(minLength)) s.prepend(QLatin1Char('0'));
    return Value::text(s);
}

Value fnDecimal(Call &c)
{
    QString t;
    double radix;
    if (!c.text(0, &t) || !c.number(1, &radix)) return c.failure();
    radix = std::trunc(radix);
    if (radix < 2 || radix > 36) return c.error(ErrorKind::Num);
    t = t.trimmed();
    if (t.isEmpty()) return Value::number(0);
    if (t.size() > 255) return c.error(ErrorKind::Value);
    double total = 0;
    for (const QChar ch : t) {
        int d;
        const ushort u = ch.toUpper().unicode();
        if (u >= '0' && u <= '9') d = u - '0';
        else if (u >= 'A' && u <= 'Z') d = u - 'A' + 10;
        else return c.error(ErrorKind::Num);
        if (d >= radix) return c.error(ErrorKind::Num);
        total = total * radix + d;
    }
    return numberResult(total);
}

Value fnCombinA(Call &c)
{
    double n, k;
    if (!c.number(0, &n) || !c.number(1, &k)) return c.failure();
    n = std::trunc(n);
    k = std::trunc(k);
    if (n < 0 || k < 0) return c.error(ErrorKind::Num);
    if (k == 0) return Value::number(1);
    if (n == 0) return c.error(ErrorKind::Num);
    const double m = n + k - 1;
    const double j = std::min(k, n - 1);
    if (!c.machine().chargeCount(j)) return c.error(ErrorKind::Value);
    double r = 1;
    for (double i = 1; i <= j; ++i) r = r * (m - j + i) / i;
    return numberResult(std::round(r));
}

// ---- rounding to a multiple, the sign of the multiple ignored

Value toMultiple(Call &c, bool up)
{
    double x, sig = 1;
    if (!c.number(0, &x) || !c.numberOr(1, 1, &sig)) return c.failure();
    sig = std::fabs(sig);
    if (sig == 0) return Value::number(0);
    const double q = x / sig;
    double rounded = up ? std::ceil(q) : std::floor(q);
    // (a quotient that is a whole number but for the last digits stays as it is)
    if (std::fabs(q - std::round(q)) < 1e-12 * std::max(1.0, std::fabs(q))) rounded = std::round(q);
    return numberResult(rounded * sig);
}
Value fnCeilingPrecise(Call &c) { return toMultiple(c, true); }
Value fnFloorPrecise(Call &c) { return toMultiple(c, false); }

// ---- sums over two series

Value pairSums(Call &c, int kind)
{
    Value a, b;
    if (!c.array(0, &a) || !c.array(1, &b)) return c.failure();
    if (a.rows() != b.rows() || a.cols() != b.cols()) return c.error(ErrorKind::NA);
    double sum = 0;
    for (int i = 0; i < a.rows(); ++i)
        for (int j = 0; j < a.cols(); ++j) {
            const Value &x = a.at(i, j), &y = b.at(i, j);
            if (x.isError()) return x;
            if (y.isError()) return y;
            if (!x.isNumber() || !y.isNumber()) continue;
            const double p = x.number(), q = y.number();
            sum += kind == 0 ? p * p - q * q : kind == 1 ? p * p + q * q : (p - q) * (p - q);
        }
    return numberResult(sum);
}
Value fnSumX2MY2(Call &c) { return pairSums(c, 0); }
Value fnSumX2PY2(Call &c) { return pairSums(c, 1); }
Value fnSumXMY2(Call &c) { return pairSums(c, 2); }

Value fnSeriesSum(Call &c)
{
    double x, n, m;
    if (!c.number(0, &x) || !c.number(1, &n) || !c.number(2, &m)) return c.failure();
    Value coefficients;
    if (!c.array(3, &coefficients)) return c.failure();
    double sum = 0;
    int k = 0;
    for (int i = 0; i < coefficients.rows(); ++i)
        for (int j = 0; j < coefficients.cols(); ++j, ++k) {
            const Value &v = coefficients.at(i, j);
            if (v.isError()) return v;
            if (!v.isNumber()) return c.error(ErrorKind::Value);
            if (x == 0 && n + k * m == 0) sum += v.number();   // (0^0 as the term of a series is 1)
            else sum += v.number() * powerOf(x, n + k * m);
        }
    return numberResult(sum);
}

// ---- matrices

bool matrixOf(Call &c, int arg, std::vector<double> *m, int *rows, int *cols)
{
    Value a;
    if (!c.array(arg, &a)) return false;
    *rows = a.rows();
    *cols = a.cols();
    m->resize(size_t(*rows) * size_t(*cols));
    for (int i = 0; i < *rows; ++i)
        for (int j = 0; j < *cols; ++j) {
            const Value &v = a.at(i, j);
            if (v.isError()) {
                c.error(v.errorKind());
                return false;
            }
            if (!v.isNumber()) {
                c.error(ErrorKind::Value);
                return false;
            }
            (*m)[size_t(i) * size_t(*cols) + size_t(j)] = v.number();
        }
    return true;
}

// LU decomposition with partial pivoting, in place; the determinant, or false when the matrix is singular. `perm` is the row order.
bool luDecompose(std::vector<double> &a, int n, std::vector<int> *perm, double *det)
{
    perm->resize(size_t(n));
    for (int i = 0; i < n; ++i) (*perm)[size_t(i)] = i;
    double d = 1;
    double scale = 0;
    for (const double v : a) scale = std::max(scale, std::fabs(v));
    bool singular = false;
    for (int k = 0; k < n; ++k) {
        int p = k;
        for (int i = k + 1; i < n; ++i)
            if (std::fabs(a[size_t(i) * n + k]) > std::fabs(a[size_t(p) * n + k])) p = i;
        if (std::fabs(a[size_t(p) * n + k]) <= scale * 1e-15 * n) singular = true;
        if (p != k) {
            for (int j = 0; j < n; ++j) std::swap(a[size_t(p) * n + j], a[size_t(k) * n + j]);
            std::swap((*perm)[size_t(p)], (*perm)[size_t(k)]);
            d = -d;
        }
        const double pivot = a[size_t(k) * n + k];
        d *= pivot;
        if (pivot == 0) continue;
        for (int i = k + 1; i < n; ++i) {
            const double f = a[size_t(i) * n + k] / pivot;
            a[size_t(i) * n + k] = f;
            for (int j = k + 1; j < n; ++j) a[size_t(i) * n + j] -= f * a[size_t(k) * n + j];
        }
    }
    *det = d;
    return !singular;
}

Value fnMDeterm(Call &c)
{
    std::vector<double> m;
    int rows, cols;
    if (!matrixOf(c, 0, &m, &rows, &cols)) return c.failure();
    if (rows != cols || rows > 256) return c.error(ErrorKind::Value);
    std::vector<int> perm;
    double det;
    luDecompose(m, rows, &perm, &det);
    return numberResult(det);
}

Value fnMInverse(Call &c)
{
    std::vector<double> m;
    int rows, cols;
    if (!matrixOf(c, 0, &m, &rows, &cols)) return c.failure();
    if (rows != cols || rows > 256) return c.error(ErrorKind::Value);
    const int n = rows;
    if (!c.machine().charge(qint64(n) * n * n)) return c.error(ErrorKind::Value);
    std::vector<int> perm;
    double det;
    if (!luDecompose(m, n, &perm, &det) || det == 0) return c.error(ErrorKind::Num);
    Value out = c.machine().newArray(n, n);
    if (!out.isArray()) return out;
    std::vector<double> col(static_cast<size_t>(n));
    for (int j = 0; j < n; ++j) {
        // solve A x = e_j, with the rows in the order of the pivoting
        for (int i = 0; i < n; ++i) col[size_t(i)] = perm[size_t(i)] == j ? 1 : 0;
        for (int i = 0; i < n; ++i)
            for (int k = 0; k < i; ++k) col[size_t(i)] -= m[size_t(i) * n + k] * col[size_t(k)];
        for (int i = n - 1; i >= 0; --i) {
            for (int k = i + 1; k < n; ++k) col[size_t(i)] -= m[size_t(i) * n + k] * col[size_t(k)];
            col[size_t(i)] /= m[size_t(i) * n + i];
        }
        for (int i = 0; i < n; ++i) {
            if (!std::isfinite(col[size_t(i)])) return c.error(ErrorKind::Num);
            out.set(i, j, Value::number(col[size_t(i)]));
        }
    }
    return out;
}

Value fnMMult(Call &c)
{
    std::vector<double> a, b;
    int ar, ac, br, bc;
    if (!matrixOf(c, 0, &a, &ar, &ac) || !matrixOf(c, 1, &b, &br, &bc)) return c.failure();
    if (ac != br) return c.error(ErrorKind::Value);
    if (!c.machine().charge(qint64(ar) * ac * bc)) return c.error(ErrorKind::Value);
    Value out = c.machine().newArray(ar, bc);
    if (!out.isArray()) return out;
    for (int i = 0; i < ar; ++i)
        for (int j = 0; j < bc; ++j) {
            double s = 0;
            for (int k = 0; k < ac; ++k) s += a[size_t(i) * ac + k] * b[size_t(k) * bc + j];
            if (!std::isfinite(s)) return c.error(ErrorKind::Num);
            out.set(i, j, Value::number(s));
        }
    return out;
}

Value fnMUnit(Call &c)
{
    double n;
    if (!c.number(0, &n)) return c.failure();
    n = std::trunc(n);
    if (n < 1 || n > 1000) return c.error(ErrorKind::Value);
    if (!c.machine().charge(qint64(n) * qint64(n))) return c.error(ErrorKind::Value);
    Value out = c.machine().newArray(int(n), int(n));
    if (!out.isArray()) return out;
    for (int i = 0; i < int(n); ++i)
        for (int j = 0; j < int(n); ++j) out.set(i, j, Value::number(i == j ? 1 : 0));
    return out;
}

// ---- arrays of numbers

Value fnSequence(Call &c)
{
    double rows, cols = 1, start = 1, step = 1;
    if (!c.number(0, &rows) || !c.numberOr(1, 1, &cols) || !c.numberOr(2, 1, &start) || !c.numberOr(3, 1, &step)) return c.failure();
    rows = std::trunc(rows);
    cols = std::trunc(cols);
    if (rows < 1 || cols < 1) return c.error(ErrorKind::Calc);
    if (rows * cols > double(kMaxArrayCells) || !c.machine().charge(qint64(rows * cols))) return c.error(ErrorKind::Num);
    Value out = c.machine().newArray(int(rows), int(cols));
    if (!out.isArray()) return out;
    double v = start;
    for (int i = 0; i < int(rows); ++i)
        for (int j = 0; j < int(cols); ++j, v += step) out.set(i, j, numberResult(v));
    return out;
}

Value fnRandArray(Call &c)
{
    double rows = 1, cols = 1, low = 0, high = 1;
    bool whole = false;
    if (!c.numberOr(0, 1, &rows) || !c.numberOr(1, 1, &cols) || !c.numberOr(2, 0, &low) || !c.numberOr(3, 1, &high) || !c.booleanOr(4, false, &whole)) return c.failure();
    rows = std::trunc(rows);
    cols = std::trunc(cols);
    if (rows < 1 || cols < 1 || low > high) return c.error(ErrorKind::Value);
    if (whole) {
        low = std::ceil(low);
        high = std::floor(high);
        if (low > high) return c.error(ErrorKind::Value);
    }
    if (rows * cols > double(kMaxArrayCells) || !c.machine().charge(qint64(rows * cols))) return c.error(ErrorKind::Num);
    Value out = c.machine().newArray(int(rows), int(cols));
    if (!out.isArray()) return out;
    for (int i = 0; i < int(rows); ++i)
        for (int j = 0; j < int(cols); ++j) {
            std::uniform_real_distribution<double> d(0.0, 1.0);
            const double u = d(c.machine().random());
            out.set(i, j, numberResult(whole ? low + std::floor(u * (high - low + 1)) : low + u * (high - low)));
        }
    return out;
}

// ---- AGGREGATE

Value fnAggregate(Call &c)
{
    double f, options;
    if (!c.number(0, &f) || !c.number(1, &options)) return c.failure();
    if (!(std::fabs(f) < 100) || !(std::fabs(options) < 100)) return c.error(ErrorKind::Value);
    const int fn = int(std::trunc(f)), opt = int(std::trunc(options));
    if (fn < 1 || fn > 19 || opt < 0 || opt > 7) return c.error(ErrorKind::Value);
    if (c.argc() < 3 || (fn >= 14 && c.argc() != 4) || (fn < 14 && c.argc() < 3)) return c.error(ErrorKind::Value);
    const bool skipHidden = opt == 1 || opt == 3 || opt == 5 || opt == 7, skipErrors = opt >= 2 && opt != 4 && opt != 5, skipNested = opt <= 3;
    const int last = fn >= 14 ? 3 : c.argc();
    std::vector<double> x;
    qint64 nonEmpty = 0;
    for (int i = 2; i < last; ++i) {
        bool failed = false;
        Value problem;
        const bool typedArgument = !c.missing(i) && c.item(i).kind == Item::Kind::Value && !c.item(i).value.isArray();
        const bool done = c.eachCell(i, [&](const Value &v, int sheet, quint32 row, quint32 col) {
            if (sheet >= 0 && skipNested && c.machine().source().isSubtotal(sheet, row, col)) return true;
            if (sheet >= 0 && skipHidden && c.machine().source().rowHiddenKind(sheet, row) >= 1) return true;
            if (v.isError()) {
                if (skipErrors) return true;
                if (fn == 3) {
                    ++nonEmpty;
                    return true;
                }
                problem = v;
                failed = true;
                return false;
            }
            if (!v.isEmpty()) ++nonEmpty;
            if (v.isNumber()) x.push_back(v.number());
            else if (typedArgument && v.isBool()) x.push_back(v.number());
            else if (typedArgument && v.isText() && fn != 3) {
                double d;
                if (!textToNumber(v.text(), c.context().dateSystem, &d)) {
                    problem = Value::error(ErrorKind::Value);
                    failed = true;
                    return false;
                }
                x.push_back(d);
            }
            return true;
        });
        if (failed) return problem;
        if (!done) return c.error(ErrorKind::Value);
    }
    const size_t n = x.size();
    double sum = 0;
    for (const double v : x) sum += v;
    auto spread = [&](bool sample, bool root) -> Value {
        if (sample ? n < 2 : n < 1) return c.error(ErrorKind::Div0);
        const double mean = sum / double(n);
        double ss = 0;
        for (const double v : x) ss += (v - mean) * (v - mean);
        const double var = ss / double(sample ? n - 1 : n);
        return numberResult(root ? std::sqrt(var) : var);
    };
    auto quantile = [&](double k, bool exclusive) -> Value {
        if (n == 0) return c.error(ErrorKind::Num);
        std::sort(x.begin(), x.end());
        if (exclusive) {
            if (!(k > 0 && k < 1)) return c.error(ErrorKind::Num);
            const double pos = k * double(n + 1);
            if (pos < 1 || pos > double(n)) return c.error(ErrorKind::Num);
            const size_t lo = size_t(std::floor(pos));
            return numberResult(lo >= n ? x.back() : x[lo - 1] + (x[lo] - x[lo - 1]) * (pos - double(lo)));
        }
        if (!(k >= 0 && k <= 1)) return c.error(ErrorKind::Num);
        const double pos = k * double(n - 1);
        const size_t lo = size_t(std::floor(pos));
        return lo + 1 >= n ? Value::number(x.back()) : numberResult(x[lo] + (x[lo + 1] - x[lo]) * (pos - double(lo)));
    };
    switch (fn) {
    case 1: return n ? numberResult(sum / double(n)) : c.error(ErrorKind::Div0);
    case 2: return Value::number(double(n));
    case 3: return Value::number(double(nonEmpty));
    case 4: return n ? Value::number(*std::max_element(x.begin(), x.end())) : Value::number(0);
    case 5: return n ? Value::number(*std::min_element(x.begin(), x.end())) : Value::number(0);
    case 6: {
        double p = 1;
        for (const double v : x) p *= v;
        return numberResult(n ? p : 0);
    }
    case 7: return spread(true, true);
    case 8: return spread(false, true);
    case 9: return numberResult(sum);
    case 10: return spread(true, false);
    case 11: return spread(false, false);
    case 12: {
        if (n == 0) return c.error(ErrorKind::Num);
        std::sort(x.begin(), x.end());
        return numberResult(n % 2 ? x[n / 2] : (x[n / 2 - 1] + x[n / 2]) / 2);
    }
    default: break;
    }
    double k;
    if (!c.number(3, &k)) return c.failure();
    switch (fn) {
    case 13: {
        // MODE.SNGL
        std::vector<double> sorted = x;
        std::sort(sorted.begin(), sorted.end());
        size_t best = 1;
        for (size_t i = 0; i < n;) {
            size_t j = i;
            while (j < n && sorted[j] == sorted[i]) ++j;
            best = std::max(best, j - i);
            i = j;
        }
        if (best < 2) return c.error(ErrorKind::NA);
        for (const double v : x)
            if (size_t(std::count(sorted.begin(), sorted.end(), v)) == best) return Value::number(v);
        return c.error(ErrorKind::NA);
    }
    case 14:
    case 15: {
        const double kk = std::trunc(k);
        if (kk < 1 || kk > double(n)) return c.error(ErrorKind::Num);
        std::sort(x.begin(), x.end());
        return Value::number(fn == 14 ? x[n - size_t(kk)] : x[size_t(kk) - 1]);
    }
    case 16: return quantile(k, false);
    case 17: {
        const double q = std::trunc(k);
        if (q < 0 || q > 4) return c.error(ErrorKind::Num);
        return quantile(q / 4, false);
    }
    case 18: return quantile(k, true);
    default: {
        const double q = std::trunc(k);
        if (q < 1 || q > 3) return c.error(ErrorKind::Num);
        return quantile(q / 4, true);
    }
    }
}

} // namespace

void registerMoreMathFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"ACOSH", 1, 1, "v", 0, domain<acoshF>});
    r.push_back({"ASINH", 1, 1, "v", 0, domain<asinhF>});
    r.push_back({"ATANH", 1, 1, "v", 0, domain<atanhF>});
    r.push_back({"ACOT", 1, 1, "v", kFnFuture, domain<acotF>});
    r.push_back({"ACOTH", 1, 1, "v", kFnFuture, domain<acothF>});
    r.push_back({"COT", 1, 1, "v", kFnFuture, reciprocal<tanD>});
    r.push_back({"COTH", 1, 1, "v", kFnFuture, reciprocal<tanhD>});
    r.push_back({"CSC", 1, 1, "v", kFnFuture, reciprocal<sinD>});
    r.push_back({"CSCH", 1, 1, "v", kFnFuture, reciprocal<sinhD>});
    r.push_back({"SEC", 1, 1, "v", kFnFuture, reciprocal<cosD>});
    r.push_back({"SECH", 1, 1, "v", kFnFuture, reciprocal<coshD>});
    r.push_back({"SQRTPI", 1, 1, "v", 0, domain<sqrtPiF>});
    r.push_back({"FACTDOUBLE", 1, 1, "v", 0, fnFactDouble});
    r.push_back({"MULTINOMIAL", 1, 255, "r", 0, fnMultinomial});
    r.push_back({"ROMAN", 1, 2, "vv", 0, fnRoman});
    r.push_back({"ARABIC", 1, 1, "v", kFnFuture, fnArabic});
    r.push_back({"BASE", 2, 3, "vvv", kFnFuture, fnBase});
    r.push_back({"DECIMAL", 2, 2, "vv", kFnFuture, fnDecimal});
    r.push_back({"COMBINA", 2, 2, "vv", kFnFuture, fnCombinA});
    r.push_back({"CEILING.PRECISE", 1, 2, "vv", kFnFuture, fnCeilingPrecise});
    r.push_back({"ISO.CEILING", 1, 2, "vv", kFnFuture, fnCeilingPrecise});
    r.push_back({"FLOOR.PRECISE", 1, 2, "vv", kFnFuture, fnFloorPrecise});
    r.push_back({"SUMX2MY2", 2, 2, "aa", 0, fnSumX2MY2});
    r.push_back({"SUMX2PY2", 2, 2, "aa", 0, fnSumX2PY2});
    r.push_back({"SUMXMY2", 2, 2, "aa", 0, fnSumXMY2});
    r.push_back({"SERIESSUM", 4, 4, "vvva", 0, fnSeriesSum});
    r.push_back({"MDETERM", 1, 1, "a", 0, fnMDeterm});
    r.push_back({"MINVERSE", 1, 1, "a", 0, fnMInverse});
    r.push_back({"MMULT", 2, 2, "aa", 0, fnMMult});
    r.push_back({"MUNIT", 1, 1, "v", kFnFuture, fnMUnit});
    r.push_back({"SEQUENCE", 1, 4, "vvvv", kFnFuture, fnSequence});
    r.push_back({"RANDARRAY", 0, 5, "vvvvv", kFnFuture | kFnVolatile, fnRandArray});
    r.push_back({"AGGREGATE", 3, 255, "vvr", kFnFuture, fnAggregate});
}

} // namespace jp::calc
