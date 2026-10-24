#include "grid/condformat.h"

#include "grid/gridsheet.h"

#include "sheet/cellref.h"

#include <QDate>
#include <QHash>

#include <algorithm>
#include <cmath>

namespace jp::grid {

using jp::calc::Value;
using jp::office::sheet::Cell;
using jp::office::sheet::CellRange;
using jp::office::sheet::CondRule;
using jp::office::sheet::CondValue;
using jp::office::sheet::DifferentialFormat;

int iconSetCount(const QString &name)
{
    static const QHash<QString, int> counts = {
        {QStringLiteral("3Arrows"), 3}, {QStringLiteral("3ArrowsGray"), 3}, {QStringLiteral("3Flags"), 3}, {QStringLiteral("3TrafficLights1"), 3}, {QStringLiteral("3TrafficLights2"), 3},
        {QStringLiteral("3Signs"), 3}, {QStringLiteral("3Symbols"), 3}, {QStringLiteral("3Symbols2"), 3}, {QStringLiteral("3Stars"), 3}, {QStringLiteral("3Triangles"), 3},
        {QStringLiteral("4Arrows"), 4}, {QStringLiteral("4ArrowsGray"), 4}, {QStringLiteral("4RedToBlack"), 4}, {QStringLiteral("4Rating"), 4}, {QStringLiteral("4TrafficLights"), 4},
        {QStringLiteral("5Arrows"), 5}, {QStringLiteral("5ArrowsGray"), 5}, {QStringLiteral("5Rating"), 5}, {QStringLiteral("5Quarters"), 5}, {QStringLiteral("5Boxes"), 5}};
    return counts.value(name, 0);
}

namespace {

constexpr qint64 kMaxCellsPerRule = 4000000;   // the cells one pass over a rule's ranges looks at, at the most
constexpr size_t kMaxCached = 200000;          // results kept of the cells that were drawn

QRgb opaque(quint32 rgb) { return 0xFF000000u | (rgb & 0xFFFFFF); }

// The cells a rule names, as text for a formula: the first cell of its first range.
QString firstCellName(const CondRule &r)
{
    if (r.ranges.isEmpty()) return QStringLiteral("A1");
    return jp::office::sheet::cellName(r.ranges.first().row1, r.ranges.first().col1);
}

QString quoted(const QString &text)
{
    QString t = text;
    t.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + t + QLatin1Char('"');
}

// The formula that says whether the rule's test holds for the first cell (the file has it for most kinds; the others are made as the original does).
QString testFormula(const CondRule &r)
{
    using T = CondRule::Type;
    using O = CondRule::Operator;
    const QString c = firstCellName(r);
    const QString f1 = QLatin1Char('(') + r.formula1 + QLatin1Char(')'), f2 = QLatin1Char('(') + r.formula2 + QLatin1Char(')');
    switch (r.type) {
    case T::Expression: return r.formula1;
    case T::CellIs:
        switch (r.op) {
        case O::LessThan: return c + QStringLiteral("<") + f1;
        case O::LessThanOrEqual: return c + QStringLiteral("<=") + f1;
        case O::Equal: return c + QStringLiteral("=") + f1;
        case O::NotEqual: return c + QStringLiteral("<>") + f1;
        case O::GreaterThanOrEqual: return c + QStringLiteral(">=") + f1;
        case O::GreaterThan: return c + QStringLiteral(">") + f1;
        case O::Between: return QStringLiteral("AND(%1>=%2,%1<=%3)").arg(c, f1, f2);
        case O::NotBetween: return QStringLiteral("OR(%1<%2,%1>%3)").arg(c, f1, f2);
        }
        return QString();
    case T::ContainsText: return !r.formula1.isEmpty() ? r.formula1 : QStringLiteral("NOT(ISERROR(SEARCH(%1,%2)))").arg(quoted(r.text), c);
    case T::NotContainsText: return !r.formula1.isEmpty() ? r.formula1 : QStringLiteral("ISERROR(SEARCH(%1,%2))").arg(quoted(r.text), c);
    case T::BeginsWith: return !r.formula1.isEmpty() ? r.formula1 : QStringLiteral("LEFT(%1,LEN(%2))=%2").arg(c, quoted(r.text));
    case T::EndsWith: return !r.formula1.isEmpty() ? r.formula1 : QStringLiteral("RIGHT(%1,LEN(%2))=%2").arg(c, quoted(r.text));
    case T::ContainsBlanks: return !r.formula1.isEmpty() ? r.formula1 : QStringLiteral("LEN(TRIM(%1))=0").arg(c);
    case T::NotContainsBlanks: return !r.formula1.isEmpty() ? r.formula1 : QStringLiteral("LEN(TRIM(%1))>0").arg(c);
    case T::ContainsErrors: return !r.formula1.isEmpty() ? r.formula1 : QStringLiteral("ISERROR(%1)").arg(c);
    case T::NotContainsErrors: return !r.formula1.isEmpty() ? r.formula1 : QStringLiteral("NOT(ISERROR(%1))").arg(c);
    case T::TimePeriod: {
        if (!r.formula1.isEmpty()) return r.formula1;
        const QString p = r.timePeriod;
        if (p == QLatin1String("today")) return QStringLiteral("FLOOR(%1,1)=TODAY()").arg(c);
        if (p == QLatin1String("yesterday")) return QStringLiteral("FLOOR(%1,1)=TODAY()-1").arg(c);
        if (p == QLatin1String("tomorrow")) return QStringLiteral("FLOOR(%1,1)=TODAY()+1").arg(c);
        if (p == QLatin1String("last7Days")) return QStringLiteral("AND(TODAY()-FLOOR(%1,1)<=6,FLOOR(%1,1)<=TODAY())").arg(c);
        if (p == QLatin1String("thisWeek")) return QStringLiteral("AND(TODAY()-ROUNDDOWN(%1,0)<=WEEKDAY(TODAY())-1,ROUNDDOWN(%1,0)-TODAY()<=7-WEEKDAY(TODAY()))").arg(c);
        if (p == QLatin1String("lastWeek")) return QStringLiteral("AND(TODAY()-ROUNDDOWN(%1,0)>=(WEEKDAY(TODAY())),TODAY()-ROUNDDOWN(%1,0)<(WEEKDAY(TODAY())+7))").arg(c);
        if (p == QLatin1String("nextWeek")) return QStringLiteral("AND(ROUNDDOWN(%1,0)-TODAY()>(7-WEEKDAY(TODAY())),ROUNDDOWN(%1,0)-TODAY()<(15-WEEKDAY(TODAY())))").arg(c);
        if (p == QLatin1String("thisMonth")) return QStringLiteral("AND(MONTH(%1)=MONTH(TODAY()),YEAR(%1)=YEAR(TODAY()))").arg(c);
        if (p == QLatin1String("lastMonth")) return QStringLiteral("AND(MONTH(%1)=MONTH(EDATE(TODAY(),-1)),YEAR(%1)=YEAR(EDATE(TODAY(),-1)))").arg(c);
        if (p == QLatin1String("nextMonth")) return QStringLiteral("AND(MONTH(%1)=MONTH(EDATE(TODAY(),1)),YEAR(%1)=YEAR(EDATE(TODAY(),1)))").arg(c);
        return QString();
    }
    default: return QString();
    }
}

bool isStatisticsRule(CondRule::Type t)
{
    using T = CondRule::Type;
    return t == T::ColorScale || t == T::DataBar || t == T::IconSet || t == T::Top10 || t == T::UniqueValues || t == T::DuplicateValues || t == T::AboveAverage;
}

// The value as the figures of a rule count it (a number), or false.
bool numberOf(const Value &v, double *out)
{
    if (!v.isNumber()) return false;
    *out = v.number();
    return std::isfinite(*out);
}

QString keyOf(const Value &v)
{
    if (v.isNumber()) return QLatin1String("n") + jp::calc::numberToText(v.number());
    if (v.isText()) return QLatin1String("t") + v.text().toLower();
    if (v.isBool()) return v.boolean() ? QStringLiteral("bT") : QStringLiteral("bF");
    if (v.isError()) return QLatin1String("e") + jp::calc::errorKindText(v.errorKind());
    return QString();
}

double percentile(const std::vector<double> &sorted, double p)
{
    if (sorted.empty()) return 0;
    const double at = std::max(0.0, std::min(1.0, p)) * double(sorted.size() - 1);
    const size_t lo = size_t(std::floor(at)), hi = size_t(std::ceil(at));
    return sorted[lo] + (sorted[hi] - sorted[lo]) * (at - double(lo));
}

QRgb mix(QRgb a, QRgb b, double t)
{
    auto channel = [&](int shift) { return int(std::lround(double((a >> shift) & 0xFF) + (double((b >> shift) & 0xFF) - double((a >> shift) & 0xFF)) * t)); };
    return 0xFF000000u | (quint32(channel(16)) << 16) | (quint32(channel(8)) << 8) | quint32(channel(0));
}

} // namespace

// What a rule works out once and keeps.
struct CondFormats::RuleState {
    bool compiled = false, statsReady = false;
    std::shared_ptr<const jp::calc::Program> test;   // the test of a rule that has one, written for the first cell of its first range
    std::vector<std::shared_ptr<const jp::calc::Program>> valuePrograms;   // the formulas of the values of a scale, bar, or icon set
    quint32 firstRow = 0, firstCol = 0;
    // statistics
    bool haveNumbers = false;
    double lo = 0, hi = 0, average = 0, deviation = 0;
    double threshold = 0, upper = 0;           // top and bottom: the value a cell has to reach; average: the two limits
    std::vector<double> points;                // a scale: the values of its stops; an icon set: the values the icons change at; a bar: the lowest and highest
    std::unordered_map<QString, int> counts;   // the times a value is there (duplicates and unique values)
};

namespace {

// What the rules' formulas read: the sheet shown, and the others, by the values the grid has.
class GridCells : public jp::calc::CellSource {
public:
    explicit GridCells(const GridSheet *g) : m_g(g) {}
    int sheetCount() const override { return std::max<int>(1, int(m_g->data().sheetNames.size())); }
    int sheetIndex(const QString &name) const override
    {
        const QStringList &names = m_g->data().sheetNames;
        for (int i = 0; i < names.size(); ++i)
            if (names[i].compare(name, Qt::CaseInsensitive) == 0) return i;
        return -1;
    }
    Value cell(int sheet, quint32 row, quint32 col) const override { return m_g->valueIn(sheet, row, col); }
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const override
    {
        const jp::office::sheet::Sheet *s = m_g->sheetAt(sheet);
        if (!s) return;
        s->cells.forEachInRange(row1, col1, row2, col2, [&](const Cell &c) { return f(c.row, c.col, m_g->valueIn(sheet, c.row, c.col)); });
    }
    void usedExtent(int sheet, quint32 *rows, quint32 *cols) const override
    {
        const jp::office::sheet::Sheet *s = m_g->sheetAt(sheet);
        *rows = s ? s->extentRows : 0;
        *cols = s ? s->extentCols : 0;
    }

private:
    const GridSheet *m_g;
};

} // namespace

CondFormats::CondFormats(const GridSheet *owner) : m_owner(owner)
{
    m_today = std::floor(jp::calc::clockSerial(jp::calc::DateSystem::Date1900));
}

CondFormats::~CondFormats() = default;

void CondFormats::reset()
{
    if (!m_todayFixed) m_today = std::floor(jp::calc::clockSerial(jp::calc::DateSystem::Date1900));
    m_states.clear();
    m_cache.clear();
    m_formats.clear();
}

bool CondFormats::any() const
{
    const jp::office::sheet::Sheet *s = m_owner->data().sheet;
    return s && !s->conditionalFormats.isEmpty();
}

void CondFormats::valuesChanged()
{
    m_cache.clear();
    for (auto &st : m_states)
        if (st) st->statsReady = false;
}

void CondFormats::valueChanged(quint32 row, quint32 col)
{
    const jp::office::sheet::Sheet *s = m_owner->data().sheet;
    if (!s) return;
    m_cache.clear();
    for (size_t i = 0; i < m_states.size() && i < size_t(s->conditionalFormats.size()); ++i) {
        if (!m_states[i] || !m_states[i]->statsReady) continue;
        for (const CellRange &r : s->conditionalFormats[int(i)].ranges)
            if (r.contains(row, col)) {
                m_states[i]->statsReady = false;
                break;
            }
    }
}

bool CondFormats::covers(quint32 row, quint32 col) const
{
    const jp::office::sheet::Sheet *s = m_owner->data().sheet;
    if (!s) return false;
    for (const CondRule &r : s->conditionalFormats)
        for (const CellRange &range : r.ranges)
            if (range.contains(row, col)) return true;
    return false;
}

CondFormats::RuleState &CondFormats::state(size_t rule) const
{
    if (m_states.size() <= rule) m_states.resize(rule + 1);
    if (!m_states[rule]) m_states[rule] = std::make_unique<RuleState>();
    return *m_states[rule];
}

QString CondFormats::iconSetName(int rule) const
{
    const jp::office::sheet::Sheet *s = m_owner->data().sheet;
    return s && rule >= 0 && rule < s->conditionalFormats.size() ? s->conditionalFormats[rule].iconSet : QString();
}

const DifferentialFormat &CondFormats::format(int index) const
{
    static const DifferentialFormat none;
    return index >= 0 && size_t(index) < m_formats.size() ? m_formats[size_t(index)] : none;
}

int CondFormats::formatIndexOf(const DifferentialFormat &merged) const
{
    for (size_t i = 0; i < m_formats.size(); ++i) {
        const DifferentialFormat &f = m_formats[i];
        if (f.bold == merged.bold && f.italic == merged.italic && f.strike == merged.strike && f.underline == merged.underline && f.fontColor == merged.fontColor && f.hasFill == merged.hasFill
            && (!f.hasFill || f.fill == merged.fill) && f.hasBorder == merged.hasBorder && (!f.hasBorder || f.border == merged.border) && f.numberFormat == merged.numberFormat)
            return int(i);
    }
    m_formats.push_back(merged);
    return int(m_formats.size() - 1);
}

// The figures of a rule over all its cells, and the programs of its formulas.
void CondFormats::prepare(size_t rule) const
{
    RuleState &st = state(rule);
    const jp::office::sheet::Sheet &sheet = *m_owner->data().sheet;
    const CondRule &r = sheet.conditionalFormats[int(rule)];
    if (!st.compiled) {
        st.compiled = true;
        st.firstRow = r.ranges.isEmpty() ? 0 : r.ranges.first().row1;
        st.firstCol = r.ranges.isEmpty() ? 0 : r.ranges.first().col1;
        const QString text = testFormula(r);
        if (!text.trimmed().isEmpty()) st.test = jp::calc::parseFormula(text).program;
        for (const CondValue &v : r.values) {
            std::shared_ptr<const jp::calc::Program> p;
            if (v.type == CondValue::Type::Formula && !v.value.trimmed().isEmpty()) p = jp::calc::parseFormula(v.value).program;
            st.valuePrograms.push_back(p);
        }
    }
    if (st.statsReady || !isStatisticsRule(r.type)) return;
    st.statsReady = true;
    st.haveNumbers = false;
    st.counts.clear();
    st.points.clear();
    std::vector<double> numbers;
    ++m_passes;
    qint64 seen = 0;
    const bool counting = r.type == CondRule::Type::UniqueValues || r.type == CondRule::Type::DuplicateValues;
    for (const CellRange &range : r.ranges) {
        if (seen > kMaxCellsPerRule) break;
        sheet.cells.forEachInRange(range.row1, range.col1, range.row2, range.col2, [&](const Cell &c) {
            if (++seen > kMaxCellsPerRule) return false;
            const Value v = m_owner->valueOf(c);
            if (counting) {
                const QString key = keyOf(v);
                if (!key.isEmpty()) ++st.counts[key];
                return true;
            }
            double x = 0;
            if (numberOf(v, &x)) numbers.push_back(x);
            return true;
        });
    }
    if (counting || numbers.empty()) return;
    st.haveNumbers = true;
    double sum = 0;
    st.lo = st.hi = numbers.front();
    for (const double x : numbers) {
        sum += x;
        st.lo = std::min(st.lo, x);
        st.hi = std::max(st.hi, x);
    }
    st.average = sum / double(numbers.size());
    if (numbers.size() > 1) {
        double squares = 0;
        for (const double x : numbers) squares += (x - st.average) * (x - st.average);
        st.deviation = std::sqrt(squares / double(numbers.size() - 1));
    }
    using T = CondRule::Type;
    std::vector<double> sorted;
    const bool needSorted = r.type == T::Top10 || [&] {
        for (const CondValue &v : r.values)
            if (v.type == CondValue::Type::Percentile) return true;
        return false;
    }();
    if (needSorted) {
        sorted = numbers;
        std::sort(sorted.begin(), sorted.end());
    }
    if (r.type == T::Top10) {
        const size_t count = sorted.size();
        size_t k = r.percent ? std::max<size_t>(1, size_t(std::floor(double(count) * r.rank / 100.0))) : size_t(std::max(1, r.rank));
        k = std::min(k, count);
        st.threshold = r.bottom ? sorted[k - 1] : sorted[count - k];
    } else if (r.type == T::AboveAverage) {
        const double spread = r.stdDev * st.deviation;
        st.threshold = r.above ? st.average + spread : st.average - spread;
    } else if (r.type == T::ColorScale || r.type == T::IconSet) {
        for (size_t i = 0; i < r.values.size(); ++i) st.points.push_back(resolve(r.values[int(i)], st, sorted, st.lo, st.hi, rule));
    } else if (r.type == T::DataBar) {
        const CondValue low = r.values.size() > 0 ? r.values[0] : CondValue{CondValue::Type::Min, QString(), true};
        const CondValue high = r.values.size() > 1 ? r.values[1] : CondValue{CondValue::Type::Max, QString(), true};
        st.points.push_back(resolve(low, st, sorted, st.lo, st.hi, rule));
        st.points.push_back(resolve(high, st, sorted, st.lo, st.hi, rule));
    }
}

double CondFormats::resolve(const CondValue &v, RuleState &st, const std::vector<double> &sorted, double lo, double hi, size_t rule) const
{
    using T = CondValue::Type;
    switch (v.type) {
    case T::Min: return lo;
    case T::Max: return hi;
    case T::AutoMin: return std::min(0.0, lo);
    case T::AutoMax: return std::max(0.0, hi);
    case T::Number: return v.value.toDouble();
    case T::Percent: return lo + (hi - lo) * v.value.toDouble() / 100.0;
    case T::Percentile: return percentile(sorted, v.value.toDouble() / 100.0);
    case T::Formula: {
        // its program is the one made for the value at the same place in the rule
        const CondRule &r = m_owner->data().sheet->conditionalFormats[int(rule)];
        for (int i = 0; i < r.values.size(); ++i)
            if (&r.values[i] == &v && size_t(i) < st.valuePrograms.size() && st.valuePrograms[size_t(i)]) {
                GridCells cells(m_owner);
                jp::calc::EvalContext ctx;
                ctx.source = &cells;
                ctx.sheet = m_owner->data().sheetIndex;
                ctx.row = st.firstRow;
                ctx.col = st.firstCol;
                ctx.dateSystem = m_owner->data().dateSystem;
                ctx.now = m_today;
                const Value x = m_evaluator.evaluate(*st.valuePrograms[size_t(i)], ctx);
                return x.isNumber() ? x.number() : 0;
            }
        return 0;
    }
    }
    return 0;
}

bool CondFormats::testOf(size_t rule, quint32 row, quint32 col) const
{
    RuleState &st = state(rule);
    if (!st.test) return false;
    GridCells cells(m_owner);
    jp::calc::EvalContext ctx;
    ctx.source = &cells;
    ctx.sheet = m_owner->data().sheetIndex;
    ctx.row = row;
    ctx.col = col;
    ctx.shiftRows = int(qint64(row) - qint64(st.firstRow));
    ctx.shiftCols = int(qint64(col) - qint64(st.firstCol));
    ctx.dateSystem = m_owner->data().dateSystem;
    ctx.now = m_today;
    m_evaluator.setWorkLimit(100000);
    ++m_tests;
    const Value v = m_evaluator.evaluate(*st.test, ctx);
    if (v.isBool()) return v.boolean();
    if (v.isNumber()) return v.number() != 0;
    return false;
}

const CondCell &CondFormats::at(quint32 row, quint32 col) const
{
    const quint64 key = (quint64(row) << 16) | col;
    const auto found = m_cache.find(key);
    if (found != m_cache.end()) return found->second;
    if (m_cache.size() >= kMaxCached) m_cache.clear();
    return m_cache.emplace(key, compute(row, col)).first->second;
}

CondCell CondFormats::compute(quint32 row, quint32 col) const
{
    using T = CondRule::Type;
    CondCell out;
    const jp::office::sheet::Sheet *sheet = m_owner->data().sheet;
    if (!sheet) return out;
    DifferentialFormat merged;
    bool anyFormat = false;
    const Cell *cell = sheet->cells.find(row, col);
    const Value value = cell ? m_owner->valueOf(*cell) : Value();
    const jp::office::sheet::Styles *styles = m_owner->data().styles;
    for (size_t i = 0; i < size_t(sheet->conditionalFormats.size()); ++i) {
        const CondRule &r = sheet->conditionalFormats[int(i)];
        bool inside = false;
        for (const CellRange &range : r.ranges)
            if (range.contains(row, col)) {
                inside = true;
                break;
            }
        if (!inside) continue;
        prepare(i);
        RuleState &st = state(i);
        bool applies = false;
        double x = 0;
        switch (r.type) {
        case T::ColorScale:
            if (out.hasScale || !st.haveNumbers || !numberOf(value, &x) || st.points.size() < 2 || r.colors.size() < 2) break;
            {
                // the stops in order; below the first and over the last the color is theirs, between two it is mixed from them
                const size_t n = std::min(st.points.size(), size_t(r.colors.size()));
                auto colorAt = [&](size_t k) { return opaque(styles ? styles->resolve(r.colors[int(k)], 0xFFFFFF) : 0xFFFFFF); };
                QRgb color = colorAt(n - 1);
                if (x <= st.points[0]) {
                    color = colorAt(0);
                } else {
                    for (size_t k = 1; k < n; ++k) {
                        if (x <= st.points[k] || k == n - 1) {
                            const double span = st.points[k] - st.points[k - 1];
                            color = x >= st.points[k] ? colorAt(k) : mix(colorAt(k - 1), colorAt(k), span > 0 ? (x - st.points[k - 1]) / span : 1);
                            break;
                        }
                    }
                }
                out.hasScale = true;
                out.scale = color;
                out.any = true;
            }
            applies = true;
            break;
        case T::DataBar:
            if (out.hasBar || !st.haveNumbers || !numberOf(value, &x) || st.points.size() < 2) break;
            {
                const double lo = st.points[0], hi = st.points[1], range = hi - lo;
                const QRgb fill = opaque(styles ? styles->resolve(r.colors.isEmpty() ? jp::office::sheet::ColorSpec::fromRgb(0x638EC6) : r.colors[0], 0x638EC6) : 0x638EC6);
                CondBar bar;
                bar.fill = fill;
                bar.gradient = r.gradient;
                bar.hasBorder = r.border;
                bar.border = r.border ? opaque(styles ? styles->resolve(r.borderColor.isSet() ? r.borderColor : (r.colors.isEmpty() ? jp::office::sheet::ColorSpec() : r.colors[0]), 0x638EC6) : 0x638EC6) : fill;
                bar.fillEnd = mix(fill, 0xFFFFFFFFu, 0.7);
                const bool mixedSigns = lo < 0 && hi > 0 && r.axis != CondRule::Axis::None;
                const double lengthLow = r.minLength / 100.0, lengthHigh = r.maxLength / 100.0;
                if (mixedSigns) {
                    // the axis is where zero is; a bar goes from it to the left (a value under zero) or to the right, as long as the value is of the whole
                    const double axis = (0 - lo) / range;
                    bar.hasAxis = true;
                    bar.axisAt = axis;
                    bar.axis = opaque(styles ? styles->resolve(r.axisColor.isSet() ? r.axisColor : jp::office::sheet::ColorSpec::fromRgb(0x000000), 0x000000) : 0);
                    const double reach = std::min(1.0, std::abs(x) / range);
                    if (x >= 0) {
                        bar.from = axis;
                        bar.to = std::min(1.0, axis + reach);
                    } else {
                        bar.to = axis;
                        bar.from = std::max(0.0, axis - reach);
                        bar.negative = true;
                        const jp::office::sheet::ColorSpec negative = r.negativeSameColor || !r.negativeColor.isSet() ? jp::office::sheet::ColorSpec::fromRgb(0xFF0000) : r.negativeColor;
                        bar.fill = r.negativeSameColor ? fill : opaque(styles ? styles->resolve(negative, 0xFF0000) : 0xFF0000);
                        bar.fillEnd = mix(bar.fill, 0xFFFFFFFFu, 0.7);
                        if (r.border) bar.border = opaque(styles ? styles->resolve(r.negativeBorderColor.isSet() ? r.negativeBorderColor : negative, 0xFF0000) : 0xFF0000);
                    }
                } else {
                    // all one side: from the left, as long as where the value is between the lowest and the highest, from the shortest bar to the longest
                    const double part = range > 0 ? std::max(0.0, std::min(1.0, (x - lo) / range)) : 1.0;
                    bar.from = 0;
                    bar.to = lengthLow + part * (lengthHigh - lengthLow);
                }
                if (r.rightToLeft && !m_owner->rightToLeft()) {   // (a bar grows from the screen's right; the grid of a sheet that reads right to left is mirrored for the screen already)
                    const double from = bar.from, to = bar.to;
                    bar.from = 1 - to;
                    bar.to = 1 - from;
                    bar.axisAt = 1 - bar.axisAt;
                }
                out.hasBar = true;
                out.bar = bar;
                out.any = true;
                if (!r.showValue) out.hideValue = true;
            }
            applies = true;
            break;
        case T::IconSet: {
            const int count = iconSetCount(r.iconSet);
            if (out.iconSet >= 0 || count == 0 || !st.haveNumbers || !numberOf(value, &x)) break;
            int icon = 0;
            for (int k = 1; k < count && size_t(k) < st.points.size(); ++k) {
                const bool reached = k < r.values.size() && !r.values[k].gte ? x > st.points[size_t(k)] : x >= st.points[size_t(k)];
                if (reached) icon = k;
            }
            if (r.reverse) icon = count - 1 - icon;
            out.icon = icon;
            out.iconSet = int(i);
            out.any = true;
            if (!r.showValue) out.hideValue = true;
            applies = true;
            break;
        }
        case T::Top10:
            if (st.haveNumbers && numberOf(value, &x)) applies = r.bottom ? x <= st.threshold : x >= st.threshold;
            break;
        case T::AboveAverage:
            if (st.haveNumbers && numberOf(value, &x)) {
                if (r.stdDev > 0) applies = r.above ? x > st.threshold : x < st.threshold;
                else applies = r.above ? (r.equalAverage ? x >= st.average : x > st.average) : (r.equalAverage ? x <= st.average : x < st.average);
            }
            break;
        case T::UniqueValues:
        case T::DuplicateValues: {
            const QString key = keyOf(value);
            if (key.isEmpty()) break;
            const int times = st.counts.count(key) ? st.counts.at(key) : 0;
            applies = r.type == T::DuplicateValues ? times > 1 : times == 1;
            break;
        }
        default: applies = testOf(i, row, col); break;
        }
        if (!applies) continue;
        const bool formatting = r.type != T::ColorScale && r.type != T::DataBar && r.type != T::IconSet;
        if (formatting && r.dxf >= 0 && styles && size_t(r.dxf) < size_t(styles->dxfs.size())) {
            const DifferentialFormat &d = styles->dxfs[int(r.dxf)];
            // what an earlier rule says stays; the later rule adds what is left
            if (d.bold && !merged.bold) merged.bold = d.bold;
            if (d.italic && !merged.italic) merged.italic = d.italic;
            if (d.strike && !merged.strike) merged.strike = d.strike;
            if (d.underline && !merged.underline) merged.underline = d.underline;
            if (d.fontColor.isSet() && !merged.fontColor.isSet()) merged.fontColor = d.fontColor;
            if (d.hasFill && !merged.hasFill) {
                merged.hasFill = true;
                merged.fill = d.fill;
            }
            if (d.hasBorder) {
                merged.hasBorder = true;
                using S = jp::office::sheet::BorderSide;
                auto side = [](S *to, const S &from) {
                    if (to->style == jp::office::sheet::BorderStyle::None) *to = from;
                };
                side(&merged.border.left, d.border.left);
                side(&merged.border.right, d.border.right);
                side(&merged.border.top, d.border.top);
                side(&merged.border.bottom, d.border.bottom);
            }
            if (d.numberFormat && !merged.numberFormat) merged.numberFormat = d.numberFormat;
            anyFormat = anyFormat || !d.isEmpty();
        }
        if (formatting) out.any = true;
        if (r.stopIfTrue) break;
    }
    if (anyFormat) out.format = formatIndexOf(merged);
    return out;
}

} // namespace jp::grid
