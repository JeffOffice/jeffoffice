// The engineering functions: numbers in other bases, bit operations, the error function, complex numbers, Bessel functions, and CONVERT.
#include "calc/machine.h"

#include <QHash>
#include <QRegularExpression>
#include <QString>

#include <algorithm>
#include <cmath>
#include <complex>

namespace jp::calc {

namespace {

constexpr double kPi = 3.14159265358979323846;

// ---- numbers in other bases

// The digits of argument i as the text of a number in `radix` (a number is written out as it is in General format).
bool digitsArg(Call &c, int i, QString *out)
{
    const Value v = c.scalar(i);
    if (v.isError()) {
        c.error(v.errorKind());
        return false;
    }
    if (v.isBool()) {
        c.error(ErrorKind::Value);
        return false;
    }
    if (v.isNumber()) {
        if (v.number() < 0 || v.number() != std::floor(v.number()) || v.number() >= 1e15) {
            c.error(ErrorKind::Num);
            return false;
        }
        *out = QString::number(qlonglong(v.number()));
        return true;
    }
    if (v.isEmpty()) {
        *out = QString();
        return true;
    }
    return c.text(i, out);
}

// The value of up to ten digits in a radix, with the leading 1 of a full-length number as the sign (ten digits in two's complement).
bool parseBased(Call &c, const QString &digits, int radix, int bitsPerDigit, qint64 *out)
{
    if (digits.size() > 10) {
        c.error(ErrorKind::Num);
        return false;
    }
    qint64 v = 0;
    for (const QChar ch : digits) {
        const ushort u = ch.toUpper().unicode();
        int d;
        if (u >= '0' && u <= '9') d = u - '0';
        else if (u >= 'A' && u <= 'F') d = u - 'A' + 10;
        else {
            c.error(ErrorKind::Num);
            return false;
        }
        if (d >= radix) {
            c.error(ErrorKind::Num);
            return false;
        }
        v = v * radix + d;
    }
    const int bits = 10 * bitsPerDigit;
    if (digits.size() == 10 && v >= (qint64(1) << (bits - 1))) v -= qint64(1) << bits;
    *out = v;
    return true;
}

// The text of v in a radix: negative numbers as ten digits of two's complement; `places` pads a positive one.
bool formatBased(Call &c, qint64 v, int radix, int bitsPerDigit, int placesArg, QString *out)
{
    const int bits = 10 * bitsPerDigit;
    const qint64 limit = qint64(1) << (bits - 1);
    if (v < -limit || v >= limit) {
        c.error(ErrorKind::Num);
        return false;
    }
    double places = 0;
    const bool hasPlaces = !c.missing(placesArg);
    if (hasPlaces) {
        if (!c.number(placesArg, &places)) return false;
        places = std::trunc(places);
        if (places < 1 || places > 10) {
            c.error(ErrorKind::Num);
            return false;
        }
    }
    if (v < 0) {
        *out = QString::number(qulonglong(v + (qint64(1) << bits)), radix).toUpper();
        return true;
    }
    QString s = QString::number(qulonglong(v), radix).toUpper();
    if (hasPlaces) {
        if (s.size() > int(places)) {
            c.error(ErrorKind::Num);
            return false;
        }
        while (s.size() < int(places)) s.prepend(QLatin1Char('0'));
    }
    *out = s;
    return true;
}

int bitsOf(int radix) { return radix == 2 ? 1 : radix == 8 ? 3 : 4; }

template <int From, int To>
Value convertBase(Call &c)
{
    qint64 v;
    if (From == 10) {
        double d;
        if (!c.number(0, &d)) return c.failure();
        d = std::trunc(d);
        if (std::fabs(d) > 1e15) return c.error(ErrorKind::Num);
        v = qint64(d);
    } else {
        QString digits;
        if (!digitsArg(c, 0, &digits) || !parseBased(c, digits, From, bitsOf(From), &v)) return c.failure();
    }
    if (To == 10) return Value::number(double(v));
    QString s;
    if (!formatBased(c, v, To, bitsOf(To), 1, &s)) return c.failure();
    return Value::text(s);
}

// ---- bits

bool bitArg(Call &c, int i, qint64 *out)
{
    double d;
    if (!c.number(i, &d)) return false;
    if (d < 0 || d != std::floor(d) || d >= 281474976710656.0) {
        c.error(ErrorKind::Num);
        return false;
    }
    *out = qint64(d);
    return true;
}

template <int Op>
Value bitwise(Call &c)
{
    qint64 a, b;
    if (!bitArg(c, 0, &a) || !bitArg(c, 1, &b)) return c.failure();
    return Value::number(double(Op == 0 ? (a & b) : Op == 1 ? (a | b) : (a ^ b)));
}

template <bool Left>
Value shifted(Call &c)
{
    qint64 a;
    double shift;
    if (!bitArg(c, 0, &a) || !c.number(1, &shift)) return c.failure();
    shift = std::trunc(shift);
    if (std::fabs(shift) > 53) return c.error(ErrorKind::Num);
    const bool left = Left == (shift >= 0);
    const int by = int(std::fabs(shift));
    double r = left ? double(a) * std::pow(2.0, by) : std::floor(double(a) / std::pow(2.0, by));
    if (r >= 281474976710656.0) return c.error(ErrorKind::Num);
    return Value::number(r);
}

Value fnDelta(Call &c)
{
    double a, b = 0;
    if (!c.number(0, &a) || !c.numberOr(1, 0, &b)) return c.failure();
    return Value::number(a == b ? 1 : 0);
}

Value fnGestep(Call &c)
{
    double a, b = 0;
    if (!c.number(0, &a) || !c.numberOr(1, 0, &b)) return c.failure();
    return Value::number(a >= b ? 1 : 0);
}

// ---- the error function

Value fnErf(Call &c)
{
    double lower, upper;
    if (!c.number(0, &lower)) return c.failure();
    if (c.missing(1)) return numberResult(std::erf(lower));
    if (!c.number(1, &upper)) return c.failure();
    return numberResult(std::erf(upper) - std::erf(lower));
}

Value fnErfPrecise(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    return numberResult(std::erf(x));
}

Value fnErfc(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    return numberResult(std::erfc(x));
}

// ---- complex numbers

using Complex = std::complex<double>;

struct ComplexValue {
    Complex z;
    QChar suffix = QLatin1Char('i');
    bool hasSuffix = false;   // the number was written with a suffix (so the other operands must agree)
};

// Reads "3+4i", "-j", "5", "2.5e3-1e-2i": a real part and an imaginary part, each optional, the suffix i or j.
bool parseComplex(const QString &text, ComplexValue *out)
{
    QString t = text;
    t.remove(QLatin1Char(' '));
    if (t.isEmpty()) {
        out->z = 0;
        return true;
    }
    static const QRegularExpression number(QStringLiteral("^[+-]?(\\d+\\.?\\d*|\\.\\d+)([eE][+-]?\\d+)?"));
    auto readNumber = [&](int from, double *v, int *end) {
        const QRegularExpressionMatch m = number.match(QStringView(t).mid(from).toString());
        if (!m.hasMatch()) return false;
        *v = m.captured().toDouble();
        *end = from + int(m.capturedLength());
        return true;
    };
    double re = 0;
    int pos = 0;
    const int n = int(t.size());
    auto isSuffix = [&](int p) { return p < n && (t[p] == QLatin1Char('i') || t[p] == QLatin1Char('j')); };
    // a bare imaginary unit, with a sign or none
    auto unit = [&](int p, double *sign, int *end) {
        int q = p;
        double s = 1;
        if (q < n && (t[q] == QLatin1Char('+') || t[q] == QLatin1Char('-'))) {
            s = t[q] == QLatin1Char('-') ? -1 : 1;
            ++q;
        }
        if (!isSuffix(q)) return false;
        *sign = s;
        *end = q + 1;
        out->suffix = t[q];
        out->hasSuffix = true;
        return true;
    };
    double sign;
    int end;
    if (unit(0, &sign, &end) && end == n) {
        out->z = Complex(0, sign);
        return true;
    }
    double v;
    if (!readNumber(0, &v, &end)) return false;
    if (isSuffix(end)) {
        // an imaginary number alone, or followed by nothing
        if (end + 1 != n) return false;
        out->suffix = t[end];
        out->hasSuffix = true;
        out->z = Complex(0, v);
        return true;
    }
    re = v;
    pos = end;
    if (pos == n) {
        out->z = Complex(re, 0);
        return true;
    }
    if (t[pos] != QLatin1Char('+') && t[pos] != QLatin1Char('-')) return false;
    if (unit(pos, &sign, &end)) {
        if (end != n) return false;
        out->z = Complex(re, sign);
        return true;
    }
    if (!readNumber(pos, &v, &end) || !isSuffix(end) || end + 1 != n) return false;
    out->suffix = t[end];
    out->hasSuffix = true;
    out->z = Complex(re, v);
    return true;
}

bool complexArg(Call &c, const Value &v, ComplexValue *out)
{
    if (v.isError()) {
        c.error(v.errorKind());
        return false;
    }
    if (v.isNumber()) {
        out->z = Complex(v.number(), 0);
        return true;
    }
    if (v.isBool()) {
        c.error(ErrorKind::Value);
        return false;
    }
    if (v.isEmpty()) {
        out->z = 0;
        return true;
    }
    if (!parseComplex(v.text(), out)) {
        c.error(ErrorKind::Num);
        return false;
    }
    return true;
}

bool complexAt(Call &c, int i, ComplexValue *out) { return complexArg(c, c.scalar(i), out); }

QString partText(double v)
{
    if (v == 0) return QStringLiteral("0");
    return numberToText(v);
}

QString complexText(const Complex &z, QChar suffix)
{
    const double re = z.real(), im = z.imag();
    QString s;
    if (im == 0) return partText(re);
    const QString imText = im == 1 ? QString() : im == -1 ? QStringLiteral("-") : partText(im);
    if (re == 0) return imText + suffix;
    s = partText(re);
    if (imText.isEmpty()) s += QLatin1Char('+');
    else if (!imText.startsWith(QLatin1Char('-'))) s += QLatin1Char('+');
    return s + imText + suffix;
}

Value complexResult(Call &c, const Complex &z, QChar suffix)
{
    if (!std::isfinite(z.real()) || !std::isfinite(z.imag())) return c.error(ErrorKind::Num);
    return Value::text(complexText(z, suffix));
}

Value fnComplex(Call &c)
{
    double re, im;
    QString suffix = QStringLiteral("i");
    if (!c.number(0, &re) || !c.number(1, &im)) return c.failure();
    if (!c.missing(2) && !c.text(2, &suffix)) return c.failure();
    if (suffix != QLatin1String("i") && suffix != QLatin1String("j")) return c.error(ErrorKind::Value);
    return complexResult(c, Complex(re, im), suffix[0]);
}

Value fnImReal(Call &c)
{
    ComplexValue z;
    if (!complexAt(c, 0, &z)) return c.failure();
    return Value::number(z.z.real());
}

Value fnImaginary(Call &c)
{
    ComplexValue z;
    if (!complexAt(c, 0, &z)) return c.failure();
    return Value::number(z.z.imag());
}

Value fnImAbs(Call &c)
{
    ComplexValue z;
    if (!complexAt(c, 0, &z)) return c.failure();
    return numberResult(std::hypot(z.z.real(), z.z.imag()));
}

Value fnImArgument(Call &c)
{
    ComplexValue z;
    if (!complexAt(c, 0, &z)) return c.failure();
    if (z.z == Complex(0, 0)) return c.error(ErrorKind::Div0);
    return numberResult(std::atan2(z.z.imag(), z.z.real()));
}

// A function of one complex number that gives one.
template <Complex (*F)(const Complex &)>
Value unaryComplex(Call &c)
{
    ComplexValue z;
    if (!complexAt(c, 0, &z)) return c.failure();
    return complexResult(c, F(z.z), z.suffix);
}

Complex conjugateF(const Complex &z) { return std::conj(z); }
Complex expF(const Complex &z) { return std::exp(z); }
Complex sqrtF(const Complex &z) { return std::sqrt(z); }
Complex sinF(const Complex &z) { return std::sin(z); }
Complex cosF(const Complex &z) { return std::cos(z); }
Complex tanF(const Complex &z) { return std::tan(z); }
Complex sinhF(const Complex &z) { return std::sinh(z); }
Complex coshF(const Complex &z) { return std::cosh(z); }
Complex cotF(const Complex &z) { return Complex(1, 0) / std::tan(z); }
Complex cscF(const Complex &z) { return Complex(1, 0) / std::sin(z); }
Complex secF(const Complex &z) { return Complex(1, 0) / std::cos(z); }
Complex cschF(const Complex &z) { return Complex(1, 0) / std::sinh(z); }
Complex sechF(const Complex &z) { return Complex(1, 0) / std::cosh(z); }

// The logarithm, to a base (0: natural); zero has none.
template <int Base>
Value logarithm(Call &c)
{
    ComplexValue z;
    if (!complexAt(c, 0, &z)) return c.failure();
    if (z.z == Complex(0, 0)) return c.error(ErrorKind::Num);
    Complex r = std::log(z.z);
    if (Base == 10) r /= std::log(10.0);
    if (Base == 2) r /= std::log(2.0);
    return complexResult(c, r, z.suffix);
}

// Every operand must use the same suffix as the others that have one.
bool commonSuffix(Call &c, const std::vector<ComplexValue> &v, QChar *suffix)
{
    *suffix = QLatin1Char('i');
    bool seen = false;
    for (const ComplexValue &z : v)
        if (z.hasSuffix) {
            if (seen && z.suffix != *suffix) {
                c.error(ErrorKind::Value);
                return false;
            }
            *suffix = z.suffix;
            seen = true;
        }
    return true;
}

Value fold(Call &c, bool product)
{
    std::vector<ComplexValue> all;
    for (int i = 0; i < c.argc(); ++i) {
        bool failed = false;
        const bool done = c.each(i, [&](const Value &v, bool) {
            ComplexValue z;
            if (v.isEmpty() || (v.isText() && v.text().isEmpty())) return true;
            if (!complexArg(c, v, &z)) {
                failed = true;
                return false;
            }
            all.push_back(z);
            return true;
        });
        if (failed) return c.failure();
        if (!done) return c.error(ErrorKind::Value);
    }
    QChar suffix;
    if (!commonSuffix(c, all, &suffix)) return c.failure();
    Complex r = product ? Complex(1, 0) : Complex(0, 0);
    if (all.empty()) r = product ? Complex(0, 0) : Complex(0, 0);
    for (const ComplexValue &z : all) r = product ? r * z.z : r + z.z;
    return complexResult(c, r, suffix);
}
Value fnImSum(Call &c) { return fold(c, false); }
Value fnImProduct(Call &c) { return fold(c, true); }

Value fnImSub(Call &c)
{
    ComplexValue a, b;
    if (!complexAt(c, 0, &a) || !complexAt(c, 1, &b)) return c.failure();
    QChar suffix;
    if (!commonSuffix(c, {a, b}, &suffix)) return c.failure();
    return complexResult(c, a.z - b.z, suffix);
}

Value fnImDiv(Call &c)
{
    ComplexValue a, b;
    if (!complexAt(c, 0, &a) || !complexAt(c, 1, &b)) return c.failure();
    QChar suffix;
    if (!commonSuffix(c, {a, b}, &suffix)) return c.failure();
    if (b.z == Complex(0, 0)) return c.error(ErrorKind::Num);
    return complexResult(c, a.z / b.z, suffix);
}

Value fnImPower(Call &c)
{
    ComplexValue z;
    double n;
    if (!complexAt(c, 0, &z) || !c.number(1, &n)) return c.failure();
    if (z.z == Complex(0, 0)) return n > 0 ? complexResult(c, Complex(0, 0), z.suffix) : c.error(ErrorKind::Num);
    const double r = std::pow(std::abs(z.z), n), angle = std::arg(z.z) * n;
    return complexResult(c, Complex(r * std::cos(angle), r * std::sin(angle)), z.suffix);
}

// ---- Bessel functions of integer order, by integrals that converge fast

// Gauss-Legendre nodes and weights on [-1, 1].
struct Rule {
    std::vector<double> x, w;
};

const Rule &legendre()
{
    static const Rule rule = [] {
        Rule r;
        const int n = 24;
        r.x.resize(n);
        r.w.resize(n);
        for (int i = 0; i < n; ++i) {
            double z = std::cos(kPi * (i + 0.75) / (n + 0.5)), pp = 1;
            for (int it = 0; it < 100; ++it) {
                double p1 = 1, p2 = 0;
                for (int j = 0; j < n; ++j) {
                    const double p3 = p2;
                    p2 = p1;
                    p1 = ((2.0 * j + 1) * z * p2 - j * p3) / (j + 1);
                }
                pp = n * (z * p1 - p2) / (z * z - 1);
                const double z1 = z;
                z = z1 - p1 / pp;
                if (std::fabs(z - z1) < 1e-15) break;
            }
            r.x[size_t(i)] = z;
            r.w[size_t(i)] = 2 / ((1 - z * z) * pp * pp);
        }
        return r;
    }();
    return rule;
}

template <typename F>
double integrate(double a, double b, int pieces, F f)
{
    const Rule &r = legendre();
    double sum = 0;
    const double h = (b - a) / pieces;
    for (int p = 0; p < pieces; ++p) {
        const double lo = a + p * h, mid = lo + h / 2;
        for (size_t i = 0; i < r.x.size(); ++i) sum += r.w[i] * f(mid + r.x[i] * h / 2) * h / 2;
    }
    return sum;
}

// The mean of a function of period 2*pi over the circle (the trapezoid rule is then as exact as the numbers allow).
template <typename F>
double circleMean(int points, F f)
{
    double sum = 0;
    for (int k = 0; k < points; ++k) sum += f(2 * kPi * k / points);
    return sum / points;
}

bool orderArg(Call &c, int i, int *n)
{
    double d;
    if (!c.number(i, &d)) return false;
    d = std::trunc(d);
    if (d < 0 || d > 1000) {
        c.error(ErrorKind::Num);
        return false;
    }
    *n = int(d);
    return true;
}

Value fnBesselJ(Call &c)
{
    double x;
    int n;
    if (!c.number(0, &x) || !orderArg(c, 1, &n)) return c.failure();
    if (!(std::fabs(x) <= 1e5)) return c.error(ErrorKind::Num);
    const int points = int(std::min(200000.0, 64 + std::fabs(x) * 2 + n * 2));
    if (!c.machine().charge(points)) return c.error(ErrorKind::Value);
    return numberResult(circleMean(points, [&](double t) { return std::cos(n * t - x * std::sin(t)); }));
}

Value fnBesselI(Call &c)
{
    double x;
    int n;
    if (!c.number(0, &x) || !orderArg(c, 1, &n)) return c.failure();
    if (std::fabs(x) > 700) return c.error(ErrorKind::Num);
    const int points = int(std::min(200000.0, 64 + std::fabs(x) * 2 + n * 2));
    if (!c.machine().charge(points)) return c.error(ErrorKind::Value);
    return numberResult(circleMean(points, [&](double t) { return std::exp(x * std::cos(t)) * std::cos(n * t); }));
}

Value fnBesselK(Call &c)
{
    double x;
    int n;
    if (!c.number(0, &x) || !orderArg(c, 1, &n)) return c.failure();
    if (x <= 0) return c.error(ErrorKind::Num);
    if (!c.machine().charge(5000)) return c.error(ErrorKind::Value);
    // e^{-x cosh t} cosh(n t): the t where it has fallen to nothing
    double top = 1;
    while (x * std::cosh(top) - n * top < 745 && top < 800) top += 0.5;
    const int pieces = int(top * 2) + 1;
    return numberResult(integrate(0, top, pieces, [&](double t) { return std::exp(-x * std::cosh(t) + n * t) * 0.5 * (1 + std::exp(-2.0 * n * t)); }));
}

Value fnBesselY(Call &c)
{
    double x;
    int n;
    if (!c.number(0, &x) || !orderArg(c, 1, &n)) return c.failure();
    if (x <= 0 || x > 1e5) return c.error(ErrorKind::Num);
    if (!c.machine().charge(10000)) return c.error(ErrorKind::Value);
    const int pieces1 = int(std::min(2000.0, 4 + std::fabs(x) / 2 + n));
    const double first = integrate(0, kPi, pieces1, [&](double t) { return std::sin(x * std::sin(t) - n * t); }) / kPi;
    double top = 1;
    while (x * std::sinh(top) - n * top < 745 && top < 800) top += 0.5;
    const double sign = n % 2 ? -1 : 1;
    const double second = integrate(0, top, int(top * 2) + 1, [&](double t) { return (std::exp(n * t - x * std::sinh(t)) + sign * std::exp(-n * t - x * std::sinh(t))); }) / kPi;
    return numberResult(first - second);
}

// ---- CONVERT

struct Unit {
    const char *name;
    const char *group;
    double factor;   // the amount of the group's base unit (and, for temperature, handled apart)
    bool prefixes;   // metric prefixes may be put before the name
};

const Unit kUnits[] = {
    // mass: grams
    {"g", "mass", 1, true}, {"sg", "mass", 14593.90293720636, false}, {"lbm", "mass", 453.59237, false}, {"u", "mass", 1.66053906660e-24, true}, {"ozm", "mass", 28.349523125, false},
    {"grain", "mass", 0.06479891, false}, {"cwt", "mass", 45359.237, false}, {"shweight", "mass", 45359.237, false}, {"uk_cwt", "mass", 50802.34544, false},
    {"lcwt", "mass", 50802.34544, false}, {"hweight", "mass", 50802.34544, false}, {"stone", "mass", 6350.29318, false}, {"ton", "mass", 907184.74, false},
    {"uk_ton", "mass", 1016046.9088, false}, {"LTON", "mass", 1016046.9088, false}, {"brton", "mass", 1016046.9088, false},
    // distance: meters
    {"m", "distance", 1, true}, {"mi", "distance", 1609.344, false}, {"Nmi", "distance", 1852, false}, {"in", "distance", 0.0254, false}, {"ft", "distance", 0.3048, false},
    {"yd", "distance", 0.9144, false}, {"ang", "distance", 1e-10, true}, {"ell", "distance", 1.143, false}, {"ly", "distance", 9460730472580800.0, false},
    {"parsec", "distance", 30856775814913673.0, false}, {"pc", "distance", 30856775814913673.0, false}, {"Pica", "distance", 0.0254 / 72, false},
    {"pica", "distance", 0.0254 / 6, false}, {"survey_mi", "distance", 1609.3472186944373, false},
    // time: seconds
    {"yr", "time", 31557600, false}, {"day", "time", 86400, false}, {"d", "time", 86400, false}, {"hr", "time", 3600, false}, {"mn", "time", 60, false}, {"min", "time", 60, false},
    {"sec", "time", 1, true}, {"s", "time", 1, true},
    // pressure: pascals
    {"Pa", "pressure", 1, true}, {"p", "pressure", 1, true}, {"atm", "pressure", 101325, true}, {"at", "pressure", 101325, true}, {"mmHg", "pressure", 133.322, true},
    {"psi", "pressure", 6894.757293168361, true}, {"Torr", "pressure", 101325.0 / 760, false},
    // force: newtons
    {"N", "force", 1, true}, {"dyn", "force", 1e-5, true}, {"dy", "force", 1e-5, true}, {"lbf", "force", 4.4482216152605, false}, {"pond", "force", 0.00980665, true},
    // energy: joules
    {"J", "energy", 1, true}, {"e", "energy", 1e-7, true}, {"c", "energy", 4.184, true}, {"cal", "energy", 4.1868, true}, {"eV", "energy", 1.602176565e-19, true},
    {"ev", "energy", 1.602176565e-19, true}, {"HPh", "energy", 2684519.537696172, false}, {"hh", "energy", 2684519.537696172, false}, {"Wh", "energy", 3600, true},
    {"wh", "energy", 3600, true}, {"flb", "energy", 1.3558179483314004, false}, {"BTU", "energy", 1055.05585262, false}, {"btu", "energy", 1055.05585262, false},
    // power: watts
    {"HP", "power", 745.6998715822702, false}, {"h", "power", 745.6998715822702, false}, {"PS", "power", 735.49875, false}, {"W", "power", 1, true}, {"w", "power", 1, true},
    // magnetism: teslas
    {"T", "magnetism", 1, true}, {"ga", "magnetism", 0.0001, true},
    // temperature: handled by name
    {"C", "temperature", 0, false}, {"cel", "temperature", 0, false}, {"F", "temperature", 0, false}, {"fah", "temperature", 0, false}, {"K", "temperature", 0, true},
    {"kel", "temperature", 0, true}, {"Rank", "temperature", 0, false}, {"Reau", "temperature", 0, false},
    // volume: cubic meters
    {"tsp", "volume", 4.92892159375e-6, false}, {"tspm", "volume", 5e-6, false}, {"tbs", "volume", 1.478676478125e-5, false}, {"oz", "volume", 2.95735295625e-5, false},
    {"cup", "volume", 2.365882365e-4, false}, {"pt", "volume", 4.73176473e-4, false}, {"us_pt", "volume", 4.73176473e-4, false}, {"uk_pt", "volume", 5.6826125e-4, false},
    {"qt", "volume", 9.46352946e-4, false}, {"uk_qt", "volume", 1.1365225e-3, false}, {"gal", "volume", 3.785411784e-3, false}, {"uk_gal", "volume", 4.54609e-3, false},
    {"l", "volume", 1e-3, true}, {"L", "volume", 1e-3, true}, {"lt", "volume", 1e-3, true}, {"m3", "volume", 1, true}, {"ang3", "volume", 1e-30, true}, 
    {"mi3", "volume", 1609.344 * 1609.344 * 1609.344, false}, {"Nmi3", "volume", 1852.0 * 1852.0 * 1852.0, false}, {"ly3", "volume", 9460730472580800.0 * 9460730472580800.0 * 9460730472580800.0, false}, {"Pica3", "volume", (0.0254 / 72) * (0.0254 / 72) * (0.0254 / 72), false}, {"barrel", "volume", 0.158987294928, false}, {"bushel", "volume", 0.03523907016688, false}, {"regton", "volume", 2.8316846592, false}, {"GRT", "volume", 2.8316846592, false},
    {"Schooner", "volume", 4.25e-4, false}, {"Middy", "volume", 2.85e-4, false}, {"Glass", "volume", 2.5e-4, false}, {"Sixpack", "volume", 2.1e-3, false}, {"Humpen", "volume", 5e-4, false},
    {"tspm", "volume", 5e-6, false}, {"in3", "volume", 1.6387064e-5, false}, {"ft3", "volume", 0.028316846592, false},
    {"yd3", "volume", 0.764554857984, false}, {"uk_pt", "volume", 5.6826125e-4, false},
    // area: square meters
    {"uk_acre", "area", 4046.8564224, false}, {"us_acre", "area", 4046.872609874252, false}, {"ang2", "area", 1e-20, true}, {"ar", "area", 100, true}, {"ft2", "area", 0.09290304, false},
    {"ha", "area", 10000, true}, {"in2", "area", 6.4516e-4, false}, {"m2", "area", 1, true}, {"Morgen", "area", 2500, false},
    {"mi2", "area", 2589988.110336, false}, {"Nmi2", "area", 3429904, false}, {"yd2", "area", 0.83612736, false}, {"ly2", "area", 9460730472580800.0 * 9460730472580800.0, false}, {"Pica2", "area", (0.0254 / 72) * (0.0254 / 72), false},
    // information: bits
    {"bit", "information", 1, true}, {"byte", "information", 8, true},
    // speed: meters per second
    {"m/h", "speed", 1.0 / 3600, true}, {"m/hr", "speed", 1.0 / 3600, true}, {"m/s", "speed", 1, true}, {"m/sec", "speed", 1, true}, {"mph", "speed", 0.44704, false},
    {"kn", "speed", 1852.0 / 3600, false}, {"admkn", "speed", 1853.184 / 3600, false},
};

struct Prefix {
    const char *name;
    double factor;
};

const Prefix kPrefixes[] = {
    {"Y", 1e24}, {"Z", 1e21}, {"E", 1e18}, {"P", 1e15}, {"T", 1e12}, {"G", 1e9}, {"M", 1e6}, {"k", 1e3}, {"h", 1e2}, {"da", 1e1}, {"e", 1e1}, {"d", 1e-1}, {"c", 1e-2},
    {"m", 1e-3}, {"u", 1e-6}, {"n", 1e-9}, {"p", 1e-12}, {"f", 1e-15}, {"a", 1e-18}, {"z", 1e-21}, {"y", 1e-24},
};
const Prefix kBinaryPrefixes[] = {
    {"Yi", 1208925819614629174706176.0}, {"Zi", 1180591620717411303424.0}, {"Ei", 1152921504606846976.0}, {"Pi", 1125899906842624.0}, {"Ti", 1099511627776.0},
    {"Gi", 1073741824.0}, {"Mi", 1048576.0}, {"ki", 1024.0},
};

// The unit a name stands for, with the prefix's factor folded in.
bool findUnit(const QString &name, const Unit **unit, double *prefix)
{
    QByteArray key = name.toUtf8();
    key.replace("^2", "2").replace("^3", "3");
    for (const Unit &u : kUnits)
        if (key == u.name) {
            *unit = &u;
            *prefix = 1;
            return true;
        }
    // a prefix and a unit that takes one (a unit with a squared or cubed name takes the prefix to that power)
    auto tryPrefixes = [&](const Prefix *list, size_t count, bool information) {
        for (size_t i = 0; i < count; ++i) {
            const QByteArray p = list[i].name;
            if (!key.startsWith(p) || key.size() <= p.size()) continue;
            const QByteArray rest = key.mid(p.size());
            for (const Unit &u : kUnits) {
                if (rest != u.name || !u.prefixes) continue;
                if (information != (QByteArray(u.group) == "information")) continue;
                const QByteArray g = u.group;
                double f = list[i].factor;
                if (g == "area" && (QByteArray(u.name) == "m2" || QByteArray(u.name) == "ang2")) f = f * f;
                else if (g == "volume" && (QByteArray(u.name) == "m3" || QByteArray(u.name) == "ang3")) f = f * f * f;
                *unit = &u;
                *prefix = f;
                return true;
            }
        }
        return false;
    };
    if (tryPrefixes(kBinaryPrefixes, sizeof(kBinaryPrefixes) / sizeof(kBinaryPrefixes[0]), true)) return true;
    return tryPrefixes(kPrefixes, sizeof(kPrefixes) / sizeof(kPrefixes[0]), false) || tryPrefixes(kPrefixes, sizeof(kPrefixes) / sizeof(kPrefixes[0]), true);
}

double toKelvin(const QString &name, double v)
{
    if (name == QLatin1String("C") || name == QLatin1String("cel")) return v + 273.15;
    if (name == QLatin1String("F") || name == QLatin1String("fah")) return (v - 32) * 5 / 9 + 273.15;
    if (name == QLatin1String("Rank")) return v * 5 / 9;
    if (name == QLatin1String("Reau")) return v * 5 / 4 + 273.15;
    return v;   // K and kel (with a prefix, scaled by the caller)
}

double fromKelvin(const QString &name, double k)
{
    if (name == QLatin1String("C") || name == QLatin1String("cel")) return k - 273.15;
    if (name == QLatin1String("F") || name == QLatin1String("fah")) return (k - 273.15) * 9 / 5 + 32;
    if (name == QLatin1String("Rank")) return k * 9 / 5;
    if (name == QLatin1String("Reau")) return (k - 273.15) * 4 / 5;
    return k;
}

Value fnConvert(Call &c)
{
    double x;
    QString from, to;
    if (!c.number(0, &x) || !c.text(1, &from) || !c.text(2, &to)) return c.failure();
    const Unit *a = nullptr, *b = nullptr;
    double pa = 1, pb = 1;
    if (!findUnit(from, &a, &pa) || !findUnit(to, &b, &pb) || QByteArray(a->group) != QByteArray(b->group)) return c.error(ErrorKind::NA);
    if (QByteArray(a->group) == "temperature") {
        const QString baseA = pa == 1 ? from : QStringLiteral("K"), baseB = pb == 1 ? to : QStringLiteral("K");
        return numberResult(fromKelvin(baseB, toKelvin(baseA, x * pa)) / pb);
    }
    return numberResult(x * pa * a->factor / (pb * b->factor));
}

} // namespace

void registerEngineeringFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"BIN2DEC", 1, 1, "v", 0, convertBase<2, 10>});
    r.push_back({"BIN2HEX", 1, 2, "vv", 0, convertBase<2, 16>});
    r.push_back({"BIN2OCT", 1, 2, "vv", 0, convertBase<2, 8>});
    r.push_back({"DEC2BIN", 1, 2, "vv", 0, convertBase<10, 2>});
    r.push_back({"DEC2HEX", 1, 2, "vv", 0, convertBase<10, 16>});
    r.push_back({"DEC2OCT", 1, 2, "vv", 0, convertBase<10, 8>});
    r.push_back({"HEX2BIN", 1, 2, "vv", 0, convertBase<16, 2>});
    r.push_back({"HEX2DEC", 1, 1, "v", 0, convertBase<16, 10>});
    r.push_back({"HEX2OCT", 1, 2, "vv", 0, convertBase<16, 8>});
    r.push_back({"OCT2BIN", 1, 2, "vv", 0, convertBase<8, 2>});
    r.push_back({"OCT2DEC", 1, 1, "v", 0, convertBase<8, 10>});
    r.push_back({"OCT2HEX", 1, 2, "vv", 0, convertBase<8, 16>});
    r.push_back({"BITAND", 2, 2, "vv", kFnFuture, bitwise<0>});
    r.push_back({"BITOR", 2, 2, "vv", kFnFuture, bitwise<1>});
    r.push_back({"BITXOR", 2, 2, "vv", kFnFuture, bitwise<2>});
    r.push_back({"BITLSHIFT", 2, 2, "vv", kFnFuture, shifted<true>});
    r.push_back({"BITRSHIFT", 2, 2, "vv", kFnFuture, shifted<false>});
    r.push_back({"DELTA", 1, 2, "vv", 0, fnDelta});
    r.push_back({"GESTEP", 1, 2, "vv", 0, fnGestep});
    r.push_back({"ERF", 1, 2, "vv", 0, fnErf});
    r.push_back({"ERF.PRECISE", 1, 1, "v", kFnFuture, fnErfPrecise});
    r.push_back({"ERFC", 1, 1, "v", 0, fnErfc});
    r.push_back({"ERFC.PRECISE", 1, 1, "v", kFnFuture, fnErfc});
    r.push_back({"COMPLEX", 2, 3, "vvv", 0, fnComplex});
    r.push_back({"IMREAL", 1, 1, "v", 0, fnImReal});
    r.push_back({"IMAGINARY", 1, 1, "v", 0, fnImaginary});
    r.push_back({"IMABS", 1, 1, "v", 0, fnImAbs});
    r.push_back({"IMARGUMENT", 1, 1, "v", 0, fnImArgument});
    r.push_back({"IMCONJUGATE", 1, 1, "v", 0, unaryComplex<conjugateF>});
    r.push_back({"IMEXP", 1, 1, "v", 0, unaryComplex<expF>});
    r.push_back({"IMSQRT", 1, 1, "v", 0, unaryComplex<sqrtF>});
    r.push_back({"IMSIN", 1, 1, "v", 0, unaryComplex<sinF>});
    r.push_back({"IMCOS", 1, 1, "v", 0, unaryComplex<cosF>});
    r.push_back({"IMTAN", 1, 1, "v", kFnFuture, unaryComplex<tanF>});
    r.push_back({"IMCOT", 1, 1, "v", kFnFuture, unaryComplex<cotF>});
    r.push_back({"IMCSC", 1, 1, "v", kFnFuture, unaryComplex<cscF>});
    r.push_back({"IMSEC", 1, 1, "v", kFnFuture, unaryComplex<secF>});
    r.push_back({"IMSINH", 1, 1, "v", kFnFuture, unaryComplex<sinhF>});
    r.push_back({"IMCOSH", 1, 1, "v", kFnFuture, unaryComplex<coshF>});
    r.push_back({"IMCSCH", 1, 1, "v", kFnFuture, unaryComplex<cschF>});
    r.push_back({"IMSECH", 1, 1, "v", kFnFuture, unaryComplex<sechF>});
    r.push_back({"IMLN", 1, 1, "v", 0, logarithm<0>});
    r.push_back({"IMLOG10", 1, 1, "v", 0, logarithm<10>});
    r.push_back({"IMLOG2", 1, 1, "v", 0, logarithm<2>});
    r.push_back({"IMSUM", 1, 255, "r", 0, fnImSum});
    r.push_back({"IMPRODUCT", 1, 255, "r", 0, fnImProduct});
    r.push_back({"IMSUB", 2, 2, "vv", 0, fnImSub});
    r.push_back({"IMDIV", 2, 2, "vv", 0, fnImDiv});
    r.push_back({"IMPOWER", 2, 2, "vv", 0, fnImPower});
    r.push_back({"BESSELJ", 2, 2, "vv", 0, fnBesselJ});
    r.push_back({"BESSELI", 2, 2, "vv", 0, fnBesselI});
    r.push_back({"BESSELK", 2, 2, "vv", 0, fnBesselK});
    r.push_back({"BESSELY", 2, 2, "vv", 0, fnBesselY});
    r.push_back({"CONVERT", 3, 3, "vvv", 0, fnConvert});
}

} // namespace jp::calc
