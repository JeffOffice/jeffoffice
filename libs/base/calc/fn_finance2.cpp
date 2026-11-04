// More financial functions: bonds and bills (prices, yields, coupon dates, and durations), loans over a stretch of payments, depreciation of the French kind and the
// fixed declining balance, and the rate of a loan.
#include "calc/machine.h"

#include <algorithm>
#include <cmath>
#include <functional>

namespace jp::calc {

namespace {

// ---- dates

struct Ymd {
    int y = 0, m = 0, d = 0;
};

bool toYmd(double serial, DateSystem sys, Ymd *r) { return serialToDate(serial, sys, &r->y, &r->m, &r->d); }

bool isLeapYear(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int daysInMonth(int y, int m)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return m == 2 && isLeapYear(y) ? 29 : days[m - 1];
}

// The date `months` months from the day number `serial`, the last day of the month if `keepEnd` is set and the date is one (as the coupon dates of a bond that matures on
// the last day of a month fall on last days).
bool addMonths(double serial, int months, bool keepEnd, DateSystem sys, double *out)
{
    Ymd t;
    if (!toYmd(serial, sys, &t)) return false;
    const int index = t.y * 12 + (t.m - 1) + months;
    if (index < 0) return false;
    const int y = index / 12, m = index % 12 + 1;
    int d = t.d;
    if (keepEnd && t.d == daysInMonth(t.y, t.m)) d = daysInMonth(y, m);
    d = std::min(d, daysInMonth(y, m));
    return dateToSerial(y, m, d, sys, out);
}

bool isMonthEnd(double serial, DateSystem sys)
{
    Ymd t;
    return toYmd(serial, sys, &t) && t.d == daysInMonth(t.y, t.m);
}

// Reads a date argument (cut to a whole day).
bool dateArg(Call &c, int i, double *out)
{
    if (!c.number(i, out)) return false;
    *out = std::floor(*out);
    if (*out < 0) {
        c.error(ErrorKind::Num);
        return false;
    }
    return true;
}

bool basisArg(Call &c, int i, int *basis)
{
    double b = 0;
    if (!c.numberOr(i, 0, &b)) return false;
    b = std::trunc(b);
    if (b < 0 || b > 4) {
        c.error(ErrorKind::Num);
        return false;
    }
    *basis = int(b);
    return true;
}

bool frequencyArg(Call &c, int i, int *f)
{
    double v;
    if (!c.number(i, &v)) return false;
    v = std::trunc(v);
    if (v != 1 && v != 2 && v != 4) {
        c.error(ErrorKind::Num);
        return false;
    }
    *f = int(v);
    return true;
}

// Days from a to b counted as the basis counts them.
double daysBy(Call &c, double a, double b, int basis)
{
    const DateSystem sys = c.context().dateSystem;
    if (basis == 0 || basis == 4) {
        double d;
        if (daysBetween360(a, b, basis == 4, sys, &d)) return d;
    }
    return b - a;
}

// The span as a fraction of a year.
bool fraction(Call &c, double a, double b, int basis, double *out) { return yearFractionOf(a, b, basis, c.context().dateSystem, out); }

// ---- coupons

struct Coupons {
    double previous = 0, next = 0;   // the dates around the settlement
    int count = 0;                   // the coupons still to be paid
    double bs = 0, e = 0, snc = 0;   // days since the last coupon, in the period, and to the next
};

// The coupon schedule of a bond: dates stepped back from the maturity.
bool couponsOf(Call &c, double settle, double maturity, int frequency, int basis, Coupons *r)
{
    const DateSystem sys = c.context().dateSystem;
    if (settle >= maturity) {
        c.error(ErrorKind::Num);
        return false;
    }
    const int step = 12 / frequency;
    const bool end = isMonthEnd(maturity, sys);
    int k = 0;
    double date = maturity, before = maturity;
    while (date > settle) {
        before = date;
        ++k;
        if (k > 20000 || !addMonths(maturity, -k * step, end, sys, &date)) {
            c.error(ErrorKind::Num);
            return false;
        }
    }
    r->previous = date;
    r->next = before;
    r->count = k;
    r->bs = daysBy(c, r->previous, settle, basis);
    if (basis == 1) r->e = r->next - r->previous;
    else if (basis == 3) r->e = 365.0 / frequency;
    else r->e = 360.0 / frequency;
    r->snc = (basis == 0 || basis == 4) ? r->e - r->bs : r->next - settle;
    return true;
}

Value fnCoupDayBs(Call &c)
{
    double s, m;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !frequencyArg(c, 2, &f) || !basisArg(c, 3, &b)) return c.failure();
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    return Value::number(k.bs);
}

Value fnCoupDays(Call &c)
{
    double s, m;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !frequencyArg(c, 2, &f) || !basisArg(c, 3, &b)) return c.failure();
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    return Value::number(k.e);
}

Value fnCoupDaysNc(Call &c)
{
    double s, m;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !frequencyArg(c, 2, &f) || !basisArg(c, 3, &b)) return c.failure();
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    return Value::number(k.snc);
}

Value fnCoupNcd(Call &c)
{
    double s, m;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !frequencyArg(c, 2, &f) || !basisArg(c, 3, &b)) return c.failure();
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    return Value::number(k.next);
}

Value fnCoupPcd(Call &c)
{
    double s, m;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !frequencyArg(c, 2, &f) || !basisArg(c, 3, &b)) return c.failure();
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    return Value::number(k.previous);
}

Value fnCoupNum(Call &c)
{
    double s, m;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !frequencyArg(c, 2, &f) || !basisArg(c, 3, &b)) return c.failure();
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    return Value::number(k.count);
}

// ---- the price of a bond by its yield, and the yield by its price

// The price per 100 of a bond (the settlement inside a coupon period, any number of coupons left).
double bondPrice(const Coupons &k, double rate, double yield, double redemption, int f)
{
    const double coupon = 100 * rate / f, y = yield / f;
    const double dscE = k.snc / k.e, aE = k.bs / k.e;
    if (k.count <= 1) return (redemption + coupon) / (1 + dscE * y) - coupon * aE;
    double price = redemption / std::pow(1 + y, k.count - 1 + dscE);
    for (int i = 1; i <= k.count; ++i) price += coupon / std::pow(1 + y, i - 1 + dscE);
    return price - coupon * aE;
}

// The yield that gives a price (the price falls as the yield rises).
bool bondYield(const Coupons &k, double rate, double price, double redemption, int f, double *out)
{
    if (k.count <= 1) {
        const double coupon = 100 * rate / f;
        const double dscE = k.snc / k.e, aE = k.bs / k.e;
        const double top = redemption + coupon, bottom = price + coupon * aE;
        if (bottom <= 0 || dscE == 0) return false;
        *out = (top / bottom - 1) * f / dscE;
        return true;
    }
    double lo = -0.99 * f, hi = 1.0;
    auto g = [&](double y) { return bondPrice(k, rate, y, redemption, f) - price; };
    for (int i = 0; i < 200 && g(hi) > 0; ++i) hi *= 2;
    if (g(lo) < 0 || g(hi) > 0) return false;
    for (int i = 0; i < 300; ++i) {
        const double mid = (lo + hi) / 2;
        if (g(mid) > 0) lo = mid;
        else hi = mid;
        if (hi - lo < 1e-15) break;
    }
    *out = (lo + hi) / 2;
    return true;
}

Value fnPrice(Call &c)
{
    double s, m, rate, yld, redemption;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !c.number(2, &rate) || !c.number(3, &yld) || !c.number(4, &redemption) || !frequencyArg(c, 5, &f) || !basisArg(c, 6, &b)) return c.failure();
    if (rate < 0 || yld < 0 || redemption <= 0) return c.error(ErrorKind::Num);
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    return numberResult(bondPrice(k, rate, yld, redemption, f));
}

Value fnYield(Call &c)
{
    double s, m, rate, pr, redemption, y;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !c.number(2, &rate) || !c.number(3, &pr) || !c.number(4, &redemption) || !frequencyArg(c, 5, &f) || !basisArg(c, 6, &b)) return c.failure();
    if (rate < 0 || pr <= 0 || redemption <= 0) return c.error(ErrorKind::Num);
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    if (!bondYield(k, rate, pr, redemption, f, &y)) return c.error(ErrorKind::Num);
    return numberResult(y);
}

Value durationOf(Call &c, bool modified)
{
    double s, m, rate, yld;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !c.number(2, &rate) || !c.number(3, &yld) || !frequencyArg(c, 4, &f) || !basisArg(c, 5, &b)) return c.failure();
    if (rate < 0 || yld < 0) return c.error(ErrorKind::Num);
    Coupons k;
    if (!couponsOf(c, s, m, f, b, &k)) return c.failure();
    const double coupon = 100 * rate / f, y = yld / f, dscE = k.snc / k.e;
    double weighted = 0, total = 0;
    for (int i = 1; i <= k.count; ++i) {
        const double t = i - 1 + dscE, flow = coupon + (i == k.count ? 100 : 0);
        const double pv = flow / std::pow(1 + y, t);
        weighted += t * pv;
        total += pv;
    }
    if (total == 0) return c.error(ErrorKind::Div0);
    const double duration = weighted / total / f;
    return numberResult(modified ? duration / (1 + y) : duration);
}
Value fnDuration(Call &c) { return durationOf(c, false); }
Value fnMDuration(Call &c) { return durationOf(c, true); }

// ---- securities that pay at maturity, and bills

Value fnPriceDisc(Call &c)
{
    double s, m, discount, redemption, yf;
    int b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !c.number(2, &discount) || !c.number(3, &redemption) || !basisArg(c, 4, &b)) return c.failure();
    if (discount <= 0 || redemption <= 0 || s >= m || !fraction(c, s, m, b, &yf)) return c.error(ErrorKind::Num);
    return numberResult(redemption - discount * redemption * yf);
}

Value fnPriceMat(Call &c)
{
    double s, m, issue, rate, yld, ymi, yms, yis;
    int b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !dateArg(c, 2, &issue) || !c.number(3, &rate) || !c.number(4, &yld) || !basisArg(c, 5, &b)) return c.failure();
    if (rate < 0 || yld < 0 || s >= m || issue > m || !fraction(c, issue, m, b, &ymi) || !fraction(c, s, m, b, &yms) || !fraction(c, std::min(issue, s), std::max(issue, s), b, &yis))
        return c.error(ErrorKind::Num);
    const double accrued = (s >= issue ? 1 : -1) * yis * rate * 100;
    return numberResult((100 + ymi * rate * 100) / (1 + yms * yld) - accrued);
}

Value fnYieldDisc(Call &c)
{
    double s, m, pr, redemption, yf;
    int b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !c.number(2, &pr) || !c.number(3, &redemption) || !basisArg(c, 4, &b)) return c.failure();
    if (pr <= 0 || redemption <= 0 || s >= m || !fraction(c, s, m, b, &yf)) return c.error(ErrorKind::Num);
    return numberResult((redemption - pr) / pr / yf);
}

Value fnYieldMat(Call &c)
{
    double s, m, issue, rate, pr, ymi, yms, yis;
    int b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !dateArg(c, 2, &issue) || !c.number(3, &rate) || !c.number(4, &pr) || !basisArg(c, 5, &b)) return c.failure();
    if (rate < 0 || pr <= 0 || s >= m || issue > m || !fraction(c, issue, m, b, &ymi) || !fraction(c, s, m, b, &yms) || !fraction(c, std::min(issue, s), std::max(issue, s), b, &yis))
        return c.error(ErrorKind::Num);
    const double accrued = (s >= issue ? 1 : -1) * yis * rate;
    const double base = pr / 100 + accrued;
    return numberResult(((1 + ymi * rate) - base) / base / yms);
}

Value fnDisc(Call &c)
{
    double s, m, pr, redemption, yf;
    int b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !c.number(2, &pr) || !c.number(3, &redemption) || !basisArg(c, 4, &b)) return c.failure();
    if (pr <= 0 || redemption <= 0 || s >= m || !fraction(c, s, m, b, &yf)) return c.error(ErrorKind::Num);
    return numberResult((redemption - pr) / redemption / yf);
}

Value fnIntRate(Call &c)
{
    double s, m, investment, redemption, yf;
    int b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !c.number(2, &investment) || !c.number(3, &redemption) || !basisArg(c, 4, &b)) return c.failure();
    if (investment <= 0 || redemption <= 0 || s >= m || !fraction(c, s, m, b, &yf)) return c.error(ErrorKind::Num);
    return numberResult((redemption - investment) / investment / yf);
}

Value fnReceived(Call &c)
{
    double s, m, investment, discount, yf;
    int b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !c.number(2, &investment) || !c.number(3, &discount) || !basisArg(c, 4, &b)) return c.failure();
    if (investment <= 0 || discount <= 0 || s >= m || !fraction(c, s, m, b, &yf)) return c.error(ErrorKind::Num);
    const double divisor = 1 - discount * yf;
    if (divisor <= 0) return c.error(ErrorKind::Num);
    return numberResult(investment / divisor);
}

bool billDates(Call &c, double *s, double *m, double *days)
{
    if (!dateArg(c, 0, s) || !dateArg(c, 1, m)) return false;
    *days = *m - *s;
    if (*days <= 0 || *days > 365) {
        c.error(ErrorKind::Num);
        return false;
    }
    return true;
}

Value fnTBillPrice(Call &c)
{
    double s, m, days, discount;
    if (!billDates(c, &s, &m, &days) || !c.number(2, &discount)) return c.failure();
    if (discount <= 0) return c.error(ErrorKind::Num);
    const double p = 100 * (1 - discount * days / 360);
    if (p <= 0) return c.error(ErrorKind::Num);
    return numberResult(p);
}

Value fnTBillYield(Call &c)
{
    double s, m, days, pr;
    if (!billDates(c, &s, &m, &days) || !c.number(2, &pr)) return c.failure();
    if (pr <= 0) return c.error(ErrorKind::Num);
    return numberResult((100 - pr) / pr * 360 / days);
}

Value fnTBillEq(Call &c)
{
    double s, m, days, discount;
    if (!billDates(c, &s, &m, &days) || !c.number(2, &discount)) return c.failure();
    if (discount <= 0 || discount * days / 360 >= 1) return c.error(ErrorKind::Num);
    if (days <= 182) return numberResult(365 * discount / (360 - discount * days));
    const DateSystem sys = c.context().dateSystem;
    Ymd a, z;
    toYmd(s, sys, &a);
    toYmd(m, sys, &z);
    bool leap = false;
    for (int y = a.y; y <= z.y; ++y) {
        double feb;
        if (isLeapYear(y) && dateToSerial(y, 2, 29, sys, &feb) && feb >= s && feb <= m) leap = true;
    }
    const double year = leap ? 366.0 : 365.0;
    const double price = 100 * (1 - discount * days / 360);
    const double qa = days / (2 * year) - 0.25, qb = days / year, qc = (price - 100) / price;
    const double root = qb * qb - 4 * qa * qc;
    if (root < 0 || qa == 0) return c.error(ErrorKind::Num);
    return numberResult((-qb + std::sqrt(root)) / (2 * qa));
}

Value fnDollarDe(Call &c)
{
    double dollar, fraction;
    if (!c.number(0, &dollar) || !c.number(1, &fraction)) return c.failure();
    fraction = std::trunc(fraction);
    if (fraction < 0) return c.error(ErrorKind::Num);
    if (fraction == 0) return c.error(ErrorKind::Div0);
    const double whole = std::trunc(dollar), rest = dollar - whole;
    return numberResult(whole + rest * std::pow(10.0, std::ceil(std::log10(fraction))) / fraction);
}

Value fnDollarFr(Call &c)
{
    double dollar, fraction;
    if (!c.number(0, &dollar) || !c.number(1, &fraction)) return c.failure();
    fraction = std::trunc(fraction);
    if (fraction < 0) return c.error(ErrorKind::Num);
    if (fraction == 0) return c.error(ErrorKind::Div0);
    const double whole = std::trunc(dollar), rest = dollar - whole;
    return numberResult(whole + rest * fraction / std::pow(10.0, std::ceil(std::log10(fraction))));
}

// ---- interest accrued

Value fnAccrIntM(Call &c)
{
    double issue, s, rate, par = 1000, yf;
    int b;
    if (!dateArg(c, 0, &issue) || !dateArg(c, 1, &s) || !c.number(2, &rate) || !c.numberOr(3, 1000, &par) || !basisArg(c, 4, &b)) return c.failure();
    if (rate <= 0 || par <= 0 || issue >= s || !fraction(c, issue, s, b, &yf)) return c.error(ErrorKind::Num);
    return numberResult(par * rate * yf);
}

// The ends of the regular periods (stepped from `anchor` by whole periods) around a span, as a list.
Value fnAccrInt(Call &c)
{
    double issue, first, s, rate, par;
    int f, b;
    bool method = true;
    if (!dateArg(c, 0, &issue) || !dateArg(c, 1, &first) || !dateArg(c, 2, &s) || !c.number(3, &rate) || !c.number(4, &par) || !frequencyArg(c, 5, &f) || !basisArg(c, 6, &b)
        || !c.booleanOr(7, true, &method))
        return c.failure();
    if (rate <= 0 || par <= 0 || issue >= s) return c.error(ErrorKind::Num);
    const DateSystem sys = c.context().dateSystem;
    const int step = 12 / f;
    // the start of the accrual: the issue date, or (when the method says so, and the settlement is past the first interest date) the first interest date
    double begin = issue;
    if (!method && s > first) begin = first;
    if (b == 0 || b == 2 || b == 4) {
        const double normal = 360.0 / f;
        double days = daysBy(c, begin, s, b);
        return numberResult(par * rate / f * days / normal);
    }
    if (b == 3) return numberResult(par * rate / f * (s - begin) / (365.0 / f));
    // actual/actual: the share of each regular period (stepped back from the first interest date) that the span covers
    const bool end = isMonthEnd(first, sys);
    std::vector<double> dates;
    double d = first;
    int k = 0;
    dates.push_back(d);
    while (d > begin) {
        ++k;
        if (k > 20000 || !addMonths(first, -k * step, end, sys, &d)) return c.error(ErrorKind::Num);
        dates.push_back(d);
    }
    double sum = 0;
    for (size_t i = dates.size() - 1; i > 0; --i) {
        const double lo = dates[i], hi = dates[i - 1];
        const double from = std::max(lo, begin), to = std::min(hi, s);
        if (to > from) sum += (to - from) / (hi - lo);
    }
    // (past the first interest date the regular periods go on forward from it)
    double previous = first;
    for (k = 1; previous < s && k < 20000; ++k) {
        double next;
        if (!addMonths(first, k * step, end, sys, &next)) return c.error(ErrorKind::Num);
        const double from = std::max(previous, begin), to = std::min(next, s);
        if (to > from) sum += (to - from) / (next - previous);
        previous = next;
    }
    return numberResult(par * rate / f * sum);
}

// ---- odd first and last periods

// The regular periods that a span from `start` to `finish` (a first or last odd period) is made of: the dates of their ends, stepped from `anchor` (the coupon date at the
// regular end of the odd period) away from the odd period's inside.
struct Quasi {
    std::vector<double> edges;   // ascending: edges[i] to edges[i+1] is a regular period
};

bool quasiBack(Call &c, double anchor, double reach, int f, Quasi *q)
{
    const DateSystem sys = c.context().dateSystem;
    const bool end = isMonthEnd(anchor, sys);
    std::vector<double> dates{anchor};
    double d = anchor;
    for (int k = 1; d > reach; ++k) {
        if (k > 20000 || !addMonths(anchor, -k * (12 / f), end, sys, &d)) return false;
        dates.push_back(d);
    }
    std::reverse(dates.begin(), dates.end());
    q->edges = dates;
    return true;
}

bool quasiForward(Call &c, double anchor, double reach, int f, Quasi *q)
{
    const DateSystem sys = c.context().dateSystem;
    const bool end = isMonthEnd(anchor, sys);
    std::vector<double> dates{anchor};
    double d = anchor;
    for (int k = 1; d < reach; ++k) {
        if (k > 20000 || !addMonths(anchor, k * (12 / f), end, sys, &d)) return false;
        dates.push_back(d);
    }
    q->edges = dates;
    return true;
}

double normalLength(const Quasi &q, size_t i, int f, int basis)
{
    if (basis == 1) return q.edges[i + 1] - q.edges[i];
    return basis == 3 ? 365.0 / f : 360.0 / f;
}

// The price of a bond with an odd first period, per 100.
bool oddFirstPrice(Call &c, double s, double m, double issue, double first, double rate, double yld, double redemption, int f, int b, double *out)
{
    if (!(m > first && first > s && s > issue)) {
        c.error(ErrorKind::Num);
        return false;
    }
    Quasi q;
    if (!quasiBack(c, first, issue, f, &q)) {
        c.error(ErrorKind::Num);
        return false;
    }
    const double coupon = 100 * rate / f, y = yld / f;
    double sumDc = 0, sumA = 0, e = 0;
    for (size_t i = 0; i + 1 < q.edges.size(); ++i) {
        const double nl = normalLength(q, i, f, b);
        const double from = std::max(q.edges[i], issue), to = q.edges[i + 1];
        sumDc += daysBy(c, from, to, b) / nl;
        const double upTo = std::min(s, to);
        if (upTo > from) sumA += daysBy(c, from, upTo, b) / nl;
        if (s >= q.edges[i] && s < q.edges[i + 1]) e = nl;
    }
    if (e == 0) e = normalLength(q, 0, f, b);
    const double dsc = daysBy(c, s, first, b);
    // the coupons from the first one to the maturity
    Coupons k;
    if (!couponsOf(c, first, m, f, b, &k)) return false;
    const int n = k.count + 1;
    const double t0 = dsc / e;
    double price = redemption / std::pow(1 + y, n - 1 + t0) + coupon * sumDc / std::pow(1 + y, t0);
    for (int i = 2; i <= n; ++i) price += coupon / std::pow(1 + y, i - 1 + t0);
    *out = price - coupon * sumA;
    return true;
}

bool oddLastPrice(Call &c, double s, double m, double last, double rate, double yld, double redemption, int f, int b, double *out)
{
    if (!(m > s && s > last)) {
        c.error(ErrorKind::Num);
        return false;
    }
    Quasi q;
    if (!quasiForward(c, last, m, f, &q)) {
        c.error(ErrorKind::Num);
        return false;
    }
    const double coupon = 100 * rate / f, y = yld / f;
    double sumDc = 0, sumA = 0, sumDsc = 0;
    for (size_t i = 0; i + 1 < q.edges.size(); ++i) {
        const double nl = normalLength(q, i, f, b);
        const double from = q.edges[i], to = std::min(q.edges[i + 1], m);
        if (to > from) sumDc += daysBy(c, from, to, b) / nl;
        const double sa = std::min(s, to);
        if (sa > from) sumA += daysBy(c, from, sa, b) / nl;
        const double ds = std::max(s, from);
        if (to > ds) sumDsc += daysBy(c, ds, to, b) / nl;
    }
    *out = (redemption + coupon * sumDc) / (1 + sumDsc * y) - coupon * sumA;
    return true;
}

// Solves for the yield of a price function that falls as the yield rises.
bool solveYield(const std::function<bool(double, double *)> &price, double target, double *out)
{
    double lo = -0.9, hi = 1.0, p;
    auto g = [&](double y, double *v) { return price(y, v); };
    if (!g(lo, &p) || p < target) return false;
    for (int i = 0; i < 100; ++i) {
        if (!g(hi, &p)) return false;
        if (p <= target) break;
        hi *= 2;
    }
    if (p > target) return false;
    for (int i = 0; i < 300; ++i) {
        const double mid = (lo + hi) / 2;
        if (!g(mid, &p)) return false;
        if (p > target) lo = mid;
        else hi = mid;
        if (hi - lo < 1e-15) break;
    }
    *out = (lo + hi) / 2;
    return true;
}

Value fnOddFPrice(Call &c)
{
    double s, m, issue, first, rate, yld, redemption, p;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !dateArg(c, 2, &issue) || !dateArg(c, 3, &first) || !c.number(4, &rate) || !c.number(5, &yld) || !c.number(6, &redemption)
        || !frequencyArg(c, 7, &f) || !basisArg(c, 8, &b))
        return c.failure();
    if (rate < 0 || yld < 0 || redemption <= 0) return c.error(ErrorKind::Num);
    if (!oddFirstPrice(c, s, m, issue, first, rate, yld, redemption, f, b, &p)) return c.failure();
    return numberResult(p);
}

Value fnOddFYield(Call &c)
{
    double s, m, issue, first, rate, pr, redemption, y;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !dateArg(c, 2, &issue) || !dateArg(c, 3, &first) || !c.number(4, &rate) || !c.number(5, &pr) || !c.number(6, &redemption)
        || !frequencyArg(c, 7, &f) || !basisArg(c, 8, &b))
        return c.failure();
    if (rate < 0 || pr <= 0 || redemption <= 0) return c.error(ErrorKind::Num);
    double probe;
    if (!oddFirstPrice(c, s, m, issue, first, rate, 0, redemption, f, b, &probe)) return c.failure();
    if (!solveYield([&](double yld, double *v) { return oddFirstPrice(c, s, m, issue, first, rate, yld, redemption, f, b, v); }, pr, &y)) return c.error(ErrorKind::Num);
    return numberResult(y);
}

Value fnOddLPrice(Call &c)
{
    double s, m, last, rate, yld, redemption, p;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !dateArg(c, 2, &last) || !c.number(3, &rate) || !c.number(4, &yld) || !c.number(5, &redemption) || !frequencyArg(c, 6, &f)
        || !basisArg(c, 7, &b))
        return c.failure();
    if (rate < 0 || yld < 0 || redemption <= 0) return c.error(ErrorKind::Num);
    if (!oddLastPrice(c, s, m, last, rate, yld, redemption, f, b, &p)) return c.failure();
    return numberResult(p);
}

Value fnOddLYield(Call &c)
{
    double s, m, last, rate, pr, redemption, y;
    int f, b;
    if (!dateArg(c, 0, &s) || !dateArg(c, 1, &m) || !dateArg(c, 2, &last) || !c.number(3, &rate) || !c.number(4, &pr) || !c.number(5, &redemption) || !frequencyArg(c, 6, &f)
        || !basisArg(c, 7, &b))
        return c.failure();
    if (rate < 0 || pr <= 0 || redemption <= 0) return c.error(ErrorKind::Num);
    double probe;
    if (!oddLastPrice(c, s, m, last, rate, 0, redemption, f, b, &probe)) return c.failure();
    if (!solveYield([&](double yld, double *v) { return oddLastPrice(c, s, m, last, rate, yld, redemption, f, b, v); }, pr, &y)) return c.error(ErrorKind::Num);
    return numberResult(y);
}

// ---- loans: the interest or principal over a stretch of payments, the rate, and the rate of an investment

using real = long double;

real growthOf(real rate, real n) { return std::pow(1 + rate, n); }

real futureOf(real rate, real n, real pmt, real pv, real type)
{
    if (rate == 0) return -(pv + pmt * n);
    const real g = growthOf(rate, n);
    return -(pv * g + pmt * (1 + rate * type) * (g - 1) / rate);
}

real paymentOf(real rate, real n, real pv, real fv, real type)
{
    if (rate == 0) return -(pv + fv) / n;
    const real g = growthOf(rate, n);
    return -(fv + pv * g) * rate / ((1 + rate * type) * (g - 1));
}

// The interest of payment `per` of a loan.
real interestOf(real rate, real per, real n, real pv, real pmt, real type)
{
    real owed;
    if (per == 1) owed = type > 0 ? 0 : -pv;
    else if (type > 0) owed = futureOf(rate, per - 2, pmt, pv, 1) - pmt;
    else owed = futureOf(rate, per - 1, pmt, pv, 0);
    (void)n;
    return owed * rate;
}

Value cumulative(Call &c, bool principal)
{
    double rate, n, pv, start, end, type;
    if (!c.number(0, &rate) || !c.number(1, &n) || !c.number(2, &pv) || !c.number(3, &start) || !c.number(4, &end) || !c.number(5, &type)) return c.failure();
    start = std::trunc(start);
    end = std::trunc(end);
    if (rate <= 0 || n <= 0 || pv <= 0 || start < 1 || end < start || end > n || (type != 0 && type != 1)) return c.error(ErrorKind::Num);
    if (!c.machine().charge(qint64(end - start) + 1)) return c.error(ErrorKind::Value);
    const real pmt = paymentOf(rate, n, pv, 0, type);
    real sum = 0;
    for (double per = start; per <= end; ++per) {
        const real interest = interestOf(rate, per, n, pv, pmt, type);
        sum += principal ? pmt - interest : interest;
    }
    return numberResult(double(sum));
}
Value fnCumIpmt(Call &c) { return cumulative(c, false); }
Value fnCumPrinc(Call &c) { return cumulative(c, true); }

Value fnIspmt(Call &c)
{
    double rate, per, n, pv;
    if (!c.number(0, &rate) || !c.number(1, &per) || !c.number(2, &n) || !c.number(3, &pv)) return c.failure();
    if (n == 0) return c.error(ErrorKind::Div0);
    return numberResult(pv * rate * (per / n - 1));
}

Value fnFvSchedule(Call &c)
{
    double principal;
    Value schedule;
    if (!c.number(0, &principal) || !c.array(1, &schedule)) return c.failure();
    double p = principal;
    for (int i = 0; i < schedule.rows(); ++i)
        for (int j = 0; j < schedule.cols(); ++j) {
            const Value &v = schedule.at(i, j);
            if (v.isError()) return v;
            if (v.isEmpty()) continue;
            if (!v.isNumber()) return c.error(ErrorKind::Value);
            p *= 1 + v.number();
        }
    return numberResult(p);
}

Value fnMirr(Call &c)
{
    std::vector<double> flows;
    Value error;
    double finance, reinvest;
    bool failed = false;
    const bool done = c.each(0, [&](const Value &v, bool) {
        if (v.isNumber()) flows.push_back(v.number());
        else if (v.isError()) {
            error = v;
            failed = true;
            return false;
        }
        return true;
    });
    if (failed) return error;
    if (!done) return c.error(ErrorKind::Value);
    if (!c.number(1, &finance) || !c.number(2, &reinvest)) return c.failure();
    const size_t n = flows.size();
    double positive = 0, negative = 0;
    bool any[2] = {false, false};
    for (size_t i = 0; i < n; ++i) {
        if (flows[i] > 0) {
            positive += flows[i] * std::pow(1 + reinvest, double(n - 1 - i));
            any[0] = true;
        } else if (flows[i] < 0) {
            negative += flows[i] / std::pow(1 + finance, double(i));
            any[1] = true;
        }
    }
    if (!any[0] || !any[1] || n < 2) return c.error(ErrorKind::Div0);
    return numberResult(std::pow(-positive / negative, 1.0 / double(n - 1)) - 1);
}

Value fnDb(Call &c)
{
    double cost, salvage, life, period, month = 12;
    if (!c.number(0, &cost) || !c.number(1, &salvage) || !c.number(2, &life) || !c.number(3, &period) || !c.numberOr(4, 12, &month)) return c.failure();
    month = std::trunc(month);
    period = std::trunc(period);
    life = std::trunc(life);
    if (cost < 0 || salvage < 0 || life <= 0 || period <= 0 || month < 1 || month > 12 || period > life + (month < 12 ? 1 : 0) || (month == 12 && period > life)) return c.error(ErrorKind::Num);
    if (cost == 0) return Value::number(0);
    if (!c.machine().charge(qint64(period))) return c.error(ErrorKind::Value);
    double rate = 1 - std::pow(salvage / cost, 1 / life);
    rate = std::round(rate * 1000) / 1000;
    double total = 0, depreciation = 0;
    for (double p = 1; p <= period; ++p) {
        if (p == 1) depreciation = cost * rate * month / 12;
        else if (p == life + 1) depreciation = (cost - total) * rate * (12 - month) / 12;
        else depreciation = (cost - total) * rate;
        if (p < period) total += depreciation;
    }
    return numberResult(depreciation);
}

// ---- French depreciation

Value amortization(Call &c, bool declining)
{
    double cost, purchased, first, salvage, period, rate;
    int basis;
    if (!c.number(0, &cost) || !dateArg(c, 1, &purchased) || !dateArg(c, 2, &first) || !c.number(3, &salvage) || !c.number(4, &period) || !c.number(5, &rate) || !basisArg(c, 6, &basis))
        return c.failure();
    period = std::trunc(period);
    if (cost <= 0 || salvage < 0 || salvage > cost || period < 0 || rate <= 0 || purchased > first) return c.error(ErrorKind::Num);
    if (basis == 2) return c.error(ErrorKind::Num);
    double yf;
    if (!fraction(c, purchased, first, basis, &yf)) return c.error(ErrorKind::Num);
    if (!declining) {
        const double one = cost * rate, firstAmount = yf * rate * cost;
        const double fullPeriods = std::trunc((cost - salvage - firstAmount) / one);
        if (period == 0) return numberResult(firstAmount);
        if (period <= fullPeriods) return numberResult(one);
        if (period == fullPeriods + 1) return numberResult(cost - salvage - one * fullPeriods - firstAmount);
        return Value::number(0);
    }
    if (!c.machine().charge(qint64(period) + 1)) return c.error(ErrorKind::Value);
    const double life = 1 / rate;
    const double coefficient = life < 3 ? 1 : life < 5 ? 1.5 : life <= 6 ? 2 : 2.5;
    rate *= coefficient;
    double amount = std::round(yf * rate * cost);
    cost -= amount;
    double rest = cost - salvage;
    for (double n = 0; n < period; ++n) {
        amount = std::round(rate * cost);
        rest -= amount;
        if (rest < 0) {
            if (period - n <= 1) return numberResult(std::round(cost * 0.5));
            return Value::number(0);
        }
        cost -= amount;
    }
    return numberResult(amount);
}
Value fnAmorLinc(Call &c) { return amortization(c, false); }
Value fnAmorDegrc(Call &c) { return amortization(c, true); }

// ---- the rate of a loan, and two rates that follow from a growth

Value fnRate(Call &c)
{
    double n, pmt, pv, fv = 0, type = 0, guess = 0.1;
    if (!c.number(0, &n) || !c.number(1, &pmt) || !c.number(2, &pv) || !c.numberOr(3, 0, &fv) || !c.numberOr(4, 0, &type) || !c.numberOr(5, 0.1, &guess)) return c.failure();
    type = type != 0 ? 1 : 0;
    if (n <= 0) return c.error(ErrorKind::Num);
    auto f = [&](double r) -> double {
        if (std::fabs(r) < 1e-12) return pv + pmt * n + fv;
        const double g = std::pow(1 + r, n);
        return pv * g + pmt * (1 + r * type) * (g - 1) / r + fv;
    };
    // Newton's method from the guess; then a search along the line for a sign change when it does not settle
    double r = guess;
    for (int i = 0; i < 200; ++i) {
        const double h = std::max(1e-8, std::fabs(r) * 1e-8);
        const double fr = f(r), slope = (f(r + h) - f(r - h)) / (2 * h);
        if (!std::isfinite(fr) || slope == 0 || !std::isfinite(slope)) break;
        const double next = r - fr / slope;
        if (next <= -1) {
            r = (r - 1) / 2;
            continue;
        }
        if (std::fabs(next - r) < 1e-14) {
            r = next;
            if (std::fabs(f(r)) <= 1e-7 * std::max(1.0, std::fabs(pv) + std::fabs(pmt * n) + std::fabs(fv))) return numberResult(r);
            break;
        }
        r = next;
    }
    double lo = -0.999999, hi = 10;
    double flo = f(lo), fhi = f(hi);
    for (int i = 0; i < 40 && std::isfinite(flo) && std::isfinite(fhi) && (flo > 0) == (fhi > 0); ++i) hi *= 2, fhi = f(hi);
    if (!std::isfinite(flo) || !std::isfinite(fhi) || (flo > 0) == (fhi > 0)) return c.error(ErrorKind::Num);
    for (int i = 0; i < 300; ++i) {
        const double mid = (lo + hi) / 2, fm = f(mid);
        if ((fm > 0) == (flo > 0)) lo = mid, flo = fm;
        else hi = mid;
    }
    return numberResult((lo + hi) / 2);
}

Value fnRri(Call &c)
{
    double n, pv, fv;
    if (!c.number(0, &n) || !c.number(1, &pv) || !c.number(2, &fv)) return c.failure();
    if (n <= 0 || pv == 0 || (fv / pv) < 0) return c.error(ErrorKind::Num);
    return numberResult(std::pow(fv / pv, 1 / n) - 1);
}

Value fnPDuration(Call &c)
{
    double rate, pv, fv;
    if (!c.number(0, &rate) || !c.number(1, &pv) || !c.number(2, &fv)) return c.failure();
    if (rate <= 0 || pv <= 0 || fv <= 0) return c.error(ErrorKind::Num);
    return numberResult((std::log(fv) - std::log(pv)) / std::log1p(rate));
}

} // namespace

void registerMoreFinanceFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"COUPDAYBS", 3, 4, "vvvv", 0, fnCoupDayBs});
    r.push_back({"COUPDAYS", 3, 4, "vvvv", 0, fnCoupDays});
    r.push_back({"COUPDAYSNC", 3, 4, "vvvv", 0, fnCoupDaysNc});
    r.push_back({"COUPNCD", 3, 4, "vvvv", 0, fnCoupNcd});
    r.push_back({"COUPNUM", 3, 4, "vvvv", 0, fnCoupNum});
    r.push_back({"COUPPCD", 3, 4, "vvvv", 0, fnCoupPcd});
    r.push_back({"PRICE", 6, 7, "vvvvvvv", 0, fnPrice});
    r.push_back({"YIELD", 6, 7, "vvvvvvv", kFnIterative, fnYield});
    r.push_back({"DURATION", 5, 6, "vvvvvv", 0, fnDuration});
    r.push_back({"MDURATION", 5, 6, "vvvvvv", 0, fnMDuration});
    r.push_back({"PRICEDISC", 4, 5, "vvvvv", 0, fnPriceDisc});
    r.push_back({"PRICEMAT", 5, 6, "vvvvvv", 0, fnPriceMat});
    r.push_back({"YIELDDISC", 4, 5, "vvvvv", 0, fnYieldDisc});
    r.push_back({"YIELDMAT", 5, 6, "vvvvvv", 0, fnYieldMat});
    r.push_back({"DISC", 4, 5, "vvvvv", 0, fnDisc});
    r.push_back({"INTRATE", 4, 5, "vvvvv", 0, fnIntRate});
    r.push_back({"RECEIVED", 4, 5, "vvvvv", 0, fnReceived});
    r.push_back({"TBILLPRICE", 3, 3, "vvv", 0, fnTBillPrice});
    r.push_back({"TBILLYIELD", 3, 3, "vvv", 0, fnTBillYield});
    r.push_back({"TBILLEQ", 3, 3, "vvv", 0, fnTBillEq});
    r.push_back({"DOLLARDE", 2, 2, "vv", 0, fnDollarDe});
    r.push_back({"DOLLARFR", 2, 2, "vv", 0, fnDollarFr});
    r.push_back({"ACCRINT", 6, 8, "vvvvvvvv", 0, fnAccrInt});
    r.push_back({"ACCRINTM", 3, 5, "vvvvv", 0, fnAccrIntM});
    r.push_back({"ODDFPRICE", 8, 9, "vvvvvvvvv", 0, fnOddFPrice});
    r.push_back({"ODDFYIELD", 8, 9, "vvvvvvvvv", kFnIterative, fnOddFYield});
    r.push_back({"ODDLPRICE", 7, 8, "vvvvvvvv", 0, fnOddLPrice});
    r.push_back({"ODDLYIELD", 7, 8, "vvvvvvvv", kFnIterative, fnOddLYield});
    r.push_back({"CUMIPMT", 6, 6, "vvvvvv", 0, fnCumIpmt});
    r.push_back({"CUMPRINC", 6, 6, "vvvvvv", 0, fnCumPrinc});
    r.push_back({"ISPMT", 4, 4, "vvvv", 0, fnIspmt});
    r.push_back({"FVSCHEDULE", 2, 2, "va", 0, fnFvSchedule});
    r.push_back({"MIRR", 3, 3, "rvv", 0, fnMirr});
    r.push_back({"DB", 4, 5, "vvvvv", 0, fnDb});
    r.push_back({"AMORLINC", 6, 7, "vvvvvvv", 0, fnAmorLinc});
    r.push_back({"AMORDEGRC", 6, 7, "vvvvvvv", 0, fnAmorDegrc});
    r.push_back({"RATE", 3, 6, "vvvvvv", kFnIterative, fnRate});
    r.push_back({"RRI", 3, 3, "vvv", kFnFuture, fnRri});
    r.push_back({"PDURATION", 3, 3, "vvv", kFnFuture, fnPDuration});
}

} // namespace jp::calc
