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

bool blocksOf(const QString &ref, int ownSheet, const ChartCells &cells, int depth, QVector<ChartBlock> *out)
{
    static const QRegularExpression columnsRe(QStringLiteral("^\\$?([A-Za-z]{1,3}):\\$?([A-Za-z]{1,3})$")), rowsRe(QStringLiteral("^\\$?(\\d{1,7}):\\$?(\\d{1,7})$")),
        nameRe(QStringLiteral("^[A-Za-z_\\\\][A-Za-z0-9_.\\\\]*$"));
    QString text = ref.trimmed();
    if (text.startsWith(QLatin1Char('='))) text = text.mid(1).trimmed();
    if (text.isEmpty()) return false;
    bool any = false;
    for (const QString &area : splitAreas(withoutOuterParentheses(text))) {
        if (out->size() >= kMaxChartBlocks) break;
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
            // a name: what it stands for is read as a reference in its turn, a few levels down at most
            if (depth >= kMaxChartNameDepth) continue;
            const QString stands = cells.definedName(tail, sheet);
            if (stands.isEmpty() || !blocksOf(stands, sheet, cells, depth + 1, out)) continue;
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
    return blocksOf(ref, ownSheet, cells, 0, out);
}

bool readChartRef(const QString &ref, int ownSheet, const ChartCells &cells, int maxCells, ChartRefData *out)
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
                    p.text = cells.text(b.sheet, row, col);
                    break;
                case jp::calc::Value::Kind::Text:
                    p.empty = false;
                    p.text = cells.text(b.sheet, row, col);
                    break;
                case jp::calc::Value::Kind::Error:
                    p.empty = false;
                    p.isError = true;
                    p.text = cells.text(b.sheet, row, col);
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

QVector<ChartBlock> chartReads(const Chart &chart, int ownSheet, const ChartCells &cells)
{
    QVector<ChartBlock> out;
    for (const ChartPlot &plot : chart.plots)
        for (const ChartSeries &s : plot.series) {
            addReads(s.nameRef, ownSheet, cells, &out);
            addReads(s.categoryRef, ownSheet, cells, &out);
            addReads(s.valueRef, ownSheet, cells, &out);
            addReads(s.xRef, ownSheet, cells, &out);
        }
    addReads(chart.title.ref, ownSheet, cells, &out);
    for (const ChartAxis &a : chart.axes) addReads(a.title.ref, ownSheet, cells, &out);
    return out;
}

namespace {

QString movedRef(const QString &ref, const QStringList &sheetNames, const std::vector<StructEdit> &edits)
{
    QString f = ref;
    for (const StructEdit &e : edits) f = adjustFormula(f, 0, sheetNames, e, nullptr, true);
    return f;
}

} // namespace

void moveChartRefs(Chart *chart, const QStringList &sheetNames, const std::vector<StructEdit> &edits)
{
    if (edits.empty()) return;
    for (ChartPlot &plot : chart->plots)
        for (ChartSeries &s : plot.series)
            for (QString *ref : {&s.nameRef, &s.categoryRef, &s.valueRef, &s.xRef})
                if (!ref->isEmpty()) *ref = movedRef(*ref, sheetNames, edits);
    if (!chart->title.ref.isEmpty()) chart->title.ref = movedRef(chart->title.ref, sheetNames, edits);
    for (ChartAxis &a : chart->axes)
        if (!a.title.ref.isEmpty()) a.title.ref = movedRef(a.title.ref, sheetNames, edits);
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
            if (!s.valueRef.isEmpty() && readChartRef(s.valueRef, ownSheet, cells, most, &d)) {
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
                for (const ChartPoint &p : std::as_const(d.points)) {
                    if (p.hidden && chart->plotVisibleOnly) continue;
                    labels << p.text.left(kChartMaxLabel);
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

} // namespace jp::office::sheet
