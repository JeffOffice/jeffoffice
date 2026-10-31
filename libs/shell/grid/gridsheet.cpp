#include "grid/gridsheet.h"

#include "sheet/cellref.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace jp::grid {

namespace {
std::atomic<qint64> g_viewCellsRebuilt{0};   // sizes put in layouts, and cached texts dropped
} // namespace

qint64 GridSheet::viewCellsRebuiltForTest() { return g_viewCellsRebuilt.load(std::memory_order_relaxed); }

namespace {
// The most characters of one cell's text that are measured for its height.
constexpr int kMaxMeasuredChars = 4000;
} // namespace

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
// (A number is shown in as many digits as the column's width holds by the digit width it is counted in, charsFor: the column must hold that many, or the number
// falls back to a shorter form, whatever its font's digits measure.)
double neededPixels(const Shown &s, const CellLook &look, int maxDigitWidth)
{
    double need = s.width + 2 * kCellPadding + (look.indent > 0 ? look.indent * 9.0 : 0.0);
    if (s.kind == Kind::Number && maxDigitWidth > 0) need = std::max(need, 5.0 + double(s.text.size()) * maxDigitWidth);
    return need;
}

} // namespace

double GridSheet::fitWidthOfCell(quint32 row, quint32 col) const
{
    const Cell *cell = cellAt(row, col);
    if (!cell) return 0;
    const CellLook &look = lookOf(row, col, cell->style);
    const Shown s = showValue(valueOf(row, col, *cell), look, m_data.dateSystem, 1e9, m_looks.maxDigitWidth(), *m_meter, &m_symbols);
    if (s.kind == Kind::Empty || s.width <= 0) return 0;
    return widthForPixels(int(std::ceil(neededPixels(s, look, m_looks.maxDigitWidth()))), m_looks.maxDigitWidth());
}

double GridSheet::fitWidth(quint32 col, qint64 limit) const
{
    if (!m_data.sheet) return 0;
    double widest = 0;
    qint64 seen = 0;
    m_data.sheet->cells.forEachInRange(0, col, jp::office::sheet::kMaxRows - 1, col, [&](quint32 cellRow, quint32 cellCol, const Cell &cell) {
        if (++seen > limit) return false;
        if (cell.type == jp::office::sheet::CellType::Empty && !cell.hasFormula()) return true;
        if (m_layout.merges.at(cellRow, cellCol) >= 0) return true;   // (a merged cell does not decide)
        const CellLook &look = lookOf(cellRow, cellCol, cell.style);
        if (look.wrap) return true;
        const Shown s = showValue(valueOf(cellRow, cellCol, cell), look, m_data.dateSystem, 1e9, m_looks.maxDigitWidth(), *m_meter, &m_symbols);
        if (s.kind != Kind::Empty && s.width > 0) widest = std::max(widest, neededPixels(s, look, m_looks.maxDigitWidth()));
        return true;
    });
    return widest > 0 ? widthForPixels(int(std::ceil(widest)), m_looks.maxDigitWidth()) : 0;
}

double GridSheet::fitHeight(quint32 row, qint64 limit) const
{
    if (!m_data.sheet) return 0;
    double tallest = 0;
    qint64 seen = 0;
    m_data.sheet->cells.forEachInRange(row, 0, row, jp::office::sheet::kMaxColumns - 1, [&](quint32 cellRow, quint32 cellCol, const Cell &cell) {
        if (++seen > limit) return false;
        if (cell.type == jp::office::sheet::CellType::Empty && !cell.hasFormula()) return true;
        if (m_layout.merges.at(cellRow, cellCol) >= 0) return true;
        const CellLook &look = lookOf(cellRow, cellCol, cell.style);
        const Shown s = showValue(valueOf(cellRow, cellCol, cell), look, m_data.dateSystem, columnWidth(cellCol), m_looks.maxDigitWidth(), *m_meter, &m_symbols);
        if (s.kind == Kind::Empty) return true;
        int lines = 1;
        if (look.wrap) {
            const QString text = textOf(cellRow, cellCol, cell);
            lines = int(text.count(QLatin1Char('\n'))) + 1;
            const double room = std::max(1.0, textRoom(columnWidth(cellCol), look));
            double longest = 0;
            for (const QString &line : text.split(QLatin1Char('\n'))) longest = std::max(longest, m_meter->width(line, look));
            lines = std::max(lines, int(std::ceil(longest / room)) + (text.contains(QLatin1Char('\n')) ? 0 : 0));
        }
        tallest = std::max(tallest, lines * m_meter->lineHeight(look) + 3);
        return true;
    });
    return tallest > 0 ? pointsForPixels(int(std::ceil(tallest))) : 0;
}

// The lines a text takes in a room of that width when it wraps at the spaces (a word wider than the room breaks across lines), each paragraph (a line break in the text) on lines of its own.
int GridSheet::wrappedLines(const QString &text, const CellLook &look, double room) const
{
    const double space = std::max(0.0, m_meter->width(QStringLiteral(" "), look));
    int lines = 0;
    for (const QString &paragraph : text.split(QLatin1Char('\n'))) {
        int n = 1;
        double used = 0;
        for (const QStringView word : QStringView(paragraph).split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
            const double w = m_meter->width(word.toString(), look);
            if (used > 0 && used + space + w <= room) {
                used += space + w;
            } else {
                if (used > 0) ++n;
                // a word wider than the room takes more than one line
                const int across = std::max(1, int(std::ceil(w / room)));
                n += across - 1;
                used = across > 1 ? std::fmod(w, room) : w;
                if (across > 1 && used <= 0) used = room;
            }
        }
        lines += n;
    }
    return std::max(1, lines);
}

double GridSheet::autoHeight(quint32 row, qint64 limit, qint64 *work) const
{
    if (!m_data.sheet) return 0;
    const double standard = std::max(1.0, m_meter->lineHeight(m_looks.look(0)));
    const double scale = std::max(1, m_layout.rows.defaultSize()) / standard;
    double tallest = 0;
    qint64 seen = 0;
    m_data.sheet->cells.forEachInRange(row, 0, row, jp::office::sheet::kMaxColumns - 1, [&](quint32 cellRow, quint32 cellCol, const Cell &cell) {
        if (++seen > limit) return false;
        if (cell.type == jp::office::sheet::CellType::Empty && !cell.hasFormula()) return true;
        if (m_layout.merges.at(cellRow, cellCol) >= 0) return true;
        const CellLook &look = lookOf(cellRow, cellCol, cell.style);
        const Shown s = showValue(valueOf(cellRow, cellCol, cell), look, m_data.dateSystem, columnWidth(cellCol), m_looks.maxDigitWidth(), *m_meter, &m_symbols);
        if (work) *work += 20;
        if (s.kind == Kind::Empty) return true;
        int lines = 1;
        if (look.wrap) {
            const QString text = textOf(cellRow, cellCol, cell);
            if (work) *work += std::min<qsizetype>(text.size(), kMaxMeasuredChars) + 20;
            const double room = std::max(1.0, textRoom(columnWidth(cellCol), look));
            lines = wrappedLines(text.left(kMaxMeasuredChars), look, room);
            // (what is past the characters that are measured is counted by the share of the text it is)
            if (text.size() > kMaxMeasuredChars) lines = int(std::ceil(double(lines) * text.size() / kMaxMeasuredChars));
            lines = std::min(lines, 409);   // (a row is at most 409 points high)
        }
        // (one line is the font's own row height, the one that a row of that font has; lines past the first are what the painter spaces them by, so that none is cut)
        tallest = std::max(tallest, lines == 1 ? m_meter->lineHeight(look) * scale : lines * m_meter->lineHeight(look));
        return true;
    });
    return tallest > 0 ? std::min(409.5, pointsForPixels(int(std::ceil(tallest - 0.01)))) : 0;
}

void GridSheet::setData(const GridData &data)
{
    m_data = data;
    m_rtl = data.sheet && data.sheet->view.rightToLeft;
    m_looks.setStyles(data.styles);
    g_viewCellsRebuilt.fetch_add(qint64(m_texts.size()), std::memory_order_relaxed);
    m_texts.clear();
    resetConditional();
    std::shared_ptr<const GridGeometry> g = data.geometry;
    if (!g) g = (data.sheet && data.styles) ? makeGeometry(*data.sheet, *data.styles) : std::make_shared<GridGeometry>();
    m_layout = g->layout;   // (a copy that shares the sizes and merges with the geometry's until one is changed)
    m_geometry = g;
}

bool GridSheet::followEdit(const jp::office::sheet::StructEdit &)
{
    return false;
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
    g_viewCellsRebuilt.fetch_add(qint64(places.size()), std::memory_order_relaxed);
    layout.cols.assign(std::move(places));
    // ---- rows
    const int defaultRow = std::max(1, rowPixels(sheet.format.defaultRowHeight > 0 ? sheet.format.defaultRowHeight : 15));
    layout.rows.setCount(qint64(jp::office::sheet::kMaxRows));
    layout.rows.setDefaultSize(defaultRow);
    const QVector<RowSpec> placedSpecs = sheet.placedRowSpecs();   // (a sheet that holds the specs of its rows by id gives them by the places the rows are at)
    std::vector<RowSpec> specs(placedSpecs.begin(), placedSpecs.end());
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
    g_viewCellsRebuilt.fetch_add(qint64(places.size()), std::memory_order_relaxed);
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

jp::calc::Value GridSheet::valueOf(quint32 row, quint32 col, const Cell &cell) const
{
    if (!m_data.sheet) return jp::calc::Value();
    return valueOfCell(m_data.sheetIndex, row, col, *m_data.sheet, cell);
}

const Sheet *GridSheet::sheetAt(int sheet) const
{
    if (sheet == m_data.sheetIndex) return m_data.sheet;
    return sheet >= 0 && size_t(sheet) < m_data.sheets.size() ? m_data.sheets[size_t(sheet)] : nullptr;
}

jp::calc::Value GridSheet::valueIn(int sheet, quint32 row, quint32 col) const
{
    const Sheet *s = sheetAt(sheet);
    if (!s || col >= jp::office::sheet::kMaxColumns) return jp::calc::Value();
    const Cell *cell = s->cells.find(row, col);
    return cell ? valueOfCell(sheet, row, col, *s, *cell) : jp::calc::Value();
}

void GridSheet::resetConditional() const
{
    m_cond.reset();
    m_looks.clearDerived();
}

void GridSheet::setToday(double serial) const
{
    m_cond.setToday(serial);
    m_looks.clearDerived();
}

void GridSheet::setTableSkins(std::vector<TableSkin> skins)
{
    if (skins.size() > kMaxTableSkins) skins.resize(kMaxTableSkins);
    m_skins = std::move(skins);
    m_texts.clear();
    m_looks.clearDerived();
}

const CellLook &GridSheet::lookOf(quint32 row, quint32 col, quint32 style) const
{
    if (!m_skins.empty()) {
        for (size_t i = 0; i < m_skins.size(); ++i) {
            const TableSkin &skin = m_skins[i];
            const jp::office::sheet::Table &t = skin.table;
            if (!skin.style || row < t.row1 || row > t.row2 || col < t.col1 || col > t.col2) continue;
            quint32 parts = 0;
            jp::office::sheet::DifferentialFormat d = jp::office::sheet::tableLookAt(*skin.style, t, row, col, &parts);
            if (d.isEmpty()) break;
            // (what the cell has of its own is kept: the style is under it)
            const CellLook &base = m_looks.look(style);
            int mask = 0;
            if (base.hasFill() && d.hasFill) {
                d.hasFill = false;
                mask |= 1;
            }
            if (base.hasBorder() && d.hasBorder) {
                d.hasBorder = false;
                mask |= 2;
            }
            if (base.textColor != 0xFF000000 && d.fontColor.isSet()) {
                d.fontColor = jp::office::sheet::ColorSpec();
                mask |= 4;
            }
            if (base.bold && d.bold) {
                d.bold.reset();
                mask |= 8;
            }
            if (d.isEmpty()) break;
            const int key = 0x40000000 | int((i & 0x3FF) << 17) | int((parts & 0x1FFF) << 4) | mask;
            return m_looks.derived(style, key, d);
        }
    }
    if (!m_cond.any()) return m_looks.look(style);
    const CondCell c = m_cond.at(row, col);
    if (c.format < 0) return m_looks.look(style);
    return m_looks.derived(style, c.format, m_cond.format(c.format));
}

jp::calc::Value GridSheet::valueOfCell(int sheetIndex, quint32 row, quint32 col, const Sheet &sheet, const Cell &cell) const
{
    using jp::calc::Value;
    if (m_data.values && (cell.hasFormula() || (cell.flags & jp::office::sheet::ArrayMember))) {
        Value v;
        if (m_data.values->find(sheetIndex, cell.rowId, cell.colId, &v)) return v;
    } else if (m_data.calc) {
        return m_data.calc->shown(sheetIndex, row, col);
    }
    switch (cell.type) {
    case CellType::Empty: return Value();
    case CellType::Number:
    case CellType::Date: return Value::number(cell.number);
    case CellType::SharedString: return Value::text(m_data.strings ? m_data.strings->at(cell.textIndex()) : QString());
    case CellType::InlineString:
    case CellType::FormulaString: return Value::text(sheet.texts.value(cell.textIndex()));
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
        s = showValue(jp::calc::Value::text(QLatin1Char('=') + displayFormula(m_data.sheet->formulaText(cell))), lookOf(row, col, cell.style), m_data.dateSystem, width, m_looks.maxDigitWidth(), *m_meter, &m_symbols);
    else {
        jp::calc::Value v = valueOf(row, col, cell);
        if (m_errors != ErrorDisplay::Shown && v.isError()) v = m_errors == ErrorDisplay::Blank ? jp::calc::Value() : m_errors == ErrorDisplay::Dash ? jp::calc::Value::text(QStringLiteral("--")) : jp::calc::Value::error(jp::calc::ErrorKind::NA);
        s = showValue(v, lookOf(row, col, cell.style), m_data.dateSystem, width, m_looks.maxDigitWidth(), *m_meter, &m_symbols);
    }
    // Where it sits in the grid's places: a text with the general alignment begins on the side its direction says (a number is at the right, true, false, and errors in the middle), and in
    // a sheet that reads right to left the places are mirrored for the screen, so left and right change.
    {
        const CellLook &look = lookOf(row, col, cell.style);
        HAlign align = look.horizontal;
        if (align == HAlign::General) {
            if (s.kind == Kind::Text) {
                bool rtl = m_rtl;
                if (look.readingOrder == 1) rtl = false;
                else if (look.readingOrder == 2) rtl = true;
                else
                    for (const QChar c : s.text) {
                        const QChar::Direction d = c.direction();
                        if (d == QChar::DirL) {
                            rtl = false;
                            break;
                        }
                        if (d == QChar::DirR || d == QChar::DirAL) {
                            rtl = true;
                            break;
                        }
                    }
                align = rtl ? HAlign::Right : HAlign::Left;
            } else {
                align = s.align;
            }
        }
        if (m_rtl) align = align == HAlign::Left ? HAlign::Right : align == HAlign::Right ? HAlign::Left : align;
        s.align = align;
    }
    if (m_texts.size() >= kMaxTexts) m_texts.clear();
    m_texts.emplace(key, s);
    return s;
}

QString GridSheet::textOf(quint32 row, quint32 col, const Cell &cell) const
{
    using jp::calc::Value;
    const Value v = valueOf(row, col, cell);
    const CellLook &look = lookOf(row, col, cell.style);
    switch (v.kind()) {
    case Value::Kind::Text: {
        if (look.hasFormat) {
            const jp::calc::Formatted f = look.format.formatText(v.text());
            if (!f.failed) return f.text;
        }
        return v.text();
    }
    case Value::Kind::Number: {
        const jp::calc::Formatted f = look.format.format(v.number(), m_data.dateSystem, 0, &m_symbols);
        return f.failed ? jp::calc::numberToText(v.number()) : f.text;
    }
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return jp::calc::errorKindText(v.errorKind());
    default: return QString();
    }
}

void GridSheet::invalidateLooks() const
{
    m_texts.clear();
    m_cond.resultsChanged();
}

void GridSheet::invalidateTexts() const
{
    m_texts.clear();
    m_cond.valuesChanged();
}

void GridSheet::setNumberSymbols(const jp::calc::NumberSymbols &symbols)
{
    if (symbols.decimal == m_symbols.decimal && symbols.group == m_symbols.group) return;
    m_symbols = symbols;
    m_texts.clear();
}

void GridSheet::setErrorDisplay(ErrorDisplay mode)
{
    if (mode == m_errors) return;
    m_errors = mode;
    m_texts.clear();
}

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
