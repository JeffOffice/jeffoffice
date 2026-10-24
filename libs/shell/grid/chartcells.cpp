#include "grid/chartcells.h"

#include <algorithm>

namespace jp::grid {

using jp::calc::Value;
using jp::office::sheet::Cell;
using jp::office::sheet::CellType;

WorkbookChartCells::WorkbookChartCells(const jp::office::sheet::WorkbookSource *source, const ValueStore *values, const jp::office::sheet::Styles *styles, jp::calc::DateSystem dateSystem)
    : m_source(source), m_values(values), m_styles(styles), m_dateSystem(dateSystem), m_looks(styles)
{
}

int WorkbookChartCells::sheetCount() const { return m_source ? m_source->sheetCount() : 0; }

int WorkbookChartCells::sheetIndex(const QString &name) const { return m_source ? m_source->sheetIndex(name) : -1; }

const Cell *WorkbookChartCells::cellAt(int sheet, quint32 row, quint32 col) const
{
    if (!m_source || sheet < 0 || sheet >= m_source->sheetCount()) return nullptr;
    return m_source->sheet(sheet).cells.find(row, col);
}

Value WorkbookChartCells::valueOf(int sheet, const Cell &cell) const
{
    // what the grid shows: the value the calculation told for a formula, else what the file stored or the cell holds
    if (m_values && (cell.hasFormula() || (cell.flags & jp::office::sheet::ArrayMember))) {
        Value v;
        if (m_values->find(sheet, cell.row, cell.col, &v)) return v;
    }
    return m_source->valueOf(m_source->sheet(sheet), cell);
}

Value WorkbookChartCells::value(int sheet, quint32 row, quint32 col) const
{
    const Cell *c = cellAt(sheet, row, col);
    return c ? valueOf(sheet, *c) : Value();
}

QString WorkbookChartCells::text(int sheet, quint32 row, quint32 col) const
{
    const Cell *c = cellAt(sheet, row, col);
    if (!c) return QString();
    m_looks.trim();   // (no look is held here between calls)
    const Value v = valueOf(sheet, *c);
    switch (v.kind()) {
    case Value::Kind::Text: {
        const CellLook &look = m_looks.look(c->style);
        if (look.hasFormat) {
            const jp::calc::Formatted f = look.format.formatText(v.text());
            if (!f.failed) return f.text;
        }
        return v.text();
    }
    case Value::Kind::Number: {
        const jp::calc::Formatted f = m_looks.look(c->style).format.format(v.number(), m_dateSystem, 0);
        return f.failed ? jp::calc::numberToText(v.number()) : f.text;
    }
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return jp::calc::errorKindText(v.errorKind());
    case Value::Kind::Empty:
    case Value::Kind::Array: break;
    }
    return QString();
}

const WorkbookChartCells::Hidden &WorkbookChartCells::hidden(int sheet) const
{
    Hidden &h = m_hidden[sheet];
    if (h.made) return h;
    h.made = true;
    if (!m_source || sheet < 0 || sheet >= m_source->sheetCount()) return h;
    const jp::office::sheet::Sheet &s = m_source->sheet(sheet);
    for (const jp::office::sheet::RowSpec &r : s.rowSpecs)
        if (r.hidden) h.rows.insert(r.row);
    for (const jp::office::sheet::ColumnSpan &c : s.columns)
        if (c.hidden) h.columns.emplace_back(c.first, c.last);
    return h;
}

bool WorkbookChartCells::rowHidden(int sheet, quint32 row) const { return hidden(sheet).rows.count(row) != 0; }

bool WorkbookChartCells::columnHidden(int sheet, quint32 col) const
{
    for (const auto &run : hidden(sheet).columns)
        if (col >= run.first && col <= run.second) return true;
    return false;
}

void WorkbookChartCells::extent(int sheet, quint32 *rows, quint32 *cols) const
{
    *rows = *cols = 0;
    if (!m_source || sheet < 0 || sheet >= m_source->sheetCount()) return;
    *rows = m_source->sheet(sheet).extentRows;
    *cols = m_source->sheet(sheet).extentCols;
}

QString WorkbookChartCells::definedName(const QString &name, int sheet) const { return m_source ? m_source->definedName(sheet, name) : QString(); }

void WorkbookChartCells::sizesChanged() { m_hidden.clear(); }

} // namespace jp::grid
