// Conditional sums and counts, SUMPRODUCT, SUBTOTAL, and the statistics the workbooks use.
#include "calc/machine.h"

#include <algorithm>
#include <cmath>

namespace jp::calc {

namespace {

// ---- a criterion of SUMIF, COUNTIF, and the others: a number, a text with ? and * in it, TRUE or FALSE, an error, or any of these after =, <>, <, <=, >, or >=

struct Criterion {
    enum class Kind { Number, Text, Bool, Error, Blank, NotBlank, EmptyText } kind = Kind::Text;
    enum class Cmp { Eq, Ne, Lt, Le, Gt, Ge } cmp = Cmp::Eq;
    double number = 0;
    bool boolean = false;
    ErrorKind error = ErrorKind::NA;
    QString text;       // Text: the pattern; Number: the text it was written as (a text cell with the same characters matches)
    bool wildcards = false;
    bool matchesEmpty = false;   // an empty cell matches
};

bool parseCriterion(const Value &v, DateSystem system, Criterion *out)
{
    Criterion c;
    switch (v.kind()) {
    case Value::Kind::Number: c.kind = Criterion::Kind::Number; c.number = v.number(); c.text = numberToText(v.number()); *out = c; return true;
    case Value::Kind::Bool: c.kind = Criterion::Kind::Bool; c.boolean = v.boolean(); *out = c; return true;
    case Value::Kind::Error: c.kind = Criterion::Kind::Error; c.error = v.errorKind(); *out = c; return true;
    case Value::Kind::Empty: c.kind = Criterion::Kind::Number; c.number = 0; c.text = QStringLiteral("0"); *out = c; return true;
    case Value::Kind::Array: return false;
    case Value::Kind::Text: break;
    }
    QString s = v.text();
    int n = 0;
    if (s.startsWith(QLatin1String("<>"))) { c.cmp = Criterion::Cmp::Ne; n = 2; }
    else if (s.startsWith(QLatin1String("<="))) { c.cmp = Criterion::Cmp::Le; n = 2; }
    else if (s.startsWith(QLatin1String(">="))) { c.cmp = Criterion::Cmp::Ge; n = 2; }
    else if (s.startsWith(QLatin1Char('<'))) { c.cmp = Criterion::Cmp::Lt; n = 1; }
    else if (s.startsWith(QLatin1Char('>'))) { c.cmp = Criterion::Cmp::Gt; n = 1; }
    else if (s.startsWith(QLatin1Char('='))) { c.cmp = Criterion::Cmp::Eq; n = 1; }
    const bool hadOperator = n > 0;
    const QString rest = s.mid(n);
    if (rest.isEmpty()) {
        if (!hadOperator) {
            c.kind = Criterion::Kind::EmptyText;
            c.matchesEmpty = true;
        } else if (c.cmp == Criterion::Cmp::Eq) {
            c.kind = Criterion::Kind::Blank;
            c.matchesEmpty = true;
        } else if (c.cmp == Criterion::Cmp::Ne) {
            c.kind = Criterion::Kind::NotBlank;
        } else {
            c.kind = Criterion::Kind::Text;
        }
        *out = c;
        return true;
    }
    double x;
    bool errKnown = false;
    const ErrorKind e = errorKindFromText(rest, &errKnown);
    if (errKnown) {
        c.kind = Criterion::Kind::Error;
        c.error = e;
    } else if (rest.compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0 || rest.compare(QLatin1String("FALSE"), Qt::CaseInsensitive) == 0) {
        c.kind = Criterion::Kind::Bool;
        c.boolean = rest.size() == 4;
    } else if (textToNumber(rest, system, &x)) {
        c.kind = Criterion::Kind::Number;
        c.number = x;
        c.text = rest;
    } else {
        c.kind = Criterion::Kind::Text;
        c.text = rest;
        c.wildcards = hasWildcards(rest);
    }
    c.matchesEmpty = c.cmp == Criterion::Cmp::Ne;
    *out = c;
    return true;
}

bool criterionMatches(const Criterion &c, const Value &v, DateSystem system)
{
    if (v.isEmpty()) return c.matchesEmpty;
    switch (c.kind) {
    case Criterion::Kind::Blank: return false;
    case Criterion::Kind::EmptyText: return v.isText() && v.text().isEmpty();
    case Criterion::Kind::NotBlank: return true;
    case Criterion::Kind::Number: {
        bool eq = false;
        if (v.isNumber()) {
            const double y = v.number();
            switch (c.cmp) {
            case Criterion::Cmp::Eq: return sameTo15Digits(y, c.number);
            case Criterion::Cmp::Ne: return !sameTo15Digits(y, c.number);
            case Criterion::Cmp::Lt: return y < c.number && !sameTo15Digits(y, c.number);
            case Criterion::Cmp::Le: return y < c.number || sameTo15Digits(y, c.number);
            case Criterion::Cmp::Gt: return y > c.number && !sameTo15Digits(y, c.number);
            case Criterion::Cmp::Ge: return y > c.number || sameTo15Digits(y, c.number);
            }
        }
        if (v.isText() && (c.cmp == Criterion::Cmp::Eq || c.cmp == Criterion::Cmp::Ne)) {
            double y;
            eq = v.text().compare(c.text, Qt::CaseInsensitive) == 0 || (textToNumber(v.text(), system, &y) && sameTo15Digits(y, c.number));
            return c.cmp == Criterion::Cmp::Eq ? eq : !eq;
        }
        return c.cmp == Criterion::Cmp::Ne;
    }
    case Criterion::Kind::Bool: {
        const bool eq = v.isBool() && v.boolean() == c.boolean;
        if (c.cmp == Criterion::Cmp::Eq) return eq;
        if (c.cmp == Criterion::Cmp::Ne) return !eq;
        return false;
    }
    case Criterion::Kind::Error: {
        const bool eq = v.isError() && v.errorKind() == c.error;
        if (c.cmp == Criterion::Cmp::Eq) return eq;
        if (c.cmp == Criterion::Cmp::Ne) return !eq;
        return false;
    }
    case Criterion::Kind::Text: {
        if (!v.isText()) return c.cmp == Criterion::Cmp::Ne;
        const QString &t = v.text();
        int order;
        switch (c.cmp) {
        case Criterion::Cmp::Eq: return c.wildcards ? wildcardMatch(t, 0, c.text, 0) : compareText(t, c.text) == 0;
        case Criterion::Cmp::Ne: return !(c.wildcards ? wildcardMatch(t, 0, c.text, 0) : compareText(t, c.text) == 0);
        default: break;
        }
        order = compareText(t, c.text);
        switch (c.cmp) {
        case Criterion::Cmp::Lt: return order < 0;
        case Criterion::Cmp::Le: return order <= 0;
        case Criterion::Cmp::Gt: return order > 0;
        default: return order >= 0;
        }
    }
    }
    return false;
}

// The one area of argument i, for a function that takes a range of cells.
bool oneArea(Call &c, int i, RefArea *area)
{
    if (c.missing(i) || !c.item(i).isRef() || c.item(i).more || c.item(i).area.sheet1 != c.item(i).area.sheet2) {
        c.error(ErrorKind::Value);
        return false;
    }
    *area = c.item(i).area;
    return true;
}

Value fnSumIf(Call &c)
{
    RefArea range;
    if (!oneArea(c, 0, &range)) return c.failure();
    Criterion crit;
    if (!parseCriterion(c.scalar(1), c.context().dateSystem, &crit)) return c.error(ErrorKind::Value);
    RefArea sumArea = range;
    const bool hasSum = c.argc() > 2 && !c.missing(2);
    if (hasSum) {
        if (!oneArea(c, 2, &sumArea)) return c.failure();
        // the same shape as the range, from the first cell of the sum range
        sumArea.row2 = quint32(std::min<qint64>(qint64(sumArea.row1) + range.rows() - 1, kGridRows - 1));
        sumArea.col2 = quint32(std::min<qint64>(qint64(sumArea.col1) + range.cols() - 1, kGridCols - 1));
        c.noteRead(sumArea);   // (the cells read are these, not the sum range as it was written)
    }
    double sum = 0;
    Value error;
    Machine &m = c.machine();
    auto add = [&](const Value &target) {
        if (target.isNumber()) sum += target.number();
        else if (target.isError()) {
            error = target;
            return false;
        }
        return true;
    };
    if (hasSum && crit.matchesEmpty) {
        // An empty cell meets the criterion, and empty cells are not among those the range holds: the cells that are there are the sum range's, each with the cell of the
        // range beside it (empty or not) to meet the criterion.
        bool ok = true;
        m.source().visit(sumArea.sheet1, sumArea.row1, sumArea.col1, sumArea.row2, sumArea.col2, [&](quint32 row, quint32 col, const Value &target) {
            if (!m.charge(1)) {
                ok = false;
                return false;
            }
            if (target.isEmpty()) return true;
            const Value tested = m.cell(range.sheet1, range.row1 + (row - sumArea.row1), range.col1 + (col - sumArea.col1));
            if (!criterionMatches(crit, tested, c.context().dateSystem)) return true;
            if (!add(target)) {
                ok = false;
                return false;
            }
            return true;
        });
        if (!ok) return error.isError() ? error : c.error(ErrorKind::Value);
        return numberResult(sum);
    }
    const bool done = c.eachAt(0, [&](const Value &v, quint32 row, quint32 col) {
        if (!criterionMatches(crit, v, c.context().dateSystem)) return true;
        return add(hasSum ? m.cell(sumArea.sheet1, sumArea.row1 + (row - range.row1), sumArea.col1 + (col - range.col1)) : v);
    });
    if (!done) return error.isError() ? error : c.error(ErrorKind::Value);
    return numberResult(sum);
}

Value fnCountIf(Call &c)
{
    RefArea range;
    if (!oneArea(c, 0, &range)) return c.failure();
    Criterion crit;
    if (!parseCriterion(c.scalar(1), c.context().dateSystem, &crit)) return c.error(ErrorKind::Value);
    qint64 matched = 0, visited = 0;
    const bool done = c.eachAt(0, [&](const Value &v, quint32, quint32) {
        ++visited;
        if (criterionMatches(crit, v, c.context().dateSystem)) ++matched;
        return true;
    });
    if (!done) return c.error(ErrorKind::Value);
    if (crit.matchesEmpty) matched += range.cells() - visited;
    return Value::number(double(matched));
}

// ---- SUMPRODUCT

Value fnSumProduct(Call &c)
{
    std::vector<Value> arrays;
    for (int i = 0; i < c.argc(); ++i) {
        Value a;
        if (c.missing(i)) a = Value::number(0);
        else if (!c.array(i, &a)) return c.failure();
        // an argument that is one value, or one cell, and holds a text, is no array of numbers to count as zeros: #VALUE! (as the stored results of the files show)
        if (a.rows() == 1 && a.cols() == 1 && a.at(0, 0).isText() && !c.item(i).isMissing()) return c.error(ErrorKind::Value);
        if (!arrays.empty() && (a.rows() != arrays[0].rows() || a.cols() != arrays[0].cols())) return c.error(ErrorKind::Value);
        arrays.push_back(a);
    }
    const int rows = arrays[0].rows(), cols = arrays[0].cols();
    if (!c.machine().charge(qint64(rows) * cols * qint64(arrays.size()))) return c.error(ErrorKind::Value);
    double sum = 0;
    for (int r = 0; r < rows; ++r)
        for (int col = 0; col < cols; ++col) {
            double product = 1;
            for (const Value &a : arrays) {
                const Value &v = a.at(r, col);
                if (v.isError()) return v;
                product *= v.isNumber() ? v.number() : 0;
            }
            sum += product;
        }
    return numberResult(sum);
}

// ---- the statistics

// Mean and sum of squared deviations.
void moments(const std::vector<double> &x, double *mean, double *ss)
{
    double sum = 0;
    for (const double v : x) sum += v;
    *mean = sum / double(x.size());
    double acc = 0;
    for (const double v : x) acc += (v - *mean) * (v - *mean);
    *ss = acc;
}

Value spread(Call &c, bool sample, bool root)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    const size_t n = x.size();
    if (sample ? n < 2 : n < 1) return c.error(ErrorKind::Div0);
    double mean, ss;
    moments(x, &mean, &ss);
    const double var = ss / double(sample ? n - 1 : n);
    return numberResult(root ? std::sqrt(var) : var);
}
Value fnStdev(Call &c) { return spread(c, true, true); }
Value fnStdevP(Call &c) { return spread(c, false, true); }
Value fnVar(Call &c) { return spread(c, true, false); }
Value fnVarP(Call &c) { return spread(c, false, false); }

Value fnMedian(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    if (x.empty()) return c.error(ErrorKind::Num);
    std::sort(x.begin(), x.end());
    const size_t n = x.size();
    return numberResult(n % 2 ? x[n / 2] : (x[n / 2 - 1] + x[n / 2]) / 2);
}

Value kth(Call &c, bool largest)
{
    std::vector<double> x;
    Value error;
    double k;
    if (!c.number(1, &k)) return c.failure();
    // the numbers of the array or reference only
    bool ok = true;
    const bool done = c.each(0, [&](const Value &v, bool) {
        if (v.isNumber()) x.push_back(v.number());
        else if (v.isError()) {
            error = v;
            ok = false;
            return false;
        }
        return true;
    });
    if (!done) return ok ? c.error(ErrorKind::Value) : error;
    k = std::ceil(k);
    if (x.empty() || !(k >= 1) || k > double(x.size())) return c.error(ErrorKind::Num);
    std::sort(x.begin(), x.end());
    return Value::number(largest ? x[x.size() - size_t(k)] : x[size_t(k) - 1]);
}
Value fnLarge(Call &c) { return kth(c, true); }
Value fnSmall(Call &c) { return kth(c, false); }

Value fnRank(Call &c)
{
    double number, order;
    if (!c.number(0, &number) || !c.numberOr(2, 0, &order)) return c.failure();
    qint64 ahead = 0;
    bool found = false;
    Value error;
    bool ok = true;
    const bool done = c.each(1, [&](const Value &v, bool) {
        if (v.isError()) {
            error = v;
            ok = false;
            return false;
        }
        if (!v.isNumber()) return true;
        if (sameTo15Digits(v.number(), number)) found = true;
        else if (order == 0 ? v.number() > number : v.number() < number) ++ahead;
        return true;
    });
    if (!done) return ok ? c.error(ErrorKind::Value) : error;
    if (!found) return c.error(ErrorKind::NA);
    return Value::number(double(ahead + 1));
}

// SUBTOTAL(function, ref1, ...): 1 to 11 for a function of the numbers, leaving out the rows a filter hides; 101 to 111 leave out every hidden row.
Value fnSubtotal(Call &c)
{
    double f;
    if (!c.number(0, &f)) return c.failure();
    int fn = int(std::trunc(f));
    // (1 to 11 leave out the rows a filter hides; 101 to 111 leave out every hidden row)
    const int hiddenRows = fn >= 101 && fn <= 111 ? 1 : 2;
    if (fn >= 101 && fn <= 111) fn -= 100;
    if (fn < 1 || fn > 11) return c.error(ErrorKind::Value);
    if (fn == 3) {
        qint64 n = 0;
        for (int i = 1; i < c.argc(); ++i)
            if (!c.eachCell(i, [&](const Value &, int sheet, quint32 row, quint32 col) {
                    if (sheet >= 0 && c.machine().source().isSubtotal(sheet, row, col)) return true;
                    if (sheet >= 0 && c.machine().source().rowHiddenKind(sheet, row) >= hiddenRows) return true;
                    ++n;
                    return true;
                }))
                return c.error(ErrorKind::Value);
        return Value::number(double(n));
    }
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 1, &x, &error, true, hiddenRows)) return error;
    const size_t n = x.size();
    double sum = 0;
    for (const double v : x) sum += v;
    switch (fn) {
    case 1: return n ? numberResult(sum / double(n)) : c.error(ErrorKind::Div0);
    case 2: return Value::number(double(n));
    case 4: return Value::number(n ? *std::max_element(x.begin(), x.end()) : 0);
    case 5: return Value::number(n ? *std::min_element(x.begin(), x.end()) : 0);
    case 6: {
        double p = 1;
        for (const double v : x) p *= v;
        return numberResult(n ? p : 0);
    }
    case 7: case 8: case 10: case 11: {
        const bool sample = fn == 7 || fn == 10;
        if (sample ? n < 2 : n < 1) return c.error(ErrorKind::Div0);
        double mean, ss;
        moments(x, &mean, &ss);
        const double var = ss / double(sample ? n - 1 : n);
        return numberResult(fn == 7 || fn == 8 ? std::sqrt(var) : var);
    }
    default: return numberResult(sum);
    }
}

// ---- two series: correlation and the line through them

// The pairs of numbers of two series (both cells numbers; any other pair is left out). False, with the error, if they are not of one shape.
bool pairsOf(Call &c, int a, int b, std::vector<double> *xs, std::vector<double> *ys)
{
    Table ta, tb;
    if (!c.table(a, &ta, false)) return false;
    if (!c.table(b, &tb, false)) return false;
    if (ta.fullRows() != tb.fullRows() || ta.fullCols() != tb.fullCols()) {
        c.error(ErrorKind::NA);
        return false;
    }
    if (ta.fullRows() * ta.fullCols() > kMaxArrayCells || !c.machine().charge(ta.fullRows() * ta.fullCols())) {
        c.error(ErrorKind::Value);
        return false;
    }
    for (qint64 r = 0; r < ta.fullRows(); ++r)
        for (qint64 col = 0; col < ta.fullCols(); ++col) {
            const Value x = ta.at(r, col), y = tb.at(r, col);
            if (x.isError()) {
                c.error(x.errorKind());
                return false;
            }
            if (y.isError()) {
                c.error(y.errorKind());
                return false;
            }
            if (x.isNumber() && y.isNumber()) {
                xs->push_back(x.number());
                ys->push_back(y.number());
            }
        }
    return true;
}

struct Fit {
    double mx = 0, my = 0, sxx = 0, syy = 0, sxy = 0;
    size_t n = 0;
};

bool fitOf(Call &c, int first, int second, Fit *fit)
{
    std::vector<double> xs, ys;
    if (!pairsOf(c, first, second, &xs, &ys)) return false;
    fit->n = xs.size();
    if (fit->n < 1) {
        c.error(ErrorKind::Div0);
        return false;
    }
    double sx = 0, sy = 0;
    for (size_t i = 0; i < xs.size(); ++i) {
        sx += xs[i];
        sy += ys[i];
    }
    fit->mx = sx / double(fit->n);
    fit->my = sy / double(fit->n);
    for (size_t i = 0; i < xs.size(); ++i) {
        fit->sxx += (xs[i] - fit->mx) * (xs[i] - fit->mx);
        fit->syy += (ys[i] - fit->my) * (ys[i] - fit->my);
        fit->sxy += (xs[i] - fit->mx) * (ys[i] - fit->my);
    }
    return true;
}

Value fnCorrel(Call &c)
{
    Fit f;
    if (!fitOf(c, 0, 1, &f)) return c.failure();
    if (f.n < 2 || f.sxx == 0 || f.syy == 0) return c.error(ErrorKind::Div0);
    return numberResult(f.sxy / std::sqrt(f.sxx * f.syy));
}

Value fnRsq(Call &c)
{
    Fit f;
    if (!fitOf(c, 0, 1, &f)) return c.failure();
    if (f.n < 2 || f.sxx == 0 || f.syy == 0) return c.error(ErrorKind::Div0);
    return numberResult(f.sxy * f.sxy / (f.sxx * f.syy));
}

// SLOPE(known_y, known_x), INTERCEPT, and FORECAST(x, known_y, known_x)
Value fnSlope(Call &c)
{
    Fit f;
    if (!fitOf(c, 1, 0, &f)) return c.failure();   // (x is the second argument, y the first, so the pairs are made the other way round)
    if (f.n < 2 || f.sxx == 0) return c.error(ErrorKind::Div0);
    return numberResult(f.sxy / f.sxx);
}

Value fnIntercept(Call &c)
{
    Fit f;
    if (!fitOf(c, 1, 0, &f)) return c.failure();
    if (f.n < 2 || f.sxx == 0) return c.error(ErrorKind::Div0);
    return numberResult(f.my - f.sxy / f.sxx * f.mx);
}

Value fnForecast(Call &c)
{
    double x;
    if (!c.number(0, &x)) return c.failure();
    Fit f;
    if (!fitOf(c, 2, 1, &f)) return c.failure();
    if (f.n < 2 || f.sxx == 0) return c.error(ErrorKind::Div0);
    const double slope = f.sxy / f.sxx;
    return numberResult(f.my - slope * f.mx + slope * x);
}

Value fnCovar(Call &c)
{
    Fit f;
    if (!fitOf(c, 0, 1, &f)) return c.failure();
    return numberResult(f.sxy / double(f.n));
}

// ---- percentiles and the mode

bool numbersOf(Call &c, int arg, std::vector<double> *x, Value *error)
{
    bool ok = true;
    const bool done = c.each(arg, [&](const Value &v, bool) {
        if (v.isNumber()) x->push_back(v.number());
        else if (v.isError()) {
            *error = v;
            ok = false;
            return false;
        }
        return true;
    });
    if (!done) {
        if (ok) *error = Value::error(ErrorKind::Value);
        return false;
    }
    return true;
}

Value percentile(Call &c, double k)
{
    std::vector<double> x;
    Value error;
    if (!numbersOf(c, 0, &x, &error)) return error;
    if (x.empty() || !(k >= 0 && k <= 1)) return c.error(ErrorKind::Num);
    std::sort(x.begin(), x.end());
    const double pos = k * double(x.size() - 1);
    const size_t lo = size_t(std::floor(pos));
    const double frac = pos - double(lo);
    if (lo + 1 >= x.size()) return Value::number(x.back());
    return numberResult(x[lo] + (x[lo + 1] - x[lo]) * frac);
}

Value fnPercentile(Call &c)
{
    double k;
    if (!c.number(1, &k)) return c.failure();
    return percentile(c, k);
}

Value fnQuartile(Call &c)
{
    double q;
    if (!c.number(1, &q)) return c.failure();
    q = std::trunc(q);
    if (q < 0 || q > 4) return c.error(ErrorKind::Num);
    return percentile(c, q / 4);
}

Value fnMode(Call &c)
{
    std::vector<double> x;
    Value error;
    if (!gatherNumbers(c, 0, &x, &error)) return error;
    // the value that comes most often; of several that come as often, the first in the order the numbers came
    std::vector<double> sorted = x;
    std::sort(sorted.begin(), sorted.end());
    size_t bestCount = 1;
    for (size_t i = 0; i < sorted.size();) {
        size_t j = i;
        while (j < sorted.size() && sorted[j] == sorted[i]) ++j;
        bestCount = std::max(bestCount, j - i);
        i = j;
    }
    if (bestCount < 2) return c.error(ErrorKind::NA);
    for (const double v : x) {
        const auto range = std::equal_range(sorted.begin(), sorted.end(), v);
        if (size_t(range.second - range.first) == bestCount) return Value::number(v);
    }
    return c.error(ErrorKind::NA);
}

// ---- conditions on several ranges: SUMIFS, COUNTIFS, AVERAGEIF, AVERAGEIFS, MAXIFS, MINIFS

enum class Aggregate { Sum, Count, Average, Max, Min };

// Goes over the cells of the value range (or, for a count, of the first range) that meet every condition; `conds` is the number of pairs of range and criterion from
// argument `firstCond` on. The value range, if there is one, is argument 0; AVERAGEIF (`averageIf`) has its range first, then the criterion, then the value range.
Value aggregateIf(Call &c, Aggregate how, bool hasValues, int firstCond, bool averageIf = false)
{
    const DateSystem sys = c.context().dateSystem;
    struct Cond {
        RefArea area;
        Criterion crit;
    };
    std::vector<Cond> conds;
    RefArea values;
    if (averageIf) {
        Cond cond;
        if (!oneArea(c, 0, &cond.area)) return c.failure();
        if (!parseCriterion(c.scalar(1), sys, &cond.crit)) return c.error(ErrorKind::Value);
        conds.push_back(cond);
        values = cond.area;
        if (c.argc() > 2 && !c.missing(2)) {
            if (!oneArea(c, 2, &values)) return c.failure();
            values.row2 = quint32(std::min<qint64>(qint64(values.row1) + cond.area.rows() - 1, kGridRows - 1));
            values.col2 = quint32(std::min<qint64>(qint64(values.col1) + cond.area.cols() - 1, kGridCols - 1));
            c.noteRead(values);
        }
    } else {
        if (hasValues && !oneArea(c, 0, &values)) return c.failure();
        for (int i = firstCond; i < c.argc(); i += 2) {
            Cond cond;
            if (i + 1 >= c.argc()) return c.error(ErrorKind::Value);
            if (!oneArea(c, i, &cond.area)) return c.failure();
            if (!parseCriterion(c.scalar(i + 1), sys, &cond.crit)) return c.error(ErrorKind::Value);
            conds.push_back(cond);
        }
        if (conds.empty()) return c.error(ErrorKind::Value);
        if (!hasValues) values = conds[0].area;
        for (const Cond &cond : conds)
            if (cond.area.rows() != values.rows() || cond.area.cols() != values.cols()) return c.error(ErrorKind::Value);
    }
    Machine &m = c.machine();
    double sum = 0, best = 0;
    qint64 count = 0;
    Value error;
    auto meets = [&](quint32 dr, quint32 dc) {
        for (const Cond &cond : conds)
            if (!criterionMatches(cond.crit, m.cell(cond.area.sheet1, cond.area.row1 + dr, cond.area.col1 + dc), sys)) return false;
        return true;
    };
    auto take = [&](const Value &v) {
        if (how == Aggregate::Count) {
            ++count;
            return true;
        }
        if (v.isError()) {
            error = v;
            return false;
        }
        if (!v.isNumber()) return true;
        ++count;
        sum += v.number();
        if (count == 1 || (how == Aggregate::Max ? v.number() > best : v.number() < best)) best = v.number();
        return true;
    };
    if (how == Aggregate::Count) {
        // every cell of the shape may count (an empty one meets "=" and "<>x"): the cells of the first range that hold something, and the empty rest
        bool anyMatchesEmpty = false;
        for (const Cond &cond : conds) anyMatchesEmpty = anyMatchesEmpty || cond.crit.matchesEmpty;
        if (!anyMatchesEmpty) {
            const RefArea &first = conds[0].area;
            bool ok = true;
            for (int s = first.sheet1; s <= first.sheet2 && ok; ++s)
                m.source().visit(s, first.row1, first.col1, first.row2, first.col2, [&](quint32 row, quint32 col, const Value &) {
                    if (!m.charge(conds.size())) {
                        ok = false;
                        return false;
                    }
                    if (meets(row - first.row1, col - first.col1)) ++count;
                    return true;
                });
            if (!ok) return c.error(ErrorKind::Value);
        } else {
            const qint64 cells = values.rows() * values.cols();
            if (cells > kMaxArrayCells * 4 || !m.charge(cells * qint64(conds.size()))) return c.error(ErrorKind::Value);
            for (quint32 dr = 0; dr < values.rows(); ++dr)
                for (quint32 dc = 0; dc < values.cols(); ++dc)
                    if (meets(dr, dc)) ++count;
        }
        return Value::number(double(count));
    }
    bool ok = true;
    for (int s = values.sheet1; s <= values.sheet2 && ok; ++s)
        m.source().visit(s, values.row1, values.col1, values.row2, values.col2, [&](quint32 row, quint32 col, const Value &v) {
            if (!m.charge(conds.size() + 1)) {
                ok = false;
                return false;
            }
            if (v.isEmpty() || !meets(row - values.row1, col - values.col1)) return true;
            if (!take(v)) {
                ok = false;
                return false;
            }
            return true;
        });
    if (!ok) return error.isError() ? error : c.error(ErrorKind::Value);
    switch (how) {
    case Aggregate::Sum: return numberResult(sum);
    case Aggregate::Average: return count ? numberResult(sum / double(count)) : c.error(ErrorKind::Div0);
    default: return Value::number(count ? best : 0);
    }
}

Value fnSumIfs(Call &c) { return aggregateIf(c, Aggregate::Sum, true, 1); }
Value fnCountIfs(Call &c) { return aggregateIf(c, Aggregate::Count, false, 0); }
Value fnAverageIf(Call &c) { return aggregateIf(c, Aggregate::Average, true, 0, true); }
Value fnAverageIfs(Call &c) { return aggregateIf(c, Aggregate::Average, true, 1); }
Value fnMaxIfs(Call &c) { return aggregateIf(c, Aggregate::Max, true, 1); }
Value fnMinIfs(Call &c) { return aggregateIf(c, Aggregate::Min, true, 1); }

// ---- the database functions: DSUM, DAVERAGE, DCOUNT, ... over a table whose first row names its fields, for the records that meet a block of criteria

// What one cell of the criteria asks of a field. A text with no operator in it is the texts that begin with it (and may have ? and * in it); "=" and the other operators work
// as they do for COUNTIF. False if the cell asks nothing (it is empty, or an empty text).
bool databaseCriterion(const Value &v, DateSystem system, Criterion *out)
{
    if (v.isEmpty() || (v.isText() && v.text().isEmpty())) return false;
    if (v.isText()) {
        const QString &t = v.text();
        const QChar first = t[0];
        double number;
        if (first != QLatin1Char('=') && first != QLatin1Char('<') && first != QLatin1Char('>') && !textToNumber(t, system, &number)) {
            Criterion c;
            c.kind = Criterion::Kind::Text;
            c.cmp = Criterion::Cmp::Eq;
            c.text = t + QLatin1Char('*');
            c.wildcards = true;
            *out = c;
            return true;
        }
    }
    return parseCriterion(v, system, out);
}

enum class Db { Sum, Average, Count, CountA, Max, Min, Get, Product, Stdev, StdevP, Var, VarP };

Value databaseFunction(Call &c, Db what)
{
    Machine &m = c.machine();
    const DateSystem sys = c.context().dateSystem;
    Table db, crit;
    // (the table and the criteria are blocks of cells: one value is neither)
    const auto isBlock = [&](int i) { return c.item(i).isRef() || (c.item(i).kind == Item::Kind::Value && c.item(i).value.isArray()); };
    for (const int i : {0, 2})
        if (!c.missing(i) && c.item(i).kind == Item::Kind::Value && c.item(i).value.isError()) return c.item(i).value;   // (an error given for either is the result)
    if (!isBlock(0) || !isBlock(2) || !c.table(0, &db, true) || !c.table(2, &crit, true)) return c.error(ErrorKind::Value);
    const Value fieldArg = c.scalar(1);
    if (fieldArg.isError()) return fieldArg;
    if (db.cols() < 1) return c.error(ErrorKind::Value);
    // the names of the fields, in the first row without regard to case, each with the first column that has it: found for each name asked for in one look at the row, not by
    // going over it for each (the table's columns and the criteria's are counted as steps)
    if (!m.charge(db.cols() + crit.cols() + crit.rows() * crit.cols())) return c.error(ErrorKind::Value);
    QHash<QString, qint64> columnOf;
    columnOf.reserve(int(std::min<qint64>(db.cols(), 1 << 16)));
    for (qint64 j = 0; j < db.cols(); ++j) {
        const Value h = db.at(0, j);
        if (!h.isText()) continue;
        const QString key = h.text().toCaseFolded();
        if (!columnOf.contains(key)) columnOf.insert(key, j);   // (the first column with a name is the field)
    }
    // the field: its name in the first row, or its number from 1
    qint64 field = -1;
    if (fieldArg.isText()) {
        field = columnOf.value(fieldArg.text().toCaseFolded(), -1);
    } else if (fieldArg.isNumber()) {
        const double k = std::trunc(fieldArg.number());
        if (k >= 1 && k <= double(db.cols())) field = qint64(k) - 1;
    }
    if (field < 0) return c.error(ErrorKind::Value);
    // what each column of the criteria asks, and of which field of the table
    std::vector<qint64> fieldOf(size_t(std::max<qint64>(0, crit.cols())), -1);
    for (qint64 k = 0; k < crit.cols(); ++k) {
        const Value label = crit.at(0, k);
        if (label.isEmpty()) continue;
        const QString name = label.isText() ? label.text() : numberToText(label.isNumber() ? label.number() : 0);
        fieldOf[size_t(k)] = columnOf.value(name.toCaseFolded(), -1);
    }
    struct Condition {
        qint64 field;
        Criterion criterion;
    };
    std::vector<std::vector<Condition>> rows;   // the rows of the criteria, each of the conditions that must all be met
    for (qint64 r = 1; r < crit.rows(); ++r) {
        std::vector<Condition> row;
        for (qint64 k = 0; k < crit.cols(); ++k) {
            Criterion cr;
            if (!databaseCriterion(crit.at(r, k), sys, &cr)) continue;
            if (fieldOf[size_t(k)] < 0) {
                m.unsupported |= 4;   // (a condition under a name that is no field is a formula for each record: it is not worked out)
                return c.error(ErrorKind::Value);
            }
            row.push_back({fieldOf[size_t(k)], cr});
        }
        rows.push_back(std::move(row));
    }
    bool matchesAll = rows.empty();
    qint64 conditions = 0;
    for (const auto &row : rows) {
        matchesAll = matchesAll || row.empty();
        conditions += qint64(row.size());
    }
    qint64 matched = 0, numbers = 0, nonEmpty = 0;
    double sum = 0, product = 1, best = 0;
    std::vector<double> values;   // (for the spread: worked out as STDEV works it out)
    Value only, error;
    for (qint64 i = 1; i < db.rows(); ++i) {
        if (!m.charge(1 + conditions)) return c.error(ErrorKind::Value);   // (each record is tried against the conditions of each row of criteria)
        bool met = matchesAll;
        for (size_t r = 0; r < rows.size() && !met; ++r) {
            bool all = true;
            for (const Condition &cond : rows[r])
                if (!criterionMatches(cond.criterion, db.at(i, cond.field), sys)) {
                    all = false;
                    break;
                }
            met = all;
        }
        if (!met) continue;
        ++matched;
        const Value v = db.at(i, field);
        if (!v.isEmpty()) ++nonEmpty;
        if (what == Db::Get) only = v;
        if (v.isError() && what != Db::Count && what != Db::CountA && what != Db::Get) {
            if (!error.isError()) error = v;
            continue;
        }
        if (!v.isNumber()) continue;
        const double x = v.number();
        ++numbers;
        sum += x;
        product *= x;
        if (numbers == 1 || (what == Db::Max ? x > best : x < best)) best = x;
        if (what == Db::Stdev || what == Db::StdevP || what == Db::Var || what == Db::VarP) values.push_back(x);
    }
    double m2 = 0, mean = 0;
    if (!values.empty()) moments(values, &mean, &m2);
    if (error.isError()) return error;
    switch (what) {
    case Db::Sum: return numberResult(sum);
    case Db::Average: return numbers ? numberResult(sum / double(numbers)) : c.error(ErrorKind::Div0);
    case Db::Count: return Value::number(double(numbers));
    case Db::CountA: return Value::number(double(nonEmpty));
    case Db::Max: case Db::Min: return Value::number(numbers ? best : 0);
    case Db::Product: return numberResult(numbers ? product : 0);
    case Db::Get:
        if (matched == 0) return c.error(ErrorKind::Value);
        if (matched > 1) return c.error(ErrorKind::Num);
        return only.isEmpty() ? Value::number(0) : only;
    case Db::Stdev: case Db::Var: return numbers > 1 ? numberResult(what == Db::Var ? m2 / double(numbers - 1) : std::sqrt(m2 / double(numbers - 1))) : c.error(ErrorKind::Div0);
    case Db::StdevP: case Db::VarP: return numbers > 0 ? numberResult(what == Db::VarP ? m2 / double(numbers) : std::sqrt(m2 / double(numbers))) : c.error(ErrorKind::Div0);
    }
    return c.error(ErrorKind::Value);
}
Value fnDSum(Call &c) { return databaseFunction(c, Db::Sum); }
Value fnDAverage(Call &c) { return databaseFunction(c, Db::Average); }
Value fnDCount(Call &c) { return databaseFunction(c, Db::Count); }
Value fnDCountA(Call &c) { return databaseFunction(c, Db::CountA); }
Value fnDMax(Call &c) { return databaseFunction(c, Db::Max); }
Value fnDMin(Call &c) { return databaseFunction(c, Db::Min); }
Value fnDGet(Call &c) { return databaseFunction(c, Db::Get); }
Value fnDProduct(Call &c) { return databaseFunction(c, Db::Product); }
Value fnDStdev(Call &c) { return databaseFunction(c, Db::Stdev); }
Value fnDStdevP(Call &c) { return databaseFunction(c, Db::StdevP); }
Value fnDVar(Call &c) { return databaseFunction(c, Db::Var); }
Value fnDVarP(Call &c) { return databaseFunction(c, Db::VarP); }

// ---- the normal distribution

double normalCdf(double x) { return 0.5 * std::erfc(-x / std::sqrt(2.0)); }

// The inverse of the standard normal distribution: an approximation, then a step of Halley's method.
bool normalInverse(double p, double *out)
{
    if (!(p > 0 && p < 1)) return false;
    static const double a[] = {-3.969683028665376e+01, 2.209460984245205e+02, -2.759285104469687e+02, 1.383577518672690e+02, -3.066479806614716e+01, 2.506628277459239e+00};
    static const double b[] = {-5.447609879822406e+01, 1.615858368580409e+02, -1.556989798598866e+02, 6.680131188771972e+01, -1.328068155288572e+01};
    static const double cc[] = {-7.784894002430293e-03, -3.223964580411365e-01, -2.400758277161838e+00, -2.549732539343734e+00, 4.374664141464968e+00, 2.938163982698783e+00};
    static const double d[] = {7.784695709041462e-03, 3.224671290700398e-01, 2.445134137142996e+00, 3.754408661907416e+00};
    double x;
    if (p < 0.02425) {
        const double q = std::sqrt(-2 * std::log(p));
        x = (((((cc[0] * q + cc[1]) * q + cc[2]) * q + cc[3]) * q + cc[4]) * q + cc[5]) / ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1);
    } else if (p > 1 - 0.02425) {
        const double q = std::sqrt(-2 * std::log(1 - p));
        x = -(((((cc[0] * q + cc[1]) * q + cc[2]) * q + cc[3]) * q + cc[4]) * q + cc[5]) / ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1);
    } else {
        const double q = p - 0.5, r = q * q;
        x = (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q / (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1);
    }
    for (int i = 0; i < 2; ++i) {
        const double e = normalCdf(x) - p;
        const double u = e * std::sqrt(2 * 3.14159265358979323846) * std::exp(x * x / 2);
        x = x - u / (1 + x * u / 2);
    }
    *out = x;
    return true;
}

Value fnNormSDist(Call &c)
{
    double z;
    if (!c.number(0, &z)) return c.failure();
    return Value::number(normalCdf(z));
}

Value fnNormSInv(Call &c)
{
    double p, x;
    if (!c.number(0, &p)) return c.failure();
    if (!normalInverse(p, &x)) return c.error(ErrorKind::Num);
    return Value::number(x);
}

Value fnNormDist(Call &c)
{
    double x, mean, sd;
    bool cumulative;
    if (!c.number(0, &x) || !c.number(1, &mean) || !c.number(2, &sd) || !c.boolean(3, &cumulative)) return c.failure();
    if (sd <= 0) return c.error(ErrorKind::Num);
    const double z = (x - mean) / sd;
    if (cumulative) return Value::number(normalCdf(z));
    return numberResult(std::exp(-z * z / 2) / (sd * std::sqrt(2 * 3.14159265358979323846)));
}

Value fnNormInv(Call &c)
{
    double p, mean, sd, z;
    if (!c.number(0, &p) || !c.number(1, &mean) || !c.number(2, &sd)) return c.failure();
    if (sd <= 0 || !normalInverse(p, &z)) return c.error(ErrorKind::Num);
    return numberResult(mean + sd * z);
}

} // namespace

// The numbers of arguments from `first` on: in a reference or array the numbers; typed, a text read as a number, a boolean as 1 or 0. An error stops it. With
// `skipSubtotals` the cells of references that hold a SUBTOTAL are left out.
bool gatherNumbers(Call &c, int first, std::vector<double> *out, Value *error, bool skipSubtotals, int hiddenRows)
{
    for (int i = first; i < c.argc(); ++i) {
        bool ok = true;
        const bool typedArgument = !c.missing(i) && c.item(i).kind == Item::Kind::Value && !c.item(i).value.isArray();
        const bool done = c.eachCell(i, [&](const Value &v, int sheet, quint32 row, quint32 col) {
            const bool typed = typedArgument;
            if (skipSubtotals && sheet >= 0 && c.machine().source().isSubtotal(sheet, row, col)) return true;
            if (hiddenRows > 0 && sheet >= 0 && c.machine().source().rowHiddenKind(sheet, row) >= hiddenRows) return true;
            double x;
            switch (v.kind()) {
            case Value::Kind::Number: out->push_back(v.number()); break;
            case Value::Kind::Bool:
                if (typed) out->push_back(v.number());
                break;
            case Value::Kind::Text:
                if (typed) {
                    if (!textToNumber(v.text(), c.context().dateSystem, &x)) {
                        *error = Value::error(ErrorKind::Value);
                        ok = false;
                        return false;
                    }
                    out->push_back(x);
                }
                break;
            case Value::Kind::Error:
                *error = v;
                ok = false;
                return false;
            default: break;
            }
            return true;
        });
        if (!done) {
            if (ok) *error = Value::error(ErrorKind::Value);
            return false;
        }
    }
    return true;
}

void registerStatFunctions(std::vector<FunctionInfo> &r)
{
    r.push_back({"SUMIF", 2, 3, "xvx", 0, fnSumIf});
    r.push_back({"COUNTIF", 2, 2, "xv", 0, fnCountIf});
    r.push_back({"SUMPRODUCT", 1, 255, "a", 0, fnSumProduct});
    r.push_back({"SUBTOTAL", 2, 255, "vr", 0, fnSubtotal});
    r.push_back({"MEDIAN", 1, 255, "r", 0, fnMedian});
    r.push_back({"STDEV", 1, 255, "r", 0, fnStdev});
    r.push_back({"STDEV.S", 1, 255, "r", kFnFuture, fnStdev});
    r.push_back({"STDEVP", 1, 255, "r", 0, fnStdevP});
    r.push_back({"STDEV.P", 1, 255, "r", kFnFuture, fnStdevP});
    r.push_back({"VAR", 1, 255, "r", 0, fnVar});
    r.push_back({"VAR.S", 1, 255, "r", kFnFuture, fnVar});
    r.push_back({"VARP", 1, 255, "r", 0, fnVarP});
    r.push_back({"VAR.P", 1, 255, "r", kFnFuture, fnVarP});
    r.push_back({"LARGE", 2, 2, "rv", 0, fnLarge});
    r.push_back({"SMALL", 2, 2, "rv", 0, fnSmall});
    r.push_back({"RANK", 2, 3, "vrv", 0, fnRank});
    r.push_back({"SUMIFS", 3, 255, "n", 0, fnSumIfs});
    r.push_back({"COUNTIFS", 2, 254, "n", 0, fnCountIfs});
    r.push_back({"AVERAGEIF", 2, 3, "n", 0, fnAverageIf});
    r.push_back({"AVERAGEIFS", 3, 255, "n", 0, fnAverageIfs});
    r.push_back({"MAXIFS", 3, 255, "n", kFnFuture, fnMaxIfs});
    r.push_back({"MINIFS", 3, 255, "n", kFnFuture, fnMinIfs});
    r.push_back({"DSUM", 3, 3, "rvr", 0, fnDSum});
    r.push_back({"DAVERAGE", 3, 3, "rvr", 0, fnDAverage});
    r.push_back({"DCOUNT", 3, 3, "rvr", 0, fnDCount});
    r.push_back({"DCOUNTA", 3, 3, "rvr", 0, fnDCountA});
    r.push_back({"DMAX", 3, 3, "rvr", 0, fnDMax});
    r.push_back({"DMIN", 3, 3, "rvr", 0, fnDMin});
    r.push_back({"DGET", 3, 3, "rvr", 0, fnDGet});
    r.push_back({"DPRODUCT", 3, 3, "rvr", 0, fnDProduct});
    r.push_back({"DSTDEV", 3, 3, "rvr", 0, fnDStdev});
    r.push_back({"DSTDEVP", 3, 3, "rvr", 0, fnDStdevP});
    r.push_back({"DVAR", 3, 3, "rvr", 0, fnDVar});
    r.push_back({"DVARP", 3, 3, "rvr", 0, fnDVarP});
    r.push_back({"CORREL", 2, 2, "rr", 0, fnCorrel});
    r.push_back({"PEARSON", 2, 2, "rr", 0, fnCorrel});
    r.push_back({"RSQ", 2, 2, "rr", 0, fnRsq});
    r.push_back({"SLOPE", 2, 2, "rr", 0, fnSlope});
    r.push_back({"INTERCEPT", 2, 2, "rr", 0, fnIntercept});
    r.push_back({"FORECAST", 3, 3, "vrr", 0, fnForecast});
    r.push_back({"COVAR", 2, 2, "rr", 0, fnCovar});
    r.push_back({"PERCENTILE", 2, 2, "rv", 0, fnPercentile});
    r.push_back({"QUARTILE", 2, 2, "rv", 0, fnQuartile});
    r.push_back({"MODE", 1, 255, "r", 0, fnMode});
    r.push_back({"NORMSDIST", 1, 1, "v", 0, fnNormSDist});
    r.push_back({"NORMSINV", 1, 1, "v", 0, fnNormSInv});
    r.push_back({"NORMDIST", 4, 4, "vvvv", 0, fnNormDist});
    r.push_back({"NORMINV", 3, 3, "vvv", 0, fnNormInv});
}

} // namespace jp::calc
