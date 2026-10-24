#include "grid/condformat.h"

#include "grid/gridsheet.h"

#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QDate>
#include <QElapsedTimer>
#include <QHash>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <tuple>

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


// A value as a 64-bit key that is the same for values that count as the same (a number to the digits it is shown to, a text without regard to capitals), and never 0 (0 is "no key"): what
// the rules of duplicate and unique values count by.
quint64 valueKey(const Value &v)
{
    auto mix = [](quint64 kind, quint64 x) {
        quint64 h = (kind + 0x9E3779B97F4A7C15ull) ^ (x + 0x9E3779B97F4A7C15ull + (kind << 6));
        h ^= h >> 33;
        h *= 0xFF51AFD7ED558CCDull;
        h ^= h >> 33;
        h *= 0xC4CEB9FE1A85EC53ull;
        h ^= h >> 33;
        return h ? h : 1;
    };
    if (v.isNumber()) {
        // (the digits it is shown to: the first fifteen, the way General shows a number; written by the C library, which is a good deal quicker than the text a cell shows)
        double x = v.number();
        if (x == 0) x = 0;   // (minus zero is zero)
        char digits[40];
        const int n = std::snprintf(digits, sizeof digits, "%.14e", x);
        quint64 h = 14695981039346656037ull;
        for (int i = 0; i < n; ++i) h = (h ^ quint8(digits[i])) * 1099511628211ull;
        return mix(1, h);
    }
    if (v.isText()) return mix(2, quint64(qHash(v.text().toLower())));
    if (v.isBool()) return mix(3, v.boolean() ? 1 : 0);
    if (v.isError()) return mix(4, quint64(int(v.errorKind())));
    return 0;
}

QRgb mix(QRgb a, QRgb b, double t)
{
    auto channel = [&](int shift) { return int(std::lround(double((a >> shift) & 0xFF) + (double((b >> shift) & 0xFF) - double((a >> shift) & 0xFF)) * t)); };
    return 0xFF000000u | (quint32(channel(16)) << 16) | (quint32(channel(8)) << 8) | quint32(channel(0));
}

constexpr double kNoNumber = std::numeric_limits<double>::quiet_NaN();

// What is kept of a cell for the figures of a set of ranges: where it is, its number (not a number: NaN), and the key of its value.
struct StatCell {
    quint64 id = 0;           // (row << 16) | column
    double num = kNoNumber;
    quint64 key = 0;
};

QString noticeText(int which, const CondFormats::Limits &limits)
{
    using M = QCoreApplication;
    switch (which) {
    case 0: return M::translate("jp::grid::CondFormats", "Some conditional formats cover more cells than JeffCrunch works out for one sheet at a time (%1), so those are not shown.").arg(QLocale().toString(limits.statCells));
    case 1: return M::translate("jp::grid::CondFormats", "This sheet has more conditional formats than JeffCrunch shows (%1 ranges): the ones after them are not shown.").arg(QLocale().toString(limits.ranges));
    case 2: return M::translate("jp::grid::CondFormats", "A conditional format has a formula that took more work than JeffCrunch allows for one cell, so that format is not shown where this happens.");
    case 3: return M::translate("jp::grid::CondFormats", "Conditional formats are shown for the first %1 cells of each part of the screen only.").arg(QLocale().toString(qint64(60000)));
    }
    return QString();
}

} // namespace

// ---- the index of the ranges of the rules: a tree of rectangles (packed by sorting, tile by tile), so that the rules that have a cell are found without looking at them all

struct CondFormats::Index {
    struct Entry {
        quint32 r1, r2, c1, c2;
        int rule;
    };
    struct Node {
        quint32 r1, r2, c1, c2;
        quint32 first, count;   // its entries (a leaf), or its children among `kids`
        bool leaf;
    };
    std::vector<Entry> entries;
    std::vector<Node> nodes;
    std::vector<quint32> kids;
    int root = -1;

    static constexpr size_t kFan = 16;

    void build()
    {
        nodes.clear();
        kids.clear();
        root = -1;
        const size_t n = entries.size();
        if (n == 0) return;
        const size_t leaves = (n + kFan - 1) / kFan;
        const size_t slices = size_t(std::ceil(std::sqrt(double(leaves))));
        std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) { return quint64(a.r1) + a.r2 < quint64(b.r1) + b.r2; });
        const size_t sliceSize = slices * kFan;
        for (size_t from = 0; from < n; from += sliceSize) {
            const size_t to = std::min(n, from + sliceSize);
            std::sort(entries.begin() + qptrdiff(from), entries.begin() + qptrdiff(to), [](const Entry &a, const Entry &b) { return quint64(a.c1) + a.c2 < quint64(b.c1) + b.c2; });
        }
        std::vector<quint32> level;
        for (size_t from = 0; from < n; from += kFan) {
            const size_t to = std::min(n, from + kFan);
            Node node{entries[from].r1, entries[from].r2, entries[from].c1, entries[from].c2, quint32(from), quint32(to - from), true};
            for (size_t i = from; i < to; ++i) {
                node.r1 = std::min(node.r1, entries[i].r1);
                node.r2 = std::max(node.r2, entries[i].r2);
                node.c1 = std::min(node.c1, entries[i].c1);
                node.c2 = std::max(node.c2, entries[i].c2);
            }
            level.push_back(quint32(nodes.size()));
            nodes.push_back(node);
        }
        while (level.size() > 1) {
            const size_t m = level.size();
            const size_t parents = (m + kFan - 1) / kFan;
            const size_t cuts = size_t(std::ceil(std::sqrt(double(parents))));
            std::sort(level.begin(), level.end(), [&](quint32 a, quint32 b) { return quint64(nodes[a].r1) + nodes[a].r2 < quint64(nodes[b].r1) + nodes[b].r2; });
            const size_t cutSize = cuts * kFan;
            for (size_t from = 0; from < m; from += cutSize) {
                const size_t to = std::min(m, from + cutSize);
                std::sort(level.begin() + qptrdiff(from), level.begin() + qptrdiff(to), [&](quint32 a, quint32 b) { return quint64(nodes[a].c1) + nodes[a].c2 < quint64(nodes[b].c1) + nodes[b].c2; });
            }
            std::vector<quint32> up;
            for (size_t from = 0; from < m; from += kFan) {
                const size_t to = std::min(m, from + kFan);
                Node node{nodes[level[from]].r1, nodes[level[from]].r2, nodes[level[from]].c1, nodes[level[from]].c2, quint32(kids.size()), quint32(to - from), false};
                for (size_t i = from; i < to; ++i) {
                    const Node &c = nodes[level[i]];
                    node.r1 = std::min(node.r1, c.r1);
                    node.r2 = std::max(node.r2, c.r2);
                    node.c1 = std::min(node.c1, c.c1);
                    node.c2 = std::max(node.c2, c.c2);
                    kids.push_back(level[i]);
                }
                up.push_back(quint32(nodes.size()));
                nodes.push_back(node);
            }
            level = std::move(up);
        }
        root = int(level.front());
    }

    // Calls f(rule) for each entry that holds the cell (a rule may be called more than once); f gives false to stop.
    template <class F>
    void query(quint32 row, quint32 col, F f, qint64 *steps) const
    {
        if (root < 0) return;
        std::vector<quint32> stack;
        stack.push_back(quint32(root));
        while (!stack.empty()) {
            const Node &node = nodes[stack.back()];
            stack.pop_back();
            ++*steps;
            if (row < node.r1 || row > node.r2 || col < node.c1 || col > node.c2) continue;
            if (node.leaf) {
                for (quint32 i = node.first; i < node.first + node.count; ++i) {
                    const Entry &e = entries[i];
                    ++*steps;
                    if (row >= e.r1 && row <= e.r2 && col >= e.c1 && col <= e.c2 && !f(e.rule)) return;
                }
            } else {
                for (quint32 i = node.first; i < node.first + node.count; ++i) stack.push_back(kids[i]);
            }
        }
    }
};

// What is worked out for a set of ranges, once, however many rules have them: the cells that are there (sorted by place) with their numbers and the keys of their values, the number of
// times each value is there, and the sum, the lowest, and the highest of the numbers. Made in slices, and kept up to date a cell at a time.
struct CondFormats::RangeStats {
    enum class Stage : quint8 { Scan, Count, Done, Refused };
    std::vector<CellRange> ranges;
    bool needCounts = false;
    Stage stage = Stage::Scan;
    // where the scan is: the range, and the last cell taken in it
    size_t rangeAt = 0;
    bool haveLast = false;
    quint64 lastId = 0;
    bool staleBuild = false;   // a value changed while the figures were being made: they are made again from the beginning when they are
    std::vector<StatCell> cells;
    std::unordered_map<quint64, quint32> counts;
    size_t countAt = 0;
    qint64 numbers = 0;
    long double sum = 0, sumSquares = 0;
    double lo = 0, hi = 0;
    bool extremaStale = false;
    quint64 version = 1;      // raised by each change of the figures: what a rule made of them is made again when it is not of this
    quint64 changeSerial = 0; // the last change that was told to these (a set several rules have is told once)

    void clearData()
    {
        cells.clear();
        cells.shrink_to_fit();
        counts.clear();
        countAt = 0;
        numbers = 0;
        sum = sumSquares = 0;
        lo = hi = 0;
        extremaStale = false;
        rangeAt = 0;
        haveLast = false;
        staleBuild = false;
    }
    void addNumber(double x)
    {
        if (numbers == 0) lo = hi = x;
        else {
            lo = std::min(lo, x);
            hi = std::max(hi, x);
        }
        ++numbers;
        sum += x;
        sumSquares += (long double)x * x;
    }
    void removeNumber(double x)
    {
        --numbers;
        sum -= x;
        sumSquares -= (long double)x * x;
        if (numbers <= 0) {
            numbers = 0;
            sum = sumSquares = 0;
            lo = hi = 0;
            extremaStale = false;
        } else if (x <= lo || x >= hi) {
            extremaStale = true;
        }
    }
    void settleExtrema()
    {
        if (!extremaStale) return;
        extremaStale = false;
        bool first = true;
        for (const StatCell &c : cells) {
            if (std::isnan(c.num)) continue;
            if (first) lo = hi = c.num;
            else {
                lo = std::min(lo, c.num);
                hi = std::max(hi, c.num);
            }
            first = false;
        }
    }
};

// What a rule works out once and keeps.
struct CondFormats::RuleState {
    bool compiled = false, never = false;
    std::shared_ptr<const jp::calc::Program> test;   // the test of a rule that has one, written for the first cell of its first range
    std::vector<std::shared_ptr<const jp::calc::Program>> valuePrograms;   // the formulas of the values of a scale, bar, or icon set
    quint32 firstRow = 0, firstCol = 0;
    RangeStats *stats = nullptr;   // the figures of its cells (a rule of statistics)
    quint64 derivedFrom = 0;       // the version of the figures `points` and the rest were made from
    bool haveNumbers = false;
    double lo = 0, hi = 0, average = 0, deviation = 0;
    double threshold = 0;                      // top and bottom: the value a cell has to reach; average: the limit
    std::vector<double> points;                // a scale: the values of its stops; an icon set: the values the icons change at; a bar: the lowest and highest
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
    m_stats.clear();
    m_index.reset();
    m_cache.clear();
    m_stale = false;
    m_formats.clear();
    m_statCells = 0;
    m_building = false;
    m_incomplete = false;
    m_noticesGiven = 0;
    m_noticesWanted = 0;
}

bool CondFormats::any() const
{
    const jp::office::sheet::Sheet *s = m_owner->data().sheet;
    return s && !s->conditionalFormats.isEmpty();
}

void CondFormats::valuesChanged()
{
    m_stale = true;
    // (every figure is made again, one set after another, in slices)
    for (auto &entry : m_stats) {
        RangeStats &rs = *entry.second;
        if (rs.stage == RangeStats::Stage::Refused) continue;
        m_statCells -= qint64(rs.cells.size());
        rs.clearData();
        rs.stage = RangeStats::Stage::Scan;
        ++rs.version;
        m_building = true;
    }
}

void CondFormats::valueChanged(quint32 row, quint32 col)
{
    const jp::office::sheet::Sheet *s = m_owner->data().sheet;
    if (!s || s->conditionalFormats.isEmpty()) return;
    m_stale = true;
    if (m_stats.empty()) return;
    static quint64 serial = 0;
    ++serial;
    std::vector<int> rules;
    rulesAt(row, col, &rules);
    for (const int rule : rules) {
        if (size_t(rule) >= m_states.size() || !m_states[size_t(rule)] || !m_states[size_t(rule)]->stats) continue;
        RangeStats *rs = m_states[size_t(rule)]->stats;
        if (rs->changeSerial == serial || rs->stage == RangeStats::Stage::Refused) continue;
        rs->changeSerial = serial;
        if (rs->stage == RangeStats::Stage::Done) applyChange(rs, row, col);
        else rs->staleBuild = true;
    }
}

// One cell's figures made again from what the cell holds now: the number, the key, and the sums and counts (and the lowest and highest, when the cell was one of them) changed by it.
void CondFormats::applyChange(RangeStats *rs, quint32 row, quint32 col) const
{
    const jp::office::sheet::Sheet *sheet = m_owner->data().sheet;
    const quint64 id = (quint64(row) << 16) | col;
    auto at = std::lower_bound(rs->cells.begin(), rs->cells.end(), id, [](const StatCell &c, quint64 v) { return c.id < v; });
    const bool had = at != rs->cells.end() && at->id == id;
    const Cell *cell = sheet->cells.find(row, col);
    if (had) {
        if (!std::isnan(at->num)) rs->removeNumber(at->num);
        if (at->key) {
            const auto found = rs->counts.find(at->key);
            if (found != rs->counts.end() && --found->second == 0) rs->counts.erase(found);
        }
    }
    if (!cell) {
        if (had) {
            rs->cells.erase(at);
            --m_statCells;
        }
    } else {
        const Value v = m_owner->valueOf(*cell);
        StatCell now;
        now.id = id;
        double x = 0;
        if (numberOf(v, &x)) {
            now.num = x;
            rs->addNumber(x);
        }
        if (rs->needCounts) {
            now.key = valueKey(v);
            if (now.key) ++rs->counts[now.key];
        }
        if (had) {
            *at = now;
        } else {
            rs->cells.insert(at, now);
            ++m_statCells;
        }
    }
    ++rs->version;
    ++m_incremental;
}

void CondFormats::noteLimit(Limit which) const { m_noticesWanted |= 1u << unsigned(which); }

void CondFormats::reportTruncated() const { noteLimit(Limit::Truncated); }

QString CondFormats::takeNotice() const
{
    for (unsigned bit = 0; bit < 4; ++bit) {
        const unsigned mask = 1u << bit;
        if ((m_noticesWanted & mask) && !(m_noticesGiven & mask)) {
            m_noticesGiven |= mask;
            return noticeText(int(bit), m_limits);
        }
    }
    return QString();
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

CondFormats::RuleState &CondFormats::state(size_t rule) const
{
    if (m_states.size() <= rule) m_states.resize(rule + 1);
    if (!m_states[rule]) m_states[rule] = std::make_unique<RuleState>();
    return *m_states[rule];
}

// ---- the index

const CondFormats::Index &CondFormats::index() const
{
    if (m_index) return *m_index;
    m_index = std::make_unique<Index>();
    const jp::office::sheet::Sheet *sheet = m_owner->data().sheet;
    if (!sheet) return *m_index;
    // (the rules in the order of their priority, and as many of their ranges as the limit allows: a rule that would go past it is not shown, nor are the rules after it)
    for (int i = 0; i < sheet->conditionalFormats.size(); ++i) {
        const CondRule &r = sheet->conditionalFormats[i];
        if (qint64(m_index->entries.size()) + r.ranges.size() > m_limits.ranges) {
            noteLimit(Limit::Ranges);
            break;
        }
        for (const CellRange &range : r.ranges)
            if (range.valid) m_index->entries.push_back({range.row1, range.row2, range.col1, range.col2, i});
    }
    m_index->build();
    return *m_index;
}

bool CondFormats::covers(quint32 row, quint32 col) const
{
    if (!any()) return false;
    bool found = false;
    index().query(row, col, [&](int) {
        found = true;
        return false;
    }, &m_indexSteps);
    return found;
}

void CondFormats::rulesAt(quint32 row, quint32 col, std::vector<int> *out) const
{
    out->clear();
    index().query(row, col, [&](int rule) {
        out->push_back(rule);
        return true;
    }, &m_indexSteps);
    if (out->size() > 1) {
        std::sort(out->begin(), out->end());
        out->erase(std::unique(out->begin(), out->end()), out->end());
    }
}

// ---- the figures of a set of cells

// The figures of the cells of a rule: found among those made for the same ranges (a rule that has the same cells as another uses them), or begun.
CondFormats::RangeStats *CondFormats::statsFor(size_t rule) const
{
    const CondRule &r = m_owner->data().sheet->conditionalFormats[int(rule)];
    std::vector<CellRange> key;
    for (const CellRange &range : r.ranges)
        if (range.valid) key.push_back(range);
    std::sort(key.begin(), key.end(), [](const CellRange &a, const CellRange &b) {
        return std::tie(a.row1, a.col1, a.row2, a.col2) < std::tie(b.row1, b.col1, b.row2, b.col2);
    });
    key.erase(std::unique(key.begin(), key.end()), key.end());
    const bool counting = r.type == CondRule::Type::UniqueValues || r.type == CondRule::Type::DuplicateValues;
    const auto found = m_stats.find(key);
    if (found != m_stats.end()) {
        RangeStats *rs = found->second.get();
        if (counting && !rs->needCounts && rs->stage != RangeStats::Stage::Refused) {
            // (the keys of the values were not kept: the cells are gone through again)
            m_statCells -= qint64(rs->cells.size());
            rs->clearData();
            rs->needCounts = true;
            rs->stage = RangeStats::Stage::Scan;
            ++rs->version;
            m_building = true;
        }
        return rs;
    }
    auto made = std::make_unique<RangeStats>();
    made->ranges = key;
    made->needCounts = counting;
    RangeStats *rs = made.get();
    m_stats.emplace(std::move(key), std::move(made));
    m_building = true;
    return rs;
}

// Goes on with the figures of a set of cells for about `budget` cells: the cells are taken range by range, in order of row and column (where it stopped is kept), then the keys of the
// values are counted; when it is done the figures are ready. A set that would take the sheet's cells held for figures past the limit is given up.
qint64 CondFormats::advance(RangeStats *rs, qint64 budget) const
{
    const qint64 gone = advanceCells(rs, budget);
    m_figureCells += gone;
    return gone;
}

qint64 CondFormats::advanceCells(RangeStats *rs, qint64 budget) const
{
    using Stage = RangeStats::Stage;
    const jp::office::sheet::Sheet *sheet = m_owner->data().sheet;
    if (!sheet) return 0;
    qint64 worked = 0;   // the cells gone through, for whoever counts them
    if (rs->stage == Stage::Scan) {
        qint64 taken = 0;
        while (rs->rangeAt < rs->ranges.size() && taken < budget) {
            const CellRange &r = rs->ranges[rs->rangeAt];
            const quint32 startRow = rs->haveLast ? quint32(rs->lastId >> 16) : r.row1;
            bool stopped = false;
            sheet->cells.forEachInRange(startRow, r.col1, r.row2, r.col2, [&](const Cell &c) {
                const quint64 id = (quint64(c.row) << 16) | c.col;
                if (rs->haveLast && id <= rs->lastId) return true;
                if (taken >= budget) {
                    stopped = true;
                    return false;
                }
                ++taken;
                const Value v = m_owner->valueOf(c);
                StatCell sc;
                sc.id = id;
                double x = 0;
                if (numberOf(v, &x)) {
                    sc.num = x;
                    rs->addNumber(x);
                }
                if (rs->needCounts) sc.key = valueKey(v);
                rs->cells.push_back(sc);
                rs->lastId = id;
                rs->haveLast = true;
                return true;
            });
            if (m_statCells + qint64(rs->cells.size()) > m_limits.statCells) {
                // (more cells than the sheet's rules may hold figures for: this set is given up, and its rules show nothing)
                rs->clearData();
                rs->stage = Stage::Refused;
                noteLimit(Limit::StatCells);
                return taken;
            }
            if (stopped) break;
            ++rs->rangeAt;
            rs->haveLast = false;
        }
        if (rs->rangeAt < rs->ranges.size()) return taken;
        if (rs->staleBuild) {
            // (a value changed while the cells were being taken: the figures are made again from the beginning)
            rs->clearData();
            return taken;
        }
        // several ranges may name a cell twice, or give the cells out of order: the cells are put in order of place, each once, and the figures made again from them
        bool ordered = true;
        for (size_t i = 1; i < rs->cells.size() && ordered; ++i) ordered = rs->cells[i - 1].id < rs->cells[i].id;
        if (!ordered) {
            std::sort(rs->cells.begin(), rs->cells.end(), [](const StatCell &a, const StatCell &b) { return a.id < b.id; });
            rs->cells.erase(std::unique(rs->cells.begin(), rs->cells.end(), [](const StatCell &a, const StatCell &b) { return a.id == b.id; }), rs->cells.end());
            rs->numbers = 0;
            rs->sum = rs->sumSquares = 0;
            rs->lo = rs->hi = 0;
            for (const StatCell &c : rs->cells)
                if (!std::isnan(c.num)) rs->addNumber(c.num);
        }
        worked = taken;
        m_statCells += qint64(rs->cells.size());
        rs->stage = rs->needCounts ? Stage::Count : Stage::Done;
        rs->countAt = 0;
        if (rs->needCounts) rs->counts.reserve(rs->cells.size());   // (the table is made once, not grown by doubling in the middle of a slice)
        if (rs->stage == Stage::Done) {
            ++m_passes;
            ++rs->version;
        }
    }
    if (rs->stage == Stage::Count) {
        qint64 counted = 0;
        while (rs->countAt < rs->cells.size() && counted < budget) {
            const quint64 key = rs->cells[rs->countAt++].key;
            if (key) ++rs->counts[key];
            ++counted;
        }
        worked += counted;
        if (rs->countAt >= rs->cells.size()) {
            rs->stage = Stage::Done;
            ++m_passes;
            ++rs->version;
        }
    }
    return worked;
}

bool CondFormats::step(qint64 budgetMicros)
{
    using Stage = RangeStats::Stage;
    QElapsedTimer clock;
    clock.start();
    m_paintWork = 0;      // (the paint that follows has its steps again)
    m_syncUsed = 0;
    m_incomplete = false;
    bool more = false;
    for (auto &entry : m_stats) {
        RangeStats &rs = *entry.second;
        while (rs.stage == Stage::Scan || rs.stage == Stage::Count) {
            advance(&rs, m_limits.sliceCells);
            if (rs.stage != Stage::Done && rs.stage != Stage::Refused && clock.nsecsElapsed() >= budgetMicros * 1000) {
                more = true;
                break;
            }
        }
        if (more) break;
    }
    m_building = more;
    for (auto &entry : m_stats)
        if (entry.second->stage == Stage::Scan || entry.second->stage == Stage::Count) m_building = true;
    return m_building;
}

void CondFormats::settle()
{
    using Stage = RangeStats::Stage;
    m_unlimited = true;
    const jp::office::sheet::Sheet *sheet = m_owner->data().sheet;
    if (sheet)
        for (size_t i = 0; i < size_t(sheet->conditionalFormats.size()); ++i) prepare(i);
    for (auto &entry : m_stats) {
        RangeStats &rs = *entry.second;
        while (rs.stage == Stage::Scan || rs.stage == Stage::Count) advance(&rs, std::numeric_limits<qint64>::max() / 4);
    }
    m_building = false;
    m_incomplete = false;
    m_paintWork = 0;
}

// What a rule's figures are made into: the lowest and highest, the average and the spread, the value a cell has to reach for the highest or lowest, and the values the stops of a scale or
// the icons of a set stand for.
void CondFormats::derive(size_t rule) const
{
    using T = CondRule::Type;
    RuleState &st = state(rule);
    RangeStats *rs = st.stats;
    const CondRule &r = m_owner->data().sheet->conditionalFormats[int(rule)];
    rs->settleExtrema();
    st.points.clear();
    st.haveNumbers = rs->numbers > 0;
    if (!st.haveNumbers) return;
    st.lo = rs->lo;
    st.hi = rs->hi;
    st.average = double(rs->sum / (long double)rs->numbers);
    st.deviation = 0;
    if (rs->numbers > 1) {
        const long double mean = rs->sum / (long double)rs->numbers;
        long double variance = (rs->sumSquares - (long double)rs->numbers * mean * mean) / (long double)(rs->numbers - 1);
        if (variance < 0) variance = 0;
        st.deviation = double(std::sqrt(double(variance)));
    }
    // the numbers in order, as far as a rule needs them (the k-th of them is found without sorting them all)
    std::vector<double> numbers;
    auto numbersNow = [&]() -> std::vector<double> & {
        if (numbers.empty()) {
            numbers.reserve(size_t(rs->numbers));
            for (const StatCell &c : rs->cells)
                if (!std::isnan(c.num)) numbers.push_back(c.num);
        }
        return numbers;
    };
    auto kth = [&](size_t k) {
        std::vector<double> &v = numbersNow();
        std::nth_element(v.begin(), v.begin() + qptrdiff(k), v.end());
        return v[k];
    };
    auto percentileOf = [&](double p) {
        const size_t count = size_t(rs->numbers);
        const double at = std::max(0.0, std::min(1.0, p)) * double(count - 1);
        const size_t lo = size_t(std::floor(at)), hi = size_t(std::ceil(at));
        const double a = kth(lo), b = hi == lo ? a : kth(hi);
        return a + (b - a) * (at - double(lo));
    };
    auto resolve = [&](int valueIndex) -> double {
        using V = CondValue::Type;
        const CondValue &v = r.values[valueIndex];
        switch (v.type) {
        case V::Min: return st.lo;
        case V::Max: return st.hi;
        case V::AutoMin: return std::min(0.0, st.lo);
        case V::AutoMax: return std::max(0.0, st.hi);
        case V::Number: return v.value.toDouble();
        case V::Percent: return st.lo + (st.hi - st.lo) * v.value.toDouble() / 100.0;
        case V::Percentile: return percentileOf(v.value.toDouble() / 100.0);
        case V::Formula: {
            if (size_t(valueIndex) >= st.valuePrograms.size() || !st.valuePrograms[size_t(valueIndex)]) return 0;
            GridCells cells(m_owner);
            jp::calc::EvalContext ctx;
            ctx.source = &cells;
            ctx.sheet = m_owner->data().sheetIndex;
            ctx.row = st.firstRow;
            ctx.col = st.firstCol;
            ctx.dateSystem = m_owner->data().dateSystem;
            ctx.now = m_today;
            m_evaluator.setWorkLimit(m_limits.testWork);
            const Value x = m_evaluator.evaluate(*st.valuePrograms[size_t(valueIndex)], ctx);
            if (m_evaluator.status() == jp::calc::EvalStatus::TooMuchWork) noteLimit(Limit::FormulaWork);
            return x.isNumber() ? x.number() : 0;
        }
        }
        return 0;
    };
    if (r.type == T::Top10) {
        const size_t count = size_t(rs->numbers);
        size_t k = r.percent ? std::max<size_t>(1, size_t(std::floor(double(count) * r.rank / 100.0))) : size_t(std::max(1, r.rank));
        k = std::min(k, count);
        st.threshold = r.bottom ? kth(k - 1) : kth(count - k);
    } else if (r.type == T::AboveAverage) {
        const double spread = r.stdDev * st.deviation;
        st.threshold = r.above ? st.average + spread : st.average - spread;
    } else if (r.type == T::ColorScale || r.type == T::IconSet) {
        for (int i = 0; i < r.values.size(); ++i) st.points.push_back(resolve(i));
    } else if (r.type == T::DataBar) {
        // (the lowest and highest of a bar: its first two values, or the lowest and highest the cells have)
        const CondValue low = r.values.size() > 0 ? r.values[0] : CondValue{CondValue::Type::Min, QString(), true};
        const CondValue high = r.values.size() > 1 ? r.values[1] : CondValue{CondValue::Type::Max, QString(), true};
        auto point = [&](const CondValue &v, int index) {
            if (index < r.values.size()) return resolve(index);
            return v.type == CondValue::Type::Min ? st.lo : st.hi;
        };
        st.points.push_back(point(low, 0));
        st.points.push_back(point(high, 1));
    }
}

// Whether a rule can be worked out for a cell now: it has no figures to wait for, or they are ready (made as far as a few thousand cells at once, the rest in slices).
CondFormats::Ready CondFormats::prepare(size_t rule) const
{
    using Stage = RangeStats::Stage;
    RuleState &st = state(rule);
    if (st.never) return Ready::Never;
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
    if (!isStatisticsRule(r.type)) return Ready::Yes;
    if (!st.stats) st.stats = statsFor(rule);
    RangeStats *rs = st.stats;
    if (rs->stage == Stage::Refused) {
        st.never = true;
        return Ready::Never;
    }
    if (rs->stage != Stage::Done) {
        // (the cells a paint goes through for figures are counted for the paint, all its rules and cells together; when they are used up the figures wait for the slices between paints)
        const qint64 allowed = m_unlimited ? std::numeric_limits<qint64>::max() / 4 : std::max<qint64>(0, m_limits.syncCells - m_syncUsed);
        if (allowed > 0) m_syncUsed += advance(rs, allowed);
        if (rs->stage == Stage::Refused) {
            st.never = true;
            return Ready::Never;
        }
        if (rs->stage != Stage::Done) {
            m_building = true;
            return Ready::Pending;
        }
    }
    if (st.derivedFrom != rs->version) {
        derive(rule);
        st.derivedFrom = rs->version;
    }
    return Ready::Yes;
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
    m_evaluator.setWorkLimit(m_limits.testWork);
    ++m_tests;
    const Value v = m_evaluator.evaluate(*st.test, ctx);
    m_paintWork += std::max<qint64>(1, m_evaluator.work());
    if (m_evaluator.status() == jp::calc::EvalStatus::TooMuchWork) noteLimit(Limit::FormulaWork);   // (the rule did not hold here for want of room: said once)
    if (v.isBool()) return v.boolean();
    if (v.isNumber()) return v.number() != 0;
    return false;
}

CondCell CondFormats::at(quint32 row, quint32 col) const
{
    if (m_stale) {
        m_cache.clear();
        m_stale = false;
    }
    if (!covers(row, col)) return CondCell();   // (a cell no rule has costs one look at the index, and nothing is kept for it)
    const quint64 key = (quint64(row) << 16) | col;
    const auto found = m_cache.find(key);
    if (found != m_cache.end()) return found->second;
    bool incomplete = false;
    const CondCell cell = compute(row, col, &incomplete);
    if (incomplete) {
        // (a rule of the cell has figures that are not ready, or the paint has used its steps: what is known now is shown, nothing is kept, and the cell is asked about again)
        m_incomplete = true;
        return cell;
    }
    if (m_cache.size() >= kMaxCached) m_cache.clear();   // (what callers hold is theirs: values)
    m_cache.emplace(key, cell);
    return cell;
}

CondCell CondFormats::compute(quint32 row, quint32 col, bool *incomplete) const
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
    std::vector<int> rules;
    rulesAt(row, col, &rules);
    for (const int ruleIndex : rules) {
        const size_t i = size_t(ruleIndex);
        const CondRule &r = sheet->conditionalFormats[int(i)];
        const Ready ready = prepare(i);
        if (ready == Ready::Never) continue;
        if (ready == Ready::Pending) {
            *incomplete = true;
            continue;
        }
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
            const quint64 key = valueKey(value);
            if (!key) break;
            const auto found = st.stats->counts.find(key);
            const quint32 times = found == st.stats->counts.end() ? 0 : found->second;
            applies = r.type == T::DuplicateValues ? times > 1 : times == 1;
            break;
        }
        default:
            // (a rule's formula: a paint has only so many steps to spend on them, and the cells left are done in the paints after it)
            if (!m_unlimited && m_paintWork >= m_limits.paintWork) {
                *incomplete = true;
                continue;
            }
            applies = testOf(i, row, col);
            break;
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
