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

void GridSheet::setData(const GridData &data)
{
    m_data = data;
    m_looks.setStyles(data.styles);
    m_texts.clear();
    m_columnStyles.clear();
    m_rowStyles.clear();
    buildGeometry();
}

void GridSheet::buildGeometry()
{
    m_layout = GridLayout();
    if (!m_data.sheet) return;
    const Sheet &sheet = *m_data.sheet;
    const int mdw = m_looks.maxDigitWidth();
    // ---- columns: the sheet's default, then each run (sorted; a run that overlaps one before it keeps what that one left)
    int defaultCol = 0;
    if (sheet.format.defaultColWidth > 0) defaultCol = columnPixels(sheet.format.defaultColWidth, mdw);
    if (defaultCol <= 0) defaultCol = (sheet.format.baseColWidth * mdw + 5 + 7) / 8 * 8;
    m_layout.cols.setCount(qint64(kMaxColumns));
    m_layout.cols.setDefaultSize(std::max(1, defaultCol));
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
            m_columnStyles.push_back(kept);
        }
    }
    m_layout.cols.assign(std::move(places));
    // ---- rows
    const int defaultRow = std::max(1, rowPixels(sheet.format.defaultRowHeight > 0 ? sheet.format.defaultRowHeight : 15));
    m_layout.rows.setCount(qint64(jp::office::sheet::kMaxRows));
    m_layout.rows.setDefaultSize(defaultRow);
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
        m_layout.rows.setCount(last + 1);
        size_t at = 0;
        for (qint64 r = 0; r <= last; ++r) {
            if (at < unique.size() && qint64(unique[at].row) == r) places.push_back({r, sizeOf(unique[at++])});
            else places.push_back({r, 0});
        }
    } else {
        for (const RowSpec &s : unique) places.push_back({qint64(s.row), sizeOf(s)});
    }
    m_layout.rows.assign(std::move(places));
    for (const RowSpec &s : unique)
        if (s.customFormat && s.style != 0) m_rowStyles.push_back(s);
    // ---- merged ranges, frozen panes, zoom, and where it was scrolled to
    m_layout.merges.assign(sheet.merges);
    const SheetView &v = sheet.view;
    m_layout.setZoom(v.zoom / 100.0);
    m_layout.setHeaders(v.showHeadings, 40, 20);
    if (v.frozen) {
        m_layout.setFrozen(qint64(v.frozenRows), qint64(v.frozenCols));
        m_layout.setScroll(m_layout.cols.position(qint64(v.paneCol)) - m_layout.cols.position(m_layout.frozenCols()),
                           m_layout.rows.position(qint64(v.paneRow)) - m_layout.rows.position(m_layout.frozenRows()));
    } else {
        m_layout.setScroll(m_layout.cols.position(qint64(v.leftCol)), m_layout.rows.position(qint64(v.topRow)));
    }
}

const Cell *GridSheet::cellAt(quint32 row, quint32 col) const { return m_data.sheet && col < kMaxColumns ? m_data.sheet->cells.find(row, col) : nullptr; }

jp::calc::Value GridSheet::valueOf(const Cell &cell) const
{
    using jp::calc::Value;
    if (!m_data.sheet) return Value();
    if (m_data.calc) return m_data.calc->shown(m_data.sheetIndex, cell.row, cell.col);
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
    if (cell) return cell->style;
    const auto r = std::lower_bound(m_rowStyles.begin(), m_rowStyles.end(), row, [](const RowSpec &s, quint32 v) { return s.row < v; });
    if (r != m_rowStyles.end() && r->row == row) return r->style;
    auto c = std::upper_bound(m_columnStyles.begin(), m_columnStyles.end(), col, [](quint32 v, const ColumnSpan &s) { return v < s.first; });
    if (c != m_columnStyles.begin()) {
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
    const Shown s = showValue(valueOf(cell), m_looks.look(cell.style), m_data.dateSystem, width, m_looks.maxDigitWidth(), *m_meter);
    if (m_texts.size() >= kMaxTexts) m_texts.clear();
    m_texts.emplace(key, s);
    return s;
}

void GridSheet::invalidateTexts() const { m_texts.clear(); }

void GridSheet::usedExtent(quint32 *rows, quint32 *cols) const
{
    *rows = m_data.sheet ? m_data.sheet->extentRows : 0;
    *cols = m_data.sheet ? m_data.sheet->extentCols : 0;
}

} // namespace jp::grid
