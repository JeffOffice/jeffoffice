#include "grid/stats.h"

#include <QCoreApplication>
#include <QLocale>

#include <algorithm>
#include <cmath>
#include <vector>

namespace jp::grid {

StatsWalk::StatsWalk(const GridSheet *sheet, const GridRange &range) : m_sheet(sheet), m_range(range), m_next(range.row1)
{
    // the columns of the range that are hidden, found once
    const GridLayout &layout = sheet->layout();
    const qint64 cols = std::min<qint64>(range.col2 - range.col1 + 1, qint64(jp::office::sheet::kMaxColumns));
    m_hidden.assign(size_t(std::max<qint64>(cols, 0)), 0);
    for (qint64 c = 0; c < cols; ++c) m_hidden[size_t(c)] = layout.cols.isHidden(range.col1 + c) ? 1 : 0;
    ++m_hiddenBuilds;
}

bool StatsWalk::step(qint64 budget)
{
    const Sheet *data = m_sheet->data().sheet;
    if (!data) return true;
    const GridLayout &layout = m_sheet->layout();
    const quint32 lastCol = quint32(std::min<qint64>(m_range.col2, qint64(jp::office::sheet::kMaxColumns) - 1));
    qint64 spent = 0;   // the work of this look: a row is one, and so is each cell in it
    while (m_next <= m_range.row2) {
        if (budget > 0 && spent >= budget) return false;
        // the next row that is shown (the hidden ones are stepped over in one step)
        const qint64 row = layout.rows.nextVisible(m_next - 1);
        if (row > m_range.row2 || row >= layout.rows.count()) {
            m_rowsLooked += m_range.row2 - m_next + 1;
            m_next = m_range.row2 + 1;
            break;
        }
        m_rowsLooked += row - m_next + 1;
        m_next = row + 1;
        ++spent;
        data->cells.forEachInRange(quint32(row), quint32(m_range.col1), quint32(row), lastCol, [&](quint32, quint32 cellCol, const jp::office::sheet::Cell &cell) {
            ++spent;
            ++m_cellsLooked;
            if (m_hidden[size_t(qint64(cellCol) - m_range.col1)]) return true;
            if (cell.type == jp::office::sheet::CellType::Empty && !cell.hasFormula()) return true;
            const jp::calc::Value v = m_sheet->valueOf(quint32(row), cellCol, cell);
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
    // (the status line is short: a sum and an average are shown to ten significant digits, in the user's locale, grouped, and in the scientific way only when very large or very small)
    auto number = [](double v) {
        const QLocale locale;
        const double a = std::fabs(v);
        if (a >= 1e15 || (a > 0 && a < 1e-9)) return locale.toString(v, 'g', 10);
        const int whole = a >= 1 ? int(std::floor(std::log10(a))) + 1 : -int(std::floor(std::log10(a))) - 1;   // (digits before the point, or zeros after it)
        const int decimals = a >= 1 ? std::max(0, 10 - whole) : std::min(20, 10 + whole);
        QString text = locale.toString(v, 'f', decimals);
        const QString point = locale.decimalPoint();
        if (decimals > 0 && text.contains(point)) {
            while (text.endsWith(QLatin1Char('0'))) text.chop(1);
            if (text.endsWith(point)) text.chop(point.size());
        }
        return text;
    };
    if (s.hasNumbers())
        return QStringLiteral("%1: %2    %3: %4    %5: %6")
            .arg(QCoreApplication::translate("jp::grid::Stats", "Average"), number(s.average()), QCoreApplication::translate("jp::grid::Stats", "Count"), QLocale().toString(s.count), QCoreApplication::translate("jp::grid::Stats", "Sum"), number(s.sum));
    if (s.count >= 2) return QStringLiteral("%1: %2").arg(QCoreApplication::translate("jp::grid::Stats", "Count"), QLocale().toString(s.count));
    return QString();
}

} // namespace jp::grid
