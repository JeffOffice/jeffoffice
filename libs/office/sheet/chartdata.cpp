#include "sheet/chartdata.h"

#include <QCoreApplication>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>
#include <limits>

namespace jp::office::sheet {

ChartCells::~ChartCells() = default;

namespace {

constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

// The text between the outer parentheses, if one pair holds the whole of it.
QString withoutOuterParentheses(const QString &text)
{
    if (text.size() < 2 || text.front() != QLatin1Char('(') || text.back() != QLatin1Char(')')) return text;
    int depth = 0;
    bool quoted = false;
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        if (c == QLatin1Char('\'')) quoted = !quoted;
        if (quoted) continue;
        if (c == QLatin1Char('(')) ++depth;
        else if (c == QLatin1Char(')') && --depth == 0) return i == text.size() - 1 ? text.mid(1, text.size() - 2) : text;
    }
    return text;
}

// The areas of a list, split at the commas that are not in quotes or parentheses.
QStringList splitAreas(const QString &text)
{
    QStringList out;
    int depth = 0, start = 0;
    bool quoted = false;
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        if (c == QLatin1Char('\'')) quoted = !quoted;
        if (quoted) continue;
        if (c == QLatin1Char('(')) ++depth;
        else if (c == QLatin1Char(')')) --depth;
        else if (c == QLatin1Char(',') && depth == 0) {
            out << text.mid(start, i - start).trimmed();
            start = i + 1;
        }
    }
    out << text.mid(start).trimmed();
    return out;
}

// A sheet's name as a reference writes it: in quotes with a quote written twice, or plain.
bool sheetNameOf(const QString &text, QString *name)
{
    if (text.size() >= 2 && text.front() == QLatin1Char('\'') && text.back() == QLatin1Char('\'')) {
        *name = text.mid(1, text.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
        return !name->isEmpty();
    }
    *name = text;
    return !name->isEmpty() && !name->contains(QLatin1Char('\'')) && !name->contains(QLatin1Char('(')) && !name->contains(QLatin1Char(':')) && !name->contains(QLatin1Char('['));
}

// The column (from 0) a name of letters is, or -1.
int columnOfLetters(const QString &letters)
{
    quint32 row = 0, col = 0;
    if (letters.isEmpty() || letters.size() > 3 || !parseCellRef(letters + QLatin1Char('1'), &row, &col)) return -1;
    return int(col);
}

// What one resolution of a reference has done so far: the areas and names it has visited (the whole is bounded by that, however names list one another), and the names it is inside of
// (a name is not read inside itself).
struct Walk {
    int visited = 0;
    QStringList inside;   // "sheet|NAME" of the names being read
};

bool blocksOf(const QString &ref, int ownSheet, const ChartCells &cells, int depth, Walk *walk, QVector<ChartBlock> *out)
{
    static const QRegularExpression columnsRe(QStringLiteral("^\\$?([A-Za-z]{1,3}):\\$?([A-Za-z]{1,3})$")), rowsRe(QStringLiteral("^\\$?(\\d{1,7}):\\$?(\\d{1,7})$")),
        nameRe(QStringLiteral("^[A-Za-z_\\\\][A-Za-z0-9_.\\\\]*$"));
    QString text = ref.trimmed();
    if (text.startsWith(QLatin1Char('='))) text = text.mid(1).trimmed();
    if (text.isEmpty()) return false;
    bool any = false;
    for (const QString &area : splitAreas(withoutOuterParentheses(text))) {
        if (out->size() >= kMaxChartBlocks || walk->visited >= kMaxChartVisits) break;
        ++walk->visited;
        if (area.isEmpty()) continue;
        // the sheet, up to the first "!" that is not in quotes
        int bang = -1;
        bool quoted = false;
        for (int i = 0; i < area.size() && bang < 0; ++i) {
            if (area[i] == QLatin1Char('\'')) quoted = !quoted;
            else if (area[i] == QLatin1Char('!') && !quoted) bang = i;
        }
        int sheet = ownSheet;
        QString tail = area;
        if (bang >= 0) {
            const QString head = area.left(bang).trimmed();
            tail = area.mid(bang + 1).trimmed();
            if (head != QLatin1String("[0]")) {   // ("[0]!" is the workbook itself: a name of it)
                QString name;
                if (!sheetNameOf(head, &name)) continue;
                sheet = cells.sheetIndex(name);
                if (sheet < 0) continue;
            }
        }
        if (tail.isEmpty()) continue;
        ChartBlock b;
        b.sheet = sheet;
        CellRange r;
        QRegularExpressionMatch m;
        if (parseRangeRef(tail, &r)) {
            b.range = r;
        } else if ((m = columnsRe.match(tail)).hasMatch()) {
            const int c1 = columnOfLetters(m.captured(1).toUpper()), c2 = columnOfLetters(m.captured(2).toUpper());
            if (c1 < 0 || c2 < 0) continue;
            b.range = CellRange{0, quint32(std::min(c1, c2)), kMaxRows - 1, quint32(std::max(c1, c2)), true};
        } else if ((m = rowsRe.match(tail)).hasMatch()) {
            const qint64 r1 = m.captured(1).toLongLong(), r2 = m.captured(2).toLongLong();
            if (r1 < 1 || r2 < 1 || r1 > kMaxRows || r2 > kMaxRows) continue;
            b.range = CellRange{quint32(std::min(r1, r2) - 1), 0, quint32(std::max(r1, r2) - 1), kMaxColumns - 1, true};
        } else if (nameRe.match(tail).hasMatch()) {
            // a name: what it stands for is read as a reference in its turn, a few levels down at most, and never inside itself
            if (depth >= kMaxChartNameDepth) continue;
            const QString key = QString::number(sheet) + QLatin1Char('|') + tail.toUpper();
            if (walk->inside.contains(key)) continue;
            const QString stands = cells.definedName(tail, sheet);
            if (stands.isEmpty()) continue;
            walk->inside.append(key);
            const bool found = blocksOf(stands, sheet, cells, depth + 1, walk, out);
            walk->inside.removeLast();
            if (!found) continue;
            any = true;
            continue;
        } else {
            continue;
        }
        out->push_back(b);
        any = true;
    }
    return any;
}

} // namespace

bool chartBlocks(const QString &ref, int ownSheet, const ChartCells &cells, QVector<ChartBlock> *out)
{
    out->clear();
    Walk walk;
    return blocksOf(ref, ownSheet, cells, 0, &walk, out);
}

bool readChartRef(const QString &ref, int ownSheet, const ChartCells &cells, int maxCells, ChartRefData *out, bool wantText)
{
    *out = ChartRefData();
    QVector<ChartBlock> blocks;
    if (!chartBlocks(ref, ownSheet, cells, &blocks)) return false;
    bool first = true;
    for (const ChartBlock &b : std::as_const(blocks)) {
        CellRange r = b.range;
        // a block of more cells than may be read (a whole column, or a range far beyond the data) is read as far as the sheet's cells go
        if (r.cellCount() > maxCells) {
            quint32 rows = 0, cols = 0;
            cells.extent(b.sheet, &rows, &cols);
            r.row2 = std::min(r.row2, std::max(r.row1, rows > 0 ? rows - 1 : 0));
            r.col2 = std::min(r.col2, std::max(r.col1, cols > 0 ? cols - 1 : 0));
        }
        if (first) {
            out->rows = qint64(r.row2) - r.row1 + 1;
            out->cols = qint64(r.col2) - r.col1 + 1;
            first = false;
        }
        // (a hidden row or column is asked about once)
        QVector<bool> hiddenCols;
        for (quint32 c = r.col1; c <= r.col2 && hiddenCols.size() < maxCells; ++c) hiddenCols.push_back(cells.columnHidden(b.sheet, c));
        for (quint32 row = r.row1; row <= r.row2; ++row) {
            const bool hiddenRow = cells.rowHidden(b.sheet, row);
            for (quint32 col = r.col1; col <= r.col2; ++col) {
                if (out->points.size() >= maxCells) {
                    out->cut = true;
                    return true;
                }
                ChartPoint p;
                const jp::calc::Value v = cells.value(b.sheet, row, col);
                p.hidden = hiddenRow || (col - r.col1 < quint32(hiddenCols.size()) && hiddenCols[int(col - r.col1)]);
                switch (v.kind()) {
                case jp::calc::Value::Kind::Empty:
                case jp::calc::Value::Kind::Array: break;
                case jp::calc::Value::Kind::Number:
                case jp::calc::Value::Kind::Bool:
                    p.empty = false;
                    p.isNumber = true;
                    p.number = v.number();
                    if (wantText) p.text = cells.text(b.sheet, row, col);
                    break;
                case jp::calc::Value::Kind::Text:
                    p.empty = false;
                    if (wantText) p.text = cells.text(b.sheet, row, col);
                    break;
                case jp::calc::Value::Kind::Error:
                    p.empty = false;
                    p.isError = true;
                    if (wantText) p.text = cells.text(b.sheet, row, col);
                    break;
                }
                out->points.push_back(p);
            }
        }
    }
    return true;
}

namespace {

void addReads(const QString &ref, int ownSheet, const ChartCells &cells, QVector<ChartBlock> *out)
{
    if (ref.isEmpty()) return;
    QVector<ChartBlock> blocks;
    chartBlocks(ref, ownSheet, cells, &blocks);
    *out += blocks;
}

// The text of the cells a name or title reads, joined by spaces.
bool textOf(const QString &ref, int ownSheet, const ChartCells &cells, int *budget, QString *out)
{
    ChartRefData d;
    if (ref.isEmpty() || !readChartRef(ref, ownSheet, cells, std::min(*budget, 256), &d)) return false;
    *budget -= int(d.points.size());
    QStringList parts;
    for (const ChartPoint &p : std::as_const(d.points))
        if (!p.empty && !p.text.isEmpty()) parts << p.text;
    *out = parts.join(QLatin1Char(' ')).left(kChartMaxLabel);
    return true;
}

} // namespace

QVector<ChartBlock> chartReads(const Chart &chart, int ownSheet, const ChartCells &cells, bool *cut)
{
    QVector<ChartBlock> out;
    QHash<int, int> overflow;   // by sheet, the place in `out` of the block that holds what is past the limit
    bool over = false;
    auto add = [&](const QString &ref) {
        if (ref.isEmpty()) return;
        QVector<ChartBlock> blocks;
        chartBlocks(ref, ownSheet, cells, &blocks);
        for (const ChartBlock &b : std::as_const(blocks)) {
            if (out.size() < kMaxChartReadBlocks) {
                out.push_back(b);
                continue;
            }
            // past the limit a block is held in the hull of those past it on its sheet (one block for each sheet, and the sheets are few): what it reads is still followed
            over = true;
            const auto it = overflow.find(b.sheet);
            if (it == overflow.end()) {
                overflow.insert(b.sheet, int(out.size()));
                out.push_back(b);
                continue;
            }
            CellRange &r = out[it.value()].range;
            r.row1 = std::min(r.row1, b.range.row1);
            r.col1 = std::min(r.col1, b.range.col1);
            r.row2 = std::max(r.row2, b.range.row2);
            r.col2 = std::max(r.col2, b.range.col2);
        }
    };
    for (const ChartPlot &plot : chart.plots)
        for (const ChartSeries &s : plot.series) {
            add(s.nameRef);
            add(s.categoryRef);
            add(s.valueRef);
            add(s.xRef);
        }
    add(chart.title.ref);
    for (const ChartAxis &a : chart.axes) add(a.title.ref);
    if (cut) *cut = over;
    return out;
}

void ChartReadIndex::Tree::build(std::vector<CellRange> list, bool rows)
{
    byRow = rows;
    blocks = std::move(list);
    std::stable_sort(blocks.begin(), blocks.end(), [rows](const CellRange &a, const CellRange &b) { return rows ? a.row1 < b.row1 : a.col1 < b.col1; });
    reach.assign(blocks.size(), 0);
    fill(0, blocks.size());
}

quint32 ChartReadIndex::Tree::fill(size_t lo, size_t hi)
{
    if (lo >= hi) return 0;
    const size_t mid = lo + (hi - lo) / 2;
    quint32 far = byRow ? blocks[mid].row2 : blocks[mid].col2;
    far = std::max(far, fill(lo, mid));
    far = std::max(far, fill(mid + 1, hi));
    reach[mid] = far;
    return far;
}

bool ChartReadIndex::Tree::stab(size_t lo, size_t hi, quint32 along, quint32 across, qint64 *probes) const
{
    while (lo < hi) {
        const size_t mid = lo + (hi - lo) / 2;
        if (reach[mid] < along) return false;   // nothing in this stretch reaches as far
        ++*probes;
        if (stab(lo, mid, along, across, probes)) return true;
        const CellRange &b = blocks[mid];
        if ((byRow ? b.row1 : b.col1) > along) return false;   // this one and all after it begin past the place
        if ((byRow ? b.row2 : b.col2) >= along && (byRow ? (across >= b.col1 && across <= b.col2) : (across >= b.row1 && across <= b.row2))) return true;
        lo = mid + 1;
    }
    return false;
}

ChartReadIndex::ChartReadIndex(const QVector<ChartBlock> &blocks)
{
    std::vector<ChartBlock> sorted(blocks.begin(), blocks.end());
    std::stable_sort(sorted.begin(), sorted.end(), [](const ChartBlock &a, const ChartBlock &b) { return a.sheet < b.sheet; });
    for (size_t from = 0; from < sorted.size();) {
        size_t to = from;
        while (to < sorted.size() && sorted[to].sheet == sorted[from].sheet) ++to;
        Sheet s;
        s.sheet = sorted[from].sheet;
        s.hull = sorted[from].range;
        s.hull.valid = true;
        std::vector<CellRange> rows, columns;
        for (size_t i = from; i < to; ++i) {
            const CellRange &r = sorted[i].range;
            s.hull.row1 = std::min(s.hull.row1, r.row1);
            s.hull.col1 = std::min(s.hull.col1, r.col1);
            s.hull.row2 = std::max(s.hull.row2, r.row2);
            s.hull.col2 = std::max(s.hull.col2, r.col2);
            // (a block is kept in the tree of the direction it is shorter in: a whole column among small blocks is one place among the columns, not a reach down all the rows)
            (qint64(r.row2) - r.row1 >= qint64(r.col2) - r.col1 ? columns : rows).push_back(r);
            ++m_count;
        }
        s.byRow.build(std::move(rows), true);
        s.byColumn.build(std::move(columns), false);
        m_sheets.push_back(std::move(s));
        from = to;
    }
}

const ChartReadIndex::Sheet *ChartReadIndex::find(int sheet) const
{
    const auto it = std::lower_bound(m_sheets.begin(), m_sheets.end(), sheet, [](const Sheet &s, int v) { return s.sheet < v; });
    return it != m_sheets.end() && it->sheet == sheet ? &*it : nullptr;
}

CellRange ChartReadIndex::hull(int sheet) const
{
    const Sheet *s = find(sheet);
    return s ? s->hull : CellRange();
}

bool ChartReadIndex::reads(int sheet, quint32 row, quint32 col) const
{
    const Sheet *s = find(sheet);
    if (!s || !s->hull.contains(row, col)) return false;
    return s->byRow.stab(0, s->byRow.blocks.size(), row, col, &m_probes) || s->byColumn.stab(0, s->byColumn.blocks.size(), col, row, &m_probes);
}

namespace {

QString movedRef(const QString &ref, const QStringList &sheetNames, const std::vector<StructEdit> &edits, int ownSheet)
{
    QString f = ref;
    for (const StructEdit &e : edits) f = adjustFormula(f, ownSheet, sheetNames, e, nullptr, true);
    return f;
}

} // namespace

QStringList chartRefs(const Chart &chart)
{
    QStringList out;
    for (const ChartPlot &plot : chart.plots)
        for (const ChartSeries &s : plot.series) out << s.nameRef << s.categoryRef << s.valueRef << s.xRef;
    out << chart.title.ref;
    for (const ChartAxis &a : chart.axes) out << a.title.ref;
    return out;
}

void setChartRefs(Chart *chart, const QStringList &refs)
{
    int at = 0;
    auto next = [&]() { return at < refs.size() ? refs[at++] : (++at, QString()); };
    for (ChartPlot &plot : chart->plots)
        for (ChartSeries &s : plot.series) {
            s.nameRef = next();
            s.categoryRef = next();
            s.valueRef = next();
            s.xRef = next();
        }
    chart->title.ref = next();
    for (ChartAxis &a : chart->axes) a.title.ref = next();
}

QString movedChartRef(const QString &ref, const QStringList &sheetNames, const std::vector<StructEdit> &edits, int ownSheet) { return ref.isEmpty() ? ref : movedRef(ref, sheetNames, edits, ownSheet); }

void moveChartRefs(Chart *chart, const QStringList &sheetNames, const std::vector<StructEdit> &edits, int ownSheet)
{
    if (edits.empty()) return;
    for (ChartPlot &plot : chart->plots)
        for (ChartSeries &s : plot.series)
            for (QString *ref : {&s.nameRef, &s.categoryRef, &s.valueRef, &s.xRef})
                if (!ref->isEmpty()) *ref = movedRef(*ref, sheetNames, edits, ownSheet);
    if (!chart->title.ref.isEmpty()) chart->title.ref = movedRef(chart->title.ref, sheetNames, edits, ownSheet);
    for (ChartAxis &a : chart->axes)
        if (!a.title.ref.isEmpty()) a.title.ref = movedRef(a.title.ref, sheetNames, edits, ownSheet);
}

int applyChartCells(Chart *chart, int ownSheet, const ChartCells &cells)
{
    int budget = kMaxChartCells;
    int read = 0, position = 0, count = 0;
    QString onlyName;
    for (ChartPlot &plot : chart->plots) {
        const bool scatter = plot.type == ChartType::Scatter;
        for (ChartSeries &s : plot.series) {
            ++position;
            ++count;
            // the name: the text of its cells
            QString name;
            if (textOf(s.nameRef, ownSheet, cells, &budget, &name)) s.name = name.isEmpty() ? QCoreApplication::translate("Chart", "Series%1").arg(position) : name;
            onlyName = s.name;
            ChartRefData d;
            const int most = std::max(0, std::min(kMaxChartSeriesCells, budget));
            // the values
            if (!s.valueRef.isEmpty() && readChartRef(s.valueRef, ownSheet, cells, most, &d, false)) {
                budget -= int(d.points.size());
                QVector<double> values;
                values.reserve(int(d.points.size()));
                for (const ChartPoint &p : std::as_const(d.points)) {
                    if (p.hidden && chart->plotVisibleOnly) continue;
                    values.push_back(p.empty ? kNaN : p.isNumber ? p.number : p.isError ? kNaN : 0.0);
                }
                s.values = values;
                ++read;
            }
            // the categories, or the x values of a scatter
            const QString &catRef = scatter ? s.xRef : s.categoryRef;
            if (!catRef.isEmpty() && readChartRef(catRef, ownSheet, cells, std::max(0, std::min(kMaxChartSeriesCells, budget)), &d)) {
                budget -= int(d.points.size());
                QStringList labels;
                QVector<double> xs;
                bool allNumbers = true;
                // numbers as categories (dates, say) are shown through the category axis's own format when it is not linked to the source's, else as the cells show them
                const ChartAxis *axis = chart->axis(plot.axisIds[0]);
                const bool byAxis = !scatter && s.categoryRefNumeric && axis && !axis->formatLinked && (axis->kind == ChartAxis::Category || axis->kind == ChartAxis::Date);
                for (const ChartPoint &p : std::as_const(d.points)) {
                    if (p.hidden && chart->plotVisibleOnly) continue;
                    labels << (byAxis && p.isNumber ? formatChartNumber(p.number, axis->format) : p.text).left(kChartMaxLabel);
                    xs.push_back(p.empty ? kNaN : p.isNumber ? p.number : kNaN);
                    if (!p.empty && !p.isNumber) allNumbers = false;
                }
                if (scatter) {
                    // a text for x makes every point's x its place
                    if (!allNumbers)
                        for (int i = 0; i < xs.size(); ++i) xs[i] = i + 1;
                    s.xValues = xs;
                    s.categories = allNumbers ? QStringList() : labels;
                } else {
                    s.categories = labels;
                    s.categoryNumbers = s.categoryRefNumeric && allNumbers ? xs : QVector<double>();
                }
            }
        }
    }
    QString text;
    if (textOf(chart->title.ref, ownSheet, cells, &budget, &text)) chart->title.text = text;
    else if (chart->title.automatic && count == 1) chart->title.text = onlyName;
    for (ChartAxis &a : chart->axes)
        if (textOf(a.title.ref, ownSheet, cells, &budget, &text)) a.title.text = text;
    return read;
}

void applyChartFormats(Chart *chart, int ownSheet, const ChartCells &cells)
{
    auto codeOf = [&](const QString &ref) {
        QVector<ChartBlock> blocks;
        if (ref.isEmpty() || !chartBlocks(ref, ownSheet, cells, &blocks) || blocks.isEmpty()) return QString();
        const ChartBlock &b = blocks.first();
        return cells.formatCode(b.sheet, b.range.row1, b.range.col1);
    };
    for (ChartPlot &plot : chart->plots)
        for (ChartSeries &s : plot.series) {
            if (s.format.isEmpty() || s.format.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0) {
                const QString code = codeOf(s.valueRef);
                if (!code.isEmpty()) s.format = code;
            }
            if (plot.type == ChartType::Scatter && !s.xRef.isEmpty() && (s.categoryFormat.isEmpty() || s.categoryFormat.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0)) {
                const QString code = codeOf(s.xRef);
                if (!code.isEmpty()) s.categoryFormat = code;
            }
        }
}

} // namespace jp::office::sheet
