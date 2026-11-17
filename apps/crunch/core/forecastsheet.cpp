#include "core/forecastsheet.h"

#include <QCoreApplication>
#include "calc/numfmt.h"

#include <QLocale>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

namespace {

QString number(double v)
{
    return QString::number(v, 'g', 17);
}

} // namespace

bool readForecastInput(const jp::office::sheet::CellRange &timeline, const ForecastCellReader &readTimeline, const jp::office::sheet::CellRange &values, const ForecastCellReader &readValues, ForecastInput *out, QString *problem, long long maxPoints, bool *sampled)
{
    using jp::office::sheet::cellName;
    if (!timeline.valid || !values.valid) {
        *problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "Choose the cells that hold the timeline and the cells that hold the values.");
        return false;
    }
    const qint64 tr = qint64(timeline.row2) - timeline.row1 + 1, tc = qint64(timeline.col2) - timeline.col1 + 1;
    const qint64 vr = qint64(values.row2) - values.row1 + 1, vc = qint64(values.col2) - values.col1 + 1;
    if ((tr != 1 && tc != 1) || (vr != 1 && vc != 1)) {
        *problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The timeline and the values must each be one column or one row of cells.");
        return false;
    }
    const qint64 tn = tr * tc, vn = vr * vc;
    if (tn != vn) {
        *problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The timeline and the values need the same number of cells.");
        return false;
    }
    if (tn > kMaxForecastInput) {
        *problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The data has more than 1,000,000 points. A forecast is not made from so many; choose a smaller range.");
        return false;
    }
    out->pairs.clear();
    // only the last `maxPoints` cells are read when it is asked (the picture of the dialog: the last of a long series is what the forecast follows)
    const qint64 first = maxPoints > 0 && tn > maxPoints ? tn - maxPoints : 0;
    if (sampled) *sampled = first > 0;
    out->pairs.reserve(size_t(tn - first));
    for (qint64 k = first; k < tn; ++k) {
        const quint32 trow = timeline.row1 + quint32(tr == 1 ? 0 : k), tcol = timeline.col1 + quint32(tr == 1 ? k : 0);
        const quint32 vrow = values.row1 + quint32(vr == 1 ? 0 : k), vcol = values.col1 + quint32(vr == 1 ? k : 0);
        const jp::calc::Value t = readTimeline(trow, tcol), v = readValues(vrow, vcol);
        if (!t.isNumber() || !std::isfinite(t.number())) {
            *problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The timeline needs dates or numbers, and %1 holds something else.").arg(cellName(trow, tcol));
            return false;
        }
        if (!v.isNumber() && !v.isEmpty()) {
            *problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The values need numbers, and %1 holds text or an error. Change it to a number, or empty the cell.").arg(cellName(vrow, vcol));
            return false;
        }
        out->pairs.push_back({t.number(), v.isNumber() ? v.number() : std::numeric_limits<double>::quiet_NaN()});
    }
    return true;
}

double defaultForecastEnd(const ForecastInput &input, jp::calc::DateSystem system)
{
    jp::calc::ets::Prepared p;
    if (jp::calc::ets::prepare(input.pairs, jp::calc::ets::Aggregation::Average, true, system, &p) != jp::calc::ets::Problem::None) {
        double last = 0;
        for (const auto &x : input.pairs) last = std::max(last, x.first);
        return last;
    }
    const long long steps = std::min<long long>(kMaxForecastPoints, std::max<long long>(1, p.timeline.n / 2));
    return p.timeline.timeAt(p.timeline.n - 1 + steps);
}

ForecastResult computeForecast(const ForecastInput &input, const ForecastSettings &settings, jp::calc::DateSystem system)
{
    namespace ets = jp::calc::ets;
    ForecastResult r;
    const bool early = std::isfinite(settings.start);
    for (const auto &x : input.pairs) {
        if (early && !(x.first < settings.start)) r.later.push_back(x);
        else r.used.push_back(x);
    }
    std::stable_sort(r.used.begin(), r.used.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    std::stable_sort(r.later.begin(), r.later.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    if (!std::isfinite(settings.end)) {
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "Write the time the forecast ends.");
        return r;
    }
    switch (ets::prepare(r.used, settings.aggregation, settings.interpolate, system, &r.prepared)) {
    case ets::Problem::None: break;
    case ets::Problem::TooFew:
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "There are too few points to make a forecast from. Use at least 3 points with values, each at its own time.");
        return r;
    case ets::Problem::Uneven:
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The timeline is not evenly spaced. The steps between its dates or numbers must be the same size (or whole months apart, on the same day of each month). Duplicate times are combined; make the other steps equal.");
        return r;
    case ets::Problem::TooMany:
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The timeline spans more than 1,000,000 steps, which is too many to make a forecast from.");
        return r;
    }
    const long long n = r.prepared.timeline.n;
    r.season = settings.seasonality == 1 ? ets::detectSeason(r.prepared.values) : std::max(0, settings.seasonality);
    if (r.season >= 2 && n < 2LL * r.season) {
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The season is too long. The data must hold at least two whole seasons.");
        return r;
    }
    if (!(settings.confidence > 0 && settings.confidence < 1)) {
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The confidence interval must be more than 0% and less than 100%.");
        return r;
    }
    r.model = ets::fitCached(r.prepared.values, r.season);
    if (!r.model) {
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "A forecast could not be made from this data.");
        return r;
    }
    const ets::Timeline &tl = r.prepared.timeline;
    long long first = n;
    if (early) first = std::max<long long>(n, (long long)std::llround(tl.position(settings.start)));
    const double endPos = tl.position(settings.end);
    if (!std::isfinite(endPos) || endPos > double(n) + double(ets::kMaxHorizon)) {
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The forecast would reach more than 100,000 steps past the last point, which is too far. Choose an earlier end.");
        return r;
    }
    const long long last = (long long)std::floor(endPos + 1e-9);
    if (last < first) {
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The forecast must end after the last point it is made from. Choose a later end.");
        return r;
    }
    if (last - first + 1 > kMaxForecastPoints) {
        r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "The forecast would be longer than 10,000 points. Choose an earlier end.");
        return r;
    }
    for (long long i = first; i <= last; ++i) {
        ForecastPoint p;
        p.time = tl.timeAt(i);
        double f, w;
        if (!r.model->forecast(i - (n - 1), &f) || !r.model->interval(i - (n - 1), settings.confidence, &w)) {
            r.problem = QCoreApplication::translate("jp::crunch::ForecastSheet", "A forecast could not be made from this data.");
            return r;
        }
        p.forecast = f;
        p.lower = f - w;
        p.upper = f + w;
        r.points.push_back(p);
    }
    return r;
}

ForecastSheetPlan planForecastSheet(const ForecastResult &r, const ForecastSettings &s)
{
    ForecastSheetPlan plan;
    struct Row {
        double time;
        double value;
        bool forecast;
    };
    const double nan = std::numeric_limits<double>::quiet_NaN();
    std::vector<Row> rows;
    for (const auto &x : r.used) rows.push_back({x.first, x.second, false});
    const int used = int(rows.size());
    const double tol = r.prepared.timeline.months ? 0.5 : 1e-6 * r.prepared.timeline.step;
    size_t li = 0;
    for (const ForecastPoint &p : r.points) {
        while (li < r.later.size() && r.later[li].first < p.time - tol) {
            rows.push_back({r.later[li].first, r.later[li].second, false});
            ++li;
        }
        if (li < r.later.size() && std::fabs(r.later[li].first - p.time) <= tol) {
            // (several rows at one time are all kept, the forecast being on the first)
            rows.push_back({r.later[li].first, r.later[li].second, true});
            ++li;
            while (li < r.later.size() && std::fabs(r.later[li].first - p.time) <= tol) {
                rows.push_back({r.later[li].first, r.later[li].second, false});
                ++li;
            }
        } else {
            rows.push_back({p.time, nan, true});
        }
    }
    for (; li < r.later.size(); ++li) rows.push_back({r.later[li].first, r.later[li].second, false});
    plan.dataRows = int(rows.size());
    plan.usedRows = used;
    const QString seasonArg = QString::number(s.seasonality), completion = s.interpolate ? QStringLiteral("1") : QStringLiteral("0"), agg = QString::number(int(s.aggregation));
    const QString tail = QStringLiteral(",") + seasonArg + QLatin1Char(',') + completion + QLatin1Char(',') + agg + QLatin1Char(')');
    const int usedLast = 1 + used;   // the sheet row (from 1) of the last data row the forecast reads
    const QString values = QStringLiteral("$B$2:$B$") + QString::number(usedLast), times = QStringLiteral("$A$2:$A$") + QString::number(usedLast);
    plan.table.append({QCoreApplication::translate("jp::crunch::ForecastSheet", "Timeline"), QCoreApplication::translate("jp::crunch::ForecastSheet", "Values"), QCoreApplication::translate("jp::crunch::ForecastSheet", "Forecast"), QCoreApplication::translate("jp::crunch::ForecastSheet", "Lower Confidence Bound"), QCoreApplication::translate("jp::crunch::ForecastSheet", "Upper Confidence Bound")});
    plan.forecastFirstRow = plan.forecastLastRow = -1;
    size_t forecastIndex = 0;
    for (int i = 0; i < int(rows.size()); ++i) {
        const int sheetRow = i + 2;
        const QString n = QString::number(sheetRow);
        QStringList line = {number(rows[size_t(i)].time), std::isnan(rows[size_t(i)].value) ? QString() : number(rows[size_t(i)].value), QString(), QString(), QString()};
        if (rows[size_t(i)].forecast) {
            const QString head = QStringLiteral("FORECAST.ETS");
            line[2] = QStringLiteral("=") + head + QStringLiteral("(A") + n + QLatin1Char(',') + values + QLatin1Char(',') + times + tail;
            const QString conf = QStringLiteral("FORECAST.ETS.CONFINT(A") + n + QLatin1Char(',') + values + QLatin1Char(',') + times + QLatin1Char(',') + QString::number(s.confidence, 'g', 15) + tail;
            line[3] = QStringLiteral("=C") + n + QLatin1Char('-') + conf;
            line[4] = QStringLiteral("=C") + n + QLatin1Char('+') + conf;
            if (plan.forecastFirstRow < 0) plan.forecastFirstRow = sheetRow - 1;
            plan.forecastLastRow = sheetRow - 1;
            ++forecastIndex;
        } else if (i == used - 1 && !std::isnan(rows[size_t(i)].value)) {
            // the last point of the data joins the line of the forecast to the line of the values
            line[2] = line[3] = line[4] = QStringLiteral("=B") + n;
        }
        plan.table.append(line);
    }
    if (s.statistics) {
        const char *const labels[] = {QT_TRANSLATE_NOOP("jp::crunch::ForecastSheet", "Alpha"), QT_TRANSLATE_NOOP("jp::crunch::ForecastSheet", "Beta"), QT_TRANSLATE_NOOP("jp::crunch::ForecastSheet", "Gamma"), QT_TRANSLATE_NOOP("jp::crunch::ForecastSheet", "MASE"), QT_TRANSLATE_NOOP("jp::crunch::ForecastSheet", "SMAPE"), QT_TRANSLATE_NOOP("jp::crunch::ForecastSheet", "MAE"), QT_TRANSLATE_NOOP("jp::crunch::ForecastSheet", "RMSE")};
        plan.statistics.append({QCoreApplication::translate("jp::crunch::ForecastSheet", "Forecast Statistics"), QString()});
        for (int k = 0; k < 7; ++k)
            plan.statistics.append({QCoreApplication::translate("jp::crunch::ForecastSheet", labels[k]), QStringLiteral("=FORECAST.ETS.STAT(") + values + QLatin1Char(',') + times + QLatin1Char(',') + QString::number(k + 1) + tail});
    }
    return plan;
}

double forecastTimeColumnWidth(const ForecastSheetPlan &plan, const QString &formatCode, jp::calc::DateSystem system)
{
    constexpr double kLeast = 12, kMost = 255;
    if (formatCode.isEmpty()) return kLeast;
    const jp::calc::NumberFormat format(formatCode);
    // (a long table is looked at from every few rows on: the longest text of dates is found in any year)
    const int rows = int(plan.table.size());
    const int stride = std::max(1, rows / 20000);
    int longest = 0;
    for (int i = 1; i < rows; i += stride) {
        if (plan.table[i].isEmpty()) continue;
        bool ok = false;
        const double t = plan.table[i][0].toDouble(&ok);
        if (!ok) continue;
        const jp::calc::Formatted f = format.format(t, system);
        longest = std::max(longest, int(f.failed ? 8 : f.text.size()));
    }
    return std::min(kMost, std::max(kLeast, std::ceil(longest * 1.1) + 2));
}

} // namespace jp::crunch
