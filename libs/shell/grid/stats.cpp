#include "grid/stats.h"

#include <QCoreApplication>
#include <QLocale>

#include <algorithm>
#include <vector>

namespace jp::grid {

StatsWalk::StatsWalk(const GridSheet *sheet, const GridRange &range) : m_sheet(sheet), m_range(range), m_next(range.row1) {}

bool StatsWalk::step(qint64 budget)
{
    const Sheet *data = m_sheet->data().sheet;
    if (!data) return true;
    const GridLayout &layout = m_sheet->layout();
    const qint64 start = m_rowsLooked;
    // the columns of the range that are hidden, found once
    const qint64 cols = std::min<qint64>(m_range.col2 - m_range.col1 + 1, qint64(jp::office::sheet::kMaxColumns));
    std::vector<char> hidden(size_t(std::max<qint64>(cols, 0)), 0);
    for (qint64 c = 0; c < cols; ++c) hidden[size_t(c)] = layout.cols.isHidden(m_range.col1 + c) ? 1 : 0;
    while (m_next <= m_range.row2) {
        if (budget > 0 && m_rowsLooked - start >= budget) return false;
        const qint64 last = std::min(m_next + kRowsPerSlice - 1, m_range.row2);
        qint64 lastRow = -1;
        bool lastHidden = false;
        data->cells.forEachInRange(quint32(m_next), quint32(m_range.col1), quint32(last), quint32(std::min<qint64>(m_range.col2, qint64(jp::office::sheet::kMaxColumns) - 1)),
                                   [&](const jp::office::sheet::Cell &cell) {
                                       if (qint64(cell.row) != lastRow) {
                                           lastRow = cell.row;
                                           lastHidden = layout.rows.isHidden(lastRow);
                                       }
                                       if (lastHidden || hidden[size_t(qint64(cell.col) - m_range.col1)]) return true;
                                       if (cell.type == jp::office::sheet::CellType::Empty && !cell.hasFormula()) return true;
                                       const jp::calc::Value v = m_sheet->valueOf(cell);
                                       if (v.isEmpty()) return true;
                                       ++m_stats.count;
                                       if (v.isNumber()) {
                                           const double x = v.number();
                                           if (m_stats.numbers == 0) m_stats.min = m_stats.max = x;
                                           m_stats.min = std::min(m_stats.min, x);
                                           m_stats.max = std::max(m_stats.max, x);
                                           m_stats.sum += x;
                                           ++m_stats.numbers;
                                       }
                                       return true;
                                   });
        m_rowsLooked += last - m_next + 1;
        m_next = last + 1;
    }
    return true;
}

Stats computeStats(const GridSheet &sheet, const GridRange &range)
{
    StatsWalk w(&sheet, range);
    w.step(0);
    return w.stats();
}

QString statsText(const Stats &s)
{
    auto number = [](double v) { return QLocale::c().toString(v, 'g', 10); };
    auto label = [](const char *text) { return QCoreApplication::translate("jp::grid::Stats", text); };
    if (s.hasNumbers())
        return QStringLiteral("%1: %2    %3: %4    %5: %6")
            .arg(label("Average"), number(s.average()), label("Count"), QString::number(s.count), label("Sum"), number(s.sum));
    if (s.count >= 2) return QStringLiteral("%1: %2").arg(label("Count"), QString::number(s.count));
    return QString();
}

} // namespace jp::grid
