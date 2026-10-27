#include "grid/textwalk.h"

#include <algorithm>

namespace jp::grid {

TextWalk::TextWalk(const GridSheet *sheet, const GridRange &range) : m_sheet(sheet), m_range(range)
{
    const GridLayout &L = sheet->layout();
    m_tooMany = range.rows() * range.cols() > kMaxCells;
    m_done = m_tooMany || range.row2 < range.row1 || range.col2 < range.col1;
    // (the first row and column shown)
    m_row = L.rows.nextVisible(range.row1 - 1);
    m_col = L.cols.nextVisible(range.col1 - 1);
}

bool TextWalk::step(qint64 budget)
{
    if (m_done) return true;
    const GridLayout &L = m_sheet->layout();
    const qint64 first = L.cols.nextVisible(m_range.col1 - 1), stop = m_looked + budget;
    while (m_row <= m_range.row2 && m_row < L.rows.count()) {
        for (; m_col <= m_range.col2 && m_col < L.cols.count(); m_col = L.cols.nextVisible(m_col)) {
            if (budget > 0 && m_looked >= stop) return false;
            if (!m_firstInLine) m_line += QLatin1Char('\t');
            m_firstInLine = false;
            const jp::office::sheet::Cell *c = m_sheet->cellAt(quint32(m_row), quint32(m_col));
            if (!c || (c->type == jp::office::sheet::CellType::Empty && !c->hasFormula())) {
                ++m_looked;
                continue;
            }
            m_looked += 8;
            QString text = m_sheet->textOf(quint32(m_row), quint32(m_col), *c);
            m_looked += text.size() / 64;   // (a long text takes its length's time to copy)
            m_chars += text.size() + 1;
            if (m_chars > kMaxChars) {
                // more text than a clipboard is given: the walk ends, with nothing to copy
                m_tooLarge = true;
                m_done = true;
                m_text = QString();
                m_line = QString();
                return true;
            }
            if (text.contains(QLatin1Char('\t')) || text.contains(QLatin1Char('\n')) || text.contains(QLatin1Char('"')))
                text = QLatin1Char('"') + text.replace(QLatin1Char('"'), QLatin1String("\"\"")) + QLatin1Char('"');
            m_line += text;
        }
        m_text += m_line + QLatin1Char('\n');
        m_line.clear();
        m_firstInLine = true;
        m_col = first;
        m_row = L.rows.nextVisible(m_row);
    }
    m_done = true;
    return true;
}

} // namespace jp::grid
