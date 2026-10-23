// The financial functions that have a closed form: loans and savings, net present value, and straight-line depreciation. (The ones that are solved by trying
// numbers, IRR and RATE, give the digits of the iteration the original makes, which are not reproduced here.)
#include "calc/machine.h"

#include <cmath>

namespace jp::calc {

namespace {

// These are worked out in the widest floating-point type the platform has (80 bits on x86, as the original works them out; the same as a double elsewhere), so
// that the last digits of a loan's payment are those of the files: in double, they are off by some units of the last place.
using real = long double;

// (1 + rate) to the power n.
real growth(real rate, real n) { return std::pow(1 + rate, n); }

real futureValue(real rate, real n, real pmt, real pv, real type)
{
    if (rate == 0) return -(pv + pmt * n);
    const real g = growth(rate, n);
    return -(pv * g + pmt * (1 + rate * type) * (g - 1) / rate);
}

bool payment(real rate, real n, real pv, real fv, real type, real *out)
{
    if (n == 0) return false;
    if (rate == 0) {
        *out = -(pv + fv) / n;
        return true;
    }
    const real g = growth(rate, n);
    *out = -(fv + pv * g) * rate / ((1 + rate * type) * (g - 1));
    return true;
}

Value fnPmt(Call &c)
{
    double rate, n, pv, fv, type;
    if (!c.number(0, &rate) || !c.number(1, &n) || !c.number(2, &pv) || !c.numberOr(3, 0, &fv) || !c.numberOr(4, 0, &type)) return c.failure();
    real p;
    if (!payment(rate, n, pv, fv, type != 0 ? 1 : 0, &p)) return c.error(rate == 0 ? ErrorKind::Div0 : ErrorKind::Num);
    return numberResult(double(p));
}

Value fnFv(Call &c)
{
    double rate, n, pmt, pv, type;
    if (!c.number(0, &rate) || !c.number(1, &n) || !c.number(2, &pmt) || !c.numberOr(3, 0, &pv) || !c.numberOr(4, 0, &type)) return c.failure();
    return numberResult(double(futureValue(rate, n, pmt, pv, type != 0 ? 1 : 0)));
}

Value fnPv(Call &c)
{
    double rate, n, pmt, fv, type;
    if (!c.number(0, &rate) || !c.number(1, &n) || !c.number(2, &pmt) || !c.numberOr(3, 0, &fv) || !c.numberOr(4, 0, &type)) return c.failure();
    type = type != 0 ? 1 : 0;
    if (rate == 0) return numberResult(-(fv + pmt * n));
    const real g = growth(rate, n);
    return numberResult(double(-(fv + pmt * (1 + rate * type) * (g - 1) / rate) / g));
}

Value fnNper(Call &c)
{
    double rate, pmt, pv, fv, type;
    if (!c.number(0, &rate) || !c.number(1, &pmt) || !c.number(2, &pv) || !c.numberOr(3, 0, &fv) || !c.numberOr(4, 0, &type)) return c.failure();
    type = type != 0 ? 1 : 0;
    if (rate == 0) {
        if (pmt == 0) return c.error(ErrorKind::Num);
        return numberResult(-(pv + fv) / pmt);
    }
    const double top = pmt * (1 + rate * type) - fv * rate, bottom = pmt * (1 + rate * type) + pv * rate;
    if (bottom == 0 || top / bottom <= 0 || 1 + rate <= 0) return c.error(ErrorKind::Num);
    return numberResult(std::log(top / bottom) / std::log(1 + rate));
}

// The interest and the principal of a payment (IPMT, PPMT): the same arguments, and the number of the payment.
Value interestOrPrincipal(Call &c, bool principal)
{
    double rate, per, n, pv, fv, type;
    if (!c.number(0, &rate) || !c.number(1, &per) || !c.number(2, &n) || !c.number(3, &pv) || !c.numberOr(4, 0, &fv) || !c.numberOr(5, 0, &type)) return c.failure();
    type = type != 0 ? 1 : 0;
    per = std::trunc(per);
    if (per < 1 || per > n) return c.error(ErrorKind::Num);
    real pmt;
    if (!payment(rate, n, pv, fv, type, &pmt)) return c.error(rate == 0 ? ErrorKind::Div0 : ErrorKind::Num);
    real owed;
    if (per == 1)
        owed = type > 0 ? 0 : -pv;
    else if (type > 0)
        owed = futureValue(rate, per - 2, pmt, pv, 1) - pmt;
    else
        owed = futureValue(rate, per - 1, pmt, pv, 0);
    const real interest = owed * rate;
    return numberResult(double(principal ? pmt - interest : interest));
}
Value fnIpmt(Call &c) { return interestOrPrincipal(c, false); }
Value fnPpmt(Call &c) { return interestOrPrincipal(c, true); }

Value fnNpv(Call &c)
{
    double rate;
    if (!c.number(0, &rate)) return c.failure();
    std::vector<double> values;
    Value error;
    if (!gatherNumbers(c, 1, &values, &error)) return error;
    if (rate == -1) return c.error(ErrorKind::Div0);
    real sum = 0;
    for (size_t i = 0; i < values.size(); ++i) sum += values[i] / growth(rate, real(i + 1));
    return numberResult(double(sum));
}

Value fnSln(Call &c)
{
    double cost, salvage, life;
    if (!c.number(0, &cost) || !c.number(1, &salvage) || !c.number(2, &life)) return c.failure();
    if (life == 0) return c.error(ErrorKind::Div0);
    return numberResult((cost - salvage) / life);
}

Value fnSyd(Call &c)
{
    double cost, salvage, life, per;
    if (!c.number(0, &cost) || !c.number(1, &salvage) || !c.number(2, &life) || !c.number(3, &per)) return c.failure();
    if (life <= 0 || per <= 0 || per > life) return c.error(ErrorKind::Num);
    return numberResult((cost - salvage) * (life - per + 1) * 2 / (life * (life + 1)));
}

Value fnEffect(Call &c)
{
    double nominal, periods;
    if (!c.number(0, &nominal) || !c.number(1, &periods)) return c.failure();
    periods = std::trunc(periods);
    if (nominal <= 0 || periods < 1) return c.error(ErrorKind::Num);
    return numberResult(double(growth(nominal / periods, periods) - 1));
}

Value fnNominal(Call &c)
{
    double effective, periods;
    if (!c.number(0, &effective) || !c.number(1, &periods)) return c.failure();
    periods = std::trunc(periods);
    if (effective <= 0 || periods < 1) return c.error(ErrorKind::Num);
    return numberResult(periods * (std::exp(std::log(1 + effective) / periods) - 1));
}

} // namespace

void registerFinanceFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"PMT", 3, 5, "vvvvv", 0, fnPmt});
    r.push_back({"FV", 3, 5, "vvvvv", 0, fnFv});
    r.push_back({"PV", 3, 5, "vvvvv", 0, fnPv});
    r.push_back({"NPER", 3, 5, "vvvvv", 0, fnNper});
    r.push_back({"IPMT", 4, 6, "vvvvvv", 0, fnIpmt});
    r.push_back({"PPMT", 4, 6, "vvvvvv", 0, fnPpmt});
    r.push_back({"NPV", 2, 255, "vr", 0, fnNpv});
    r.push_back({"SLN", 3, 3, "vvv", 0, fnSln});
    r.push_back({"SYD", 4, 4, "vvvv", 0, fnSyd});
    r.push_back({"EFFECT", 2, 2, "vv", 0, fnEffect});
    r.push_back({"NOMINAL", 2, 2, "vv", 0, fnNominal});
}

} // namespace jp::calc
