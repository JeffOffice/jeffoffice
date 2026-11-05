#include "core/goalseek.h"

#include <QString>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

GoalSeekResult goalSeek(const std::function<std::optional<double>(double)> &trial, double start, double target, double tolerance, int maxTrials)
{
    GoalSeekResult out;
    out.input = start;
    // (the search goes on past the tolerance to a much closer number, so that the number found is a tidy one; it is the tolerance that says whether it was found)
    const double tight = std::min(tolerance, 1e-10 * std::max(1.0, std::fabs(target)));
    bool haveBest = false;
    auto g = [&](double x, double *miss) -> bool {
        if (out.trials >= maxTrials) return false;
        ++out.trials;
        const std::optional<double> v = trial(x);
        if (!v || !std::isfinite(*v)) return false;
        *miss = *v - target;
        if (!haveBest || std::fabs(*miss) < std::fabs(out.result - target)) {
            haveBest = true;
            out.input = x;
            out.result = *v;
        }
        return true;
    };
    auto done = [&] { return haveBest && std::fabs(out.result - target) <= tight; };
    auto finish = [&] {
        out.found = haveBest && std::fabs(out.result - target) <= tolerance;
        // a number with fewer digits that does as well is the tidier answer (4, not 4.00000000000666)
        if (out.found) {
            const double best = out.input, bestResult = out.result;
            for (const int digits : {6, 9, 12}) {
                const double tidy = QString::number(best, 'g', digits).toDouble();
                if (tidy == best) break;
                const std::optional<double> v = trial(tidy);
                if (v && std::isfinite(*v) && std::fabs(*v - target) <= tolerance) {
                    out.input = tidy;
                    out.result = *v;
                    return out;
                }
            }
            // (the last number tried is not the best: put the best back)
            trial(best);
            out.input = best;
            out.result = bestResult;
        }
        return out;
    };

    double x0 = start, g0 = 0;
    if (!g(x0, &g0)) return finish();
    if (done()) return finish();

    // the secant method, from the start and a step away
    double x1 = x0 + std::max(std::fabs(x0) * 1e-4, 1e-4), g1 = 0;
    bool secant = g(x1, &g1);
    if (done()) return finish();
    for (int i = 0; secant && i < 60 && out.trials < maxTrials; ++i) {
        const double slope = (g1 - g0) / (x1 - x0);
        if (slope == 0 || !std::isfinite(slope)) break;
        const double x2 = x1 - g1 / slope;
        if (!std::isfinite(x2) || std::fabs(x2) > 1e15) break;
        double g2 = 0;
        if (!g(x2, &g2)) break;
        if (done()) return finish();
        x0 = x1;
        g0 = g1;
        x1 = x2;
        g1 = g2;
    }
    if (done()) return finish();

    // a number on each side of the target: widen the steps away from the start until the formula crosses the target, then halve the gap
    double lo = start, glo = 0, hi = start, ghi = 0;
    if (!g(start, &glo)) return finish();
    ghi = glo;
    bool crossed = false;
    double width = std::max(std::fabs(start) * 0.1, 1.0);
    for (int k = 0; k < 80 && !crossed && out.trials < maxTrials; ++k, width *= 2) {
        for (const double side : {-1.0, 1.0}) {
            const double x = start + side * width;
            double gx = 0;
            if (!g(x, &gx)) continue;
            if ((gx < 0) != (glo < 0) || gx == 0) {
                lo = start;
                hi = x;
                ghi = gx;
                crossed = true;
                break;
            }
        }
    }
    if (!crossed) return finish();
    // Illinois: the false-position method that does not stall
    double fLo = glo, fHi = ghi;
    int side = 0;
    for (int i = 0; i < 100 && out.trials < maxTrials; ++i) {
        if (fHi == fLo) break;
        double x = (lo * fHi - hi * fLo) / (fHi - fLo);
        if (!std::isfinite(x) || x == lo || x == hi) x = lo + (hi - lo) / 2;
        double gx = 0;
        if (!g(x, &gx)) x = lo + (hi - lo) / 2, g(x, &gx);
        if (done()) break;
        if ((gx < 0) == (fHi < 0)) {
            hi = x;
            fHi = gx;
            if (side == -1) fLo /= 2;
            side = -1;
        } else {
            lo = x;
            fLo = gx;
            if (side == 1) fHi /= 2;
            side = 1;
        }
        if (std::fabs(hi - lo) <= 1e-15 * std::max(std::fabs(lo), std::fabs(hi))) break;
    }
    return finish();
}

} // namespace jp::crunch
