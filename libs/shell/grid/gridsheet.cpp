#include "grid/gridsheet.h"

#include "sheet/cellref.h"

#include <algorithm>
#include <cmath>

namespace jp::grid {

using jp::office::sheet::CellType;
using jp::office::sheet::ColumnSpan;
using jp::office::sheet::kMaxColumns;
using jp::office::sheet::RowSpec;
using jp::office::sheet::SheetView;

namespace {

// Seven for every character: what text is measured with until the view gives its fonts.
class PlainMeter : public TextMeter {
public:
    double width(const QString &text, const CellLook &) const override { return 7.0 * text.size(); }
};

const PlainMeter kPlainMeter;

} // namespace

GridSheet::GridSheet() : m_meter(&kPlainMeter) {}

void GridSheet::setMeter(const TextMeter *meter)
{
    m_meter = meter ? meter : &kPlainMeter;
    m_texts.clear();
}

int GridSheet::columnPixels(double chars, int maxDigitWidth)
{
    if (!(chars > 0) || maxDigitWidth <= 0) return 0;
    // as the format says: the width in characters includes the padding, and the pixels are what that comes to in the widest digit, cut
    return int(((256.0 * chars + int(128 / maxDigitWidth)) / 256.0) * maxDigitWidth);
}

int GridSheet::rowPixels(double points) { return points > 0 ? int(std::lround(points * 96.0 / 72.0)) : 0; }

double GridSheet::widthForPixels(int pixels, int maxDigitWidth)
{
    if (pixels <= 0 || maxDigitWidth <= 0) return 0;
    return std::floor(double(pixels) / maxDigitWidth * 256.0) / 256.0;
}

double GridSheet::pointsForPixels(int pixels) { return pixels > 0 ? std::round(pixels * 72.0 / 96.0 * 100.0) / 100.0 : 0; }

double GridSheet::shownWidth(double fileWidth, int maxDigitWidth)
{
    if (maxDigitWidth <= 0) return fileWidth;
    return std::max(0.0, std::round((fileWidth - 5.0 / maxDigitWidth) * 100.0) / 100.0);
}

double GridSheet::widthOfShown(double shown, int maxDigitWidth)
{
    if (maxDigitWidth <= 0) return shown;
    return std::floor((shown * maxDigitWidth + 5.0) / maxDigitWidth * 256.0) / 256.0;
}

namespace {

// The width in pixels a cell's text needs, in a column wide enough for any text.
double neededPixels(const Shown &s, const CellLook &look) { return s.width + 2 * kCellPadding + (look.indent > 0 ? look.indent * 9.0 : 0.0); }

} // namespace

double GridSheet::fitWidthOfCell(quint32 row, quint32 col) const
{
    const Cell *cell = cellAt(row, col);
    if (!cell) return 0;
    const CellLook &look = m_looks.look(cell->style);
    const Shown s = showValue(valueOf(*cell), look, m_data.dateSystem, 1e9, m_looks.maxDigitWidth(), *m_meter);
    if (s.kind == Kind::Empty || s.width <= 0) return 0;
    return widthForPixels(int(std::ceil(neededPixels(s, look))), m_looks.maxDigitWidth());
}

double GridSheet::fitWidth(quint32 col, qint64 limit) const
{
    if (!m_data.sheet) return 0;
    double widest = 0;
    qint64 seen = 0;
    m_data.sheet->cells.forEachInRange(0, col, jp::office::sheet::kMaxRows - 1, col, [&](const Cell &cell) {
        if (++seen > limit) return false;
        if (cell.type == jp::office::sheet::CellType::Empty && !cell.hasFormula()) return true;
        if (m_layout.merges.at(cell.row, cell.col) >= 0) return true;   // (a merged cell does not decide)
        const CellLook &look = m_looks.look(cell.style);
        if (look.wrap) return true;
        const Shown s = showValue(valueOf(cell), look, m_data.dateSystem, 1e9, m_looks.maxDigitWidth(), *m_meter);
        if (s.kind != Kind::Empty && s.width > 0) widest = std::max(widest, neededPixels(s, look));
        return true;
    });
    return widest > 0 ? widthForPixels(int(std::ceil(widest)), m_looks.maxDigitWidth()) : 0;
}

double GridSheet::fitHeight(quint32 row, qint64 limit) const
{
    if (!m_data.sheet) return 0;
    double tallest = 0;
    qint64 seen = 0;
    m_data.sheet->cells.forEachInRange(row, 0, row, jp::office::sheet::kMaxColumns - 1, [&](const Cell &cell) {
        if (++seen > limit) return false;
        if (cell.type == jp::office::sheet::CellType::Empty && !cell.hasFormula()) return true;
        if (m_layout.merges.at(cell.row, cell.col) >= 0) return true;
        const CellLook &look = m_looks.look(cell.style);
        const Shown s = showValue(valueOf(cell), look, m_data.dateSystem, columnWidth(cell.col), m_looks.maxDigitWidth(), *m_meter);
        if (s.kind == Kind::Empty) return true;
        int lines = 1;
        if (look.wrap) {
            const QString text = textOf(cell);
            lines = int(text.count(QLatin1Char('\n'))) + 1;
            const double room = std::max(1.0, textRoom(columnWidth(cell.col), look));
            double longest = 0;
            for (const QString &line : text.split(QLatin1Char('\n'))) longest = std::max(longest, m_meter->width(line, look));
            lines = std::max(lines, int(std::ceil(longest / room)) + (text.contains(QLatin1Char('\n')) ? 0 : 0));
        }
        tallest = std::max(tallest, lines * m_meter->lineHeight(look) + 3);
        return true;
    });
    return tallest > 0 ? pointsForPixels(int(std::ceil(tallest))) : 0;
}

void GridSheet::setData(const GridData &data)
{
    m_data = data;
    m_looks.setStyles(data.styles);
    m_texts.clear();
    std::shared_ptr<const GridGeometry> g = data.geometry;
    if (!g) g = (data.sheet && data.styles) ? makeGeometry(*data.sheet, *data.styles) : std::make_shared<GridGeometry>();
    m_layout = g->layout;   // (a copy that shares the sizes and merges with the geometry's until one is changed)
    m_geometry = g;
}

std::shared_ptr<GridGeometry> GridSheet::makeGeometry(const Sheet &sheet, const Styles &styles)
{
    auto geometry = std::make_shared<GridGeometry>();
    GridLayout &layout = geometry->layout;
    const int mdw = LookTable(&styles).maxDigitWidth();
    // ---- columns: the sheet's default, then each run (sorted; a run that overlaps one before it keeps what that one left)
    int defaultCol = 0;
    if (sheet.format.defaultColWidth > 0) defaultCol = columnPixels(sheet.format.defaultColWidth, mdw);
    if (defaultCol <= 0) defaultCol = (sheet.format.baseColWidth * mdw + 5 + 7) / 8 * 8;
    layout.cols.setCount(qint64(kMaxColumns));
    layout.cols.setDefaultSize(std::max(1, defaultCol));
    std::vector<ColumnSpan> runs(sheet.columns.begin(), sheet.columns.end());
    std::stable_sort(runs.begin(), runs.end(), [](const ColumnSpan &a, const ColumnSpan &b) { return a.first < b.first; });
    std::vector<Axis::Place> places;
    quint32 next = 0;
    for (const ColumnSpan &run : runs) {
        const quint32 first = std::max(run.first, next);
        if (first > run.last || first >= kMaxColumns) continue;
        const quint32 last = std::min(run.last, kMaxColumns - 1);
        next = last + 1;
        const int size = run.hidden ? 0 : run.width >= 0 ? columnPixels(run.width, mdw) : 0;
        if (run.hidden || run.width >= 0)
            for (quint32 c = first; c <= last; ++c) places.push_back({qint64(c), size});
        if (run.style != 0) {
            ColumnSpan kept = run;
            kept.first = first;
            kept.last = last;
            geometry->columnStyles.push_back(kept);
        }
    }
    layout.cols.assign(std::move(places));
    // ---- rows
    const int defaultRow = std::max(1, rowPixels(sheet.format.defaultRowHeight > 0 ? sheet.format.defaultRowHeight : 15));
    layout.rows.setCount(qint64(jp::office::sheet::kMaxRows));
    layout.rows.setDefaultSize(defaultRow);
    std::vector<RowSpec> specs(sheet.rowSpecs.begin(), sheet.rowSpecs.end());
    std::stable_sort(specs.begin(), specs.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
    // (a row the file gives twice has its last)
    std::vector<RowSpec> unique;
    for (const RowSpec &s : specs) {
        if (!unique.empty() && unique.back().row == s.row) unique.back() = s;
        else unique.push_back(s);
    }
    auto sizeOf = [&](const RowSpec &s) { return s.hidden ? 0 : s.height >= 0 ? rowPixels(s.height) : defaultRow; };
    places.clear();
    if (sheet.format.zeroHeight && !unique.empty()) {
        // the rows the file does not list are hidden, and the sheet ends at its last listed row
        const qint64 last = qint64(unique.back().row);
        layout.rows.setCount(last + 1);
        size_t at = 0;
        for (qint64 r = 0; r <= last; ++r) {
            if (at < unique.size() && qint64(unique[at].row) == r) places.push_back({r, sizeOf(unique[at++])});
            else places.push_back({r, 0});
        }
    } else {
        for (const RowSpec &s : unique) places.push_back({qint64(s.row), sizeOf(s)});
    }
    layout.rows.assign(std::move(places));
    for (const RowSpec &s : unique)
        if (s.customFormat && s.style != 0) geometry->rowStyles.push_back(s);
    // ---- merged ranges, frozen panes, zoom, and where it was scrolled to
    layout.merges.assign(sheet.merges);
    const SheetView &v = sheet.view;
    layout.setZoom(v.zoom / 100.0);
    layout.setHeaders(v.showHeadings, 40, 20);
    if (v.frozen) {
        // (a frozen part may begin below the first row, or right of the first column: the view's top left is where it begins, and what is before it is out of sight)
        const qint64 skipRows = v.frozenRows > 0 ? qint64(v.topRow) : 0, skipCols = v.frozenCols > 0 ? qint64(v.leftCol) : 0;
        layout.setSkipped(skipRows, skipCols);
        layout.setFrozen(skipRows + qint64(v.frozenRows), skipCols + qint64(v.frozenCols));
        layout.setScroll(layout.cols.position(qint64(v.paneCol)) - layout.cols.position(layout.frozenCols()), layout.rows.position(qint64(v.paneRow)) - layout.rows.position(layout.frozenRows()));
    } else {
        layout.setScroll(layout.cols.position(qint64(v.leftCol)), layout.rows.position(qint64(v.topRow)));
    }
    // (what a search would build on the first look is built now, by whoever makes it, and goes with the copy)
    layout.rows.total();
    layout.cols.total();
    return geometry;
}

const Cell *GridSheet::cellAt(quint32 row, quint32 col) const { return m_data.sheet && col < kMaxColumns ? m_data.sheet->cells.find(row, col) : nullptr; }

jp::calc::Value GridSheet::valueOf(const Cell &cell) const
{
    using jp::calc::Value;
    if (!m_data.sheet) return Value();
    if (m_data.values && (cell.hasFormula() || (cell.flags & jp::office::sheet::ArrayMember))) {
        Value v;
        if (m_data.values->find(m_data.sheetIndex, cell.row, cell.col, &v)) return v;
    } else if (m_data.calc) {
        return m_data.calc->shown(m_data.sheetIndex, cell.row, cell.col);
    }
    switch (cell.type) {
    case CellType::Empty: return Value();
    case CellType::Number:
    case CellType::Date: return Value::number(cell.number);
    case CellType::SharedString: return Value::text(m_data.strings ? m_data.strings->at(cell.textIndex()) : QString());
    case CellType::InlineString:
    case CellType::FormulaString: return Value::text(m_data.sheet->texts.value(cell.textIndex()));
    case CellType::Bool: return Value::boolean(cell.number != 0);
    case CellType::Error: return Value::error(jp::calc::ErrorKind(std::max(0, std::min(int(jp::calc::ErrorKind::Unknown), int(cell.number)))));
    }
    return Value();
}

quint32 GridSheet::styleOf(quint32 row, quint32 col, const Cell *cell) const
{
    if (cell || !m_geometry) return cell ? cell->style : 0;
    const std::vector<RowSpec> &rowStyles = m_geometry->rowStyles;
    const std::vector<ColumnSpan> &columnStyles = m_geometry->columnStyles;
    const auto r = std::lower_bound(rowStyles.begin(), rowStyles.end(), row, [](const RowSpec &s, quint32 v) { return s.row < v; });
    if (r != rowStyles.end() && r->row == row) return r->style;
    auto c = std::upper_bound(columnStyles.begin(), columnStyles.end(), col, [](quint32 v, const ColumnSpan &s) { return v < s.first; });
    if (c != columnStyles.begin()) {
        --c;
        if (col <= c->last) return c->style;
    }
    return 0;
}

double GridSheet::columnWidth(qint64 col) const { return m_layout.cols.size(col); }

Shown GridSheet::shown(quint32 row, quint32 col, const Cell &cell) const
{
    const quint64 key = (quint64(row) << 16) | col;
    const auto found = m_texts.find(key);
    if (found != m_texts.end()) return found->second;
    ++m_textsMade;
    double width = columnWidth(col);
    const int m = m_layout.merges.at(row, col);
    if (m >= 0) {
        const jp::office::sheet::CellRange &r = m_layout.merges.range(m);
        width = 0;
        // (a merge of many columns is counted to the most the grid ever shows)
        for (quint32 c = r.col1; c <= r.col2 && c < r.col1 + 1024; ++c) width += columnWidth(c);
    }
    Shown s;
    if (m_showFormulas && cell.hasFormula() && m_data.sheet && !m_data.sheet->formulaText(cell).isEmpty())
        s = showValue(jp::calc::Value::text(QLatin1Char('=') + displayFormula(m_data.sheet->formulaText(cell))), m_looks.look(cell.style), m_data.dateSystem, width, m_looks.maxDigitWidth(), *m_meter);
    else
        s = showValue(valueOf(cell), m_looks.look(cell.style), m_data.dateSystem, width, m_looks.maxDigitWidth(), *m_meter);
    if (m_texts.size() >= kMaxTexts) m_texts.clear();
    m_texts.emplace(key, s);
    return s;
}

QString GridSheet::textOf(const Cell &cell) const
{
    using jp::calc::Value;
    const Value v = valueOf(cell);
    const CellLook &look = m_looks.look(cell.style);
    switch (v.kind()) {
    case Value::Kind::Text: {
        if (look.hasFormat) {
            const jp::calc::Formatted f = look.format.formatText(v.text());
            if (!f.failed) return f.text;
        }
        return v.text();
    }
    case Value::Kind::Number: {
        const jp::calc::Formatted f = look.format.format(v.number(), m_data.dateSystem, 0);
        return f.failed ? jp::calc::numberToText(v.number()) : f.text;
    }
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return jp::calc::errorKindText(v.errorKind());
    default: return QString();
    }
}

void GridSheet::invalidateTexts() const { m_texts.clear(); }

void GridSheet::setShowFormulas(bool on)
{
    if (on == m_showFormulas) return;
    m_showFormulas = on;
    m_texts.clear();
}

void GridSheet::usedExtent(quint32 *rows, quint32 *cols) const
{
    *rows = m_data.sheet ? m_data.sheet->extentRows : 0;
    *cols = m_data.sheet ? m_data.sheet->extentCols : 0;
}

} // namespace jp::grid
