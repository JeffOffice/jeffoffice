#include "office/chart_trend.h"

#include <QCoreApplication>

#include <algorithm>
#include <cmath>
#include <deque>

namespace jp::office {

namespace {

constexpr int kMostPoints = 100000;

// Least squares for a polynomial of `degree` through the points, by the normal equations solved with partial pivoting. Empty if there is not enough to go on.
QVector<double> polyFit(const QVector<double> &x, const QVector<double> &y, int degree, bool fixedFirst, double first)
{
    const int n = int(x.size());
    const int lo = fixedFirst ? 1 : 0;   // with a fixed intercept the constant term is not fitted
    const int m = degree + 1 - lo;
    if (m < 1 || n < m) return {};
    // scale x so that high powers stay in range
    double scale = 0;
    for (double v : x) scale = std::max(scale, std::fabs(v));
    if (scale <= 0) scale = 1;
    QVector<QVector<double>> a(m, QVector<double>(m + 1, 0));
    for (int i = 0; i < n; ++i) {
        const double xs = x[i] / scale, target = y[i] - (fixedFirst ? first : 0);
        double powers[16];   // (the degree is 6 at the most)
        const int powerCount = 2 * degree + 2;
        powers[0] = 1;
        for (int k = 1; k < powerCount; ++k) powers[k] = powers[k - 1] * xs;
        for (int r = 0; r < m; ++r) {
            for (int c = 0; c < m; ++c) a[r][c] += powers[r + lo + c + lo];
            a[r][m] += powers[r + lo] * target;
        }
    }
    for (int col = 0; col < m; ++col) {
        int pivot = col;
        for (int r = col + 1; r < m; ++r)
            if (std::fabs(a[r][col]) > std::fabs(a[pivot][col])) pivot = r;
        if (std::fabs(a[pivot][col]) < 1e-12) return {};
        std::swap(a[pivot], a[col]);
        for (int r = 0; r < m; ++r) {
            if (r == col) continue;
            const double f = a[r][col] / a[col][col];
            for (int c = col; c <= m; ++c) a[r][c] -= f * a[col][c];
        }
    }
    QVector<double> out(degree + 1, 0);
    if (fixedFirst) out[0] = first;
    for (int r = 0; r < m; ++r) out[r + lo] = a[r][m] / a[r][r] / std::pow(scale, r + lo);
    return out;
}

double polyAt(const QVector<double> &c, double x)
{
    double v = 0;
    for (int k = int(c.size()) - 1; k >= 0; --k) v = v * x + c[k];
    return v;
}

QString num(double v)
{
    if (std::fabs(v) < 1e-12) v = 0;
    return QString::number(v, 'g', 5);
}

// An equation's terms written the way a person writes them: the numbers rounded first (a term that rounds to nothing is left out), the sign chosen from the rounded number, and a
// coefficient of 1 not written ("x", "-x"). `part` is what follows the number ("x", "x²", "ln(x)", or nothing for a constant).
struct Part {
    double value;
    QString text;
};
QString written(const QVector<Part> &parts)
{
    QString out;
    double biggest = 0;
    for (const Part &p : parts) biggest = std::max(biggest, std::fabs(p.value));
    for (const Part &p : parts) {
        // (what is a millionth of the biggest is the noise of the fit, not a term)
        const QString mag = std::fabs(p.value) < biggest * 1e-6 ? QStringLiteral("0") : num(std::fabs(p.value));
        if (mag == QLatin1String("0")) continue;
        const bool negative = p.value < 0;
        const QString body = p.text.isEmpty() ? mag : (mag == QLatin1String("1") ? p.text : mag + p.text);
        if (out.isEmpty()) out = (negative ? QStringLiteral("-") : QString()) + body;
        else out += (negative ? QStringLiteral(" - ") : QStringLiteral(" + ")) + body;
    }
    return QStringLiteral("y = ") + (out.isEmpty() ? QStringLiteral("0") : out);
}

QString superscript(int n)
{
    static const QChar digits[] = {u'⁰', u'¹', u'²', u'³', u'⁴', u'⁵', u'⁶', u'⁷', u'⁸', u'⁹'};
    return n >= 0 && n <= 9 ? QString(digits[n]) : QString::number(n);
}

} // namespace

QString trendlineName(const ChartTrendline &t, const QString &seriesName)
{
    if (!t.name.isEmpty()) return t.name;
    if (t.type == QLatin1String("exp")) return QCoreApplication::translate("Chart", "Exponential (%1)").arg(seriesName);
    if (t.type == QLatin1String("log")) return QCoreApplication::translate("Chart", "Logarithmic (%1)").arg(seriesName);
    if (t.type == QLatin1String("poly")) return QCoreApplication::translate("Chart", "Poly. (%1)").arg(seriesName);
    if (t.type == QLatin1String("power")) return QCoreApplication::translate("Chart", "Power (%1)").arg(seriesName);
    if (t.type == QLatin1String("movingAvg")) return QCoreApplication::translate("Chart", "%1 per. Mov. Avg. (%2)").arg(t.period).arg(seriesName);
    return QCoreApplication::translate("Chart", "Linear (%1)").arg(seriesName);
}

static TrendFit fitTrendUncached(const ChartTrendline &t, const QVector<double> &xAll, const QVector<double> &yAll, int samples)
{
    TrendFit fit;
    QVector<double> x, y;
    const int n = int(std::min(xAll.size(), yAll.size()));
    for (int i = 0; i < n && x.size() < kMostPoints; ++i)
        if (std::isfinite(xAll[i]) && std::isfinite(yAll[i])) {
            x.append(xAll[i]);
            y.append(yAll[i]);
        }
    if (x.size() < 2) return fit;
    const double x0 = *std::min_element(x.begin(), x.end()) - t.backward, x1 = *std::max_element(x.begin(), x.end()) + t.forward;
    samples = std::clamp(samples, 2, 1000);

    if (t.type == QLatin1String("movingAvg")) {
        const int period = std::clamp(t.period, 2, 255);
        if (x.size() <= period) return fit;
        // the points in the order of x (the order they come in when it is already), each the mean of it and the period - 1 before it, by a running sum
        QVector<double> xo, yo;
        xo.reserve(x.size());
        yo.reserve(y.size());
        if (std::is_sorted(x.begin(), x.end())) {
            xo = x;
            yo = y;
        } else {
            QVector<int> order(x.size());
            for (int i = 0; i < order.size(); ++i) order[i] = i;
            std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return x[a] < x[b]; });
            for (const int i : order) {
                xo.append(x[i]);
                yo.append(y[i]);
            }
        }
        double sum = 0;
        for (int i = 0; i < xo.size(); ++i) {
            sum += yo[i];
            if (i >= period) sum -= yo[i - period];
            if (i >= period - 1) fit.curve.append(QPointF(xo[i], sum / period));
        }
        fit.ok = fit.curve.size() >= 1;
        return fit;
    }

    // the line is fitted in a space where it is straight (or a polynomial); `back` takes it to the chart's values
    QVector<double> tx = x, ty = y;
    const bool exp = t.type == QLatin1String("exp"), log = t.type == QLatin1String("log"), power = t.type == QLatin1String("power"), poly = t.type == QLatin1String("poly");
    if (exp || power)
        for (double v : y)
            if (v <= 0) return fit;
    if (log || power)
        for (double v : x)
            if (v <= 0) return fit;
    if (exp || power)
        for (double &v : ty) v = std::log(v);
    if (log || power)
        for (double &v : tx) v = std::log(v);
    const int degree = poly ? std::clamp(t.order, 2, 6) : 1;
    const bool fixed = t.hasIntercept;
    double first = t.intercept;
    if (exp) {
        if (t.intercept <= 0) {
            if (t.hasIntercept) return fit;
        } else {
            first = std::log(t.intercept);
        }
    }
    if (power) {
        if (t.hasIntercept) return fit;   // (a power curve has no say in its intercept)
    }
    const QVector<double> c = polyFit(tx, ty, degree, fixed && !power && !(exp && t.intercept <= 0), first);
    if (c.isEmpty()) return fit;
    auto back = [&](double xv) -> double {
        const double xi = log || power ? std::log(xv) : xv;
        const double v = polyAt(c, xi);
        return exp || power ? std::exp(v) : v;
    };
    // R-squared in the space the line was fitted in
    double mean = 0;
    for (double v : ty) mean += v;
    mean /= ty.size();
    double total = 0, rest = 0;
    for (int i = 0; i < tx.size(); ++i) {
        const double d = ty[i] - mean, r = ty[i] - polyAt(c, tx[i]);
        total += d * d;
        rest += r * r;
    }
    fit.rSquared = total > 0 ? 1 - rest / total : 1;
    fit.hasRSquared = true;
    // the line itself: two ends for a straight one, else along x (a logarithmic or power curve begins above 0)
    double from = x0;
    if ((log || power) && from <= 0) from = std::max(*std::min_element(x.begin(), x.end()) * 0.5, 1e-9);
    const int count = !poly && !exp && !log && !power ? 2 : samples;
    for (int i = 0; i < count; ++i) {
        const double xv = from + (x1 - from) * i / (count - 1);
        const double yv = back(xv);
        if (std::isfinite(yv)) fit.curve.append(QPointF(xv, yv));
    }
    // the words
    if (poly) {
        QVector<Part> parts;
        for (int k = degree; k >= 0; --k) parts.append({c[k], k == 0 ? QString() : k == 1 ? QStringLiteral("x") : QStringLiteral("x") + superscript(k)});
        fit.equation = written(parts);
    } else if (exp) {
        const QString a = num(std::exp(c[0])), b = num(c[1]);
        fit.equation = QStringLiteral("y = %1e^%2x").arg(a == QLatin1String("1") ? QString() : a, b);
    } else if (log) {
        fit.equation = written({{c[1], QStringLiteral("ln(x)")}, {c[0], QString()}});
    } else if (power) {
        const QString a = num(std::exp(c[0]));
        fit.equation = QStringLiteral("y = %1x^%2").arg(a == QLatin1String("1") ? QString() : a, num(c[1]));
    } else {
        fit.equation = written({{c[1], QStringLiteral("x")}, {c[0], QString()}});
    }
    fit.coefficients = c;
    if (exp || power) fit.coefficients[0] = std::exp(c[0]);
    fit.ok = fit.curve.size() >= 2;
    return fit;
}

// A chart is drawn again and again (each frame of a deck, each time a picture is made): the fits of the last few lines are kept, by what they were made from.
TrendFit fitTrend(const ChartTrendline &t, const QVector<double> &x, const QVector<double> &y, int samples)
{
    struct Entry {
        ChartTrendline t;
        QVector<double> x, y;
        int samples;
        TrendFit fit;
    };
    thread_local std::deque<Entry> cache;
    for (const Entry &e : cache)
        if (e.samples == samples && e.t.type == t.type && e.t.order == t.order && e.t.period == t.period && e.t.forward == t.forward && e.t.backward == t.backward && e.t.hasIntercept == t.hasIntercept &&
            e.t.intercept == t.intercept && e.x == x && e.y == y)
            return e.fit;
    TrendFit fit = fitTrendUncached(t, x, y, samples);
    if (x.size() <= 20000) {
        cache.push_back({t, x, y, samples, fit});
        if (cache.size() > 16) cache.pop_front();
    }
    return fit;
}

} // namespace jp::office
