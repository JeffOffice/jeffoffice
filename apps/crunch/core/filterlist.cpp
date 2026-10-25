#include "core/filterlist.h"

#include "calc/dates.h"
#include "calc/numfmt.h"

#include <QCollator>
#include <QLocale>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

FilterListRun::FilterListRun(const EditSession *session, int sheet, const AutoFilter &filter, quint32 column) : m_session(session), m_sheet(sheet), m_col(filter.ref.col1 + column)
{
    m_row = filter.ref.row1 + 1;
    m_last = filter.ref.row2;
    if (!session || !session->calculation() || !filter.ref.valid || m_row > m_last || sheet < 0 || sheet >= session->calculation()->sheetCount()) {
        m_done = true;
        return;
    }
    // (a filter over more rows than an edit holds is looked at as far as one does)
    if (qint64(m_last) - m_row + 1 > EditSession::kMaxFilterRows) m_last = quint32(m_row + EditSession::kMaxFilterRows - 1);
}

bool FilterListRun::step(qint64 budget)
{
    if (m_done) return true;
    const Calculation *calc = m_session->calculation();
    const Sheet &s = calc->sheet(m_sheet);
    const Workbook *book = m_session->workbook();
    qint64 spent = 0;
    for (; m_row <= m_last; ++m_row) {
        if (budget > 0 && spent >= budget) return false;
        ++spent;
        ++m_list.cells;
        const Cell *c = s.cells.find(m_row, m_col);
        // colors
        {
            ColorSpec fill, font;
            if (c) {
                const CellFormat &format = book->styles().format(c->style);
                const FillSpec &f = book->styles().fill(format.fill);
                if (f.pattern != PatternKind::None) fill = f.fg;
                font = book->styles().font(format.font).color;
                if (fill.kind == ColorSpec::Kind::Auto) fill = ColorSpec();
                if (font.kind == ColorSpec::Kind::Auto) font = ColorSpec();
            }
            auto note = [](std::vector<ColorSpec> *list, bool *none, const ColorSpec &color) {
                if (!color.isSet()) {
                    *none = true;
                    return;
                }
                if (list->size() < 64 && std::find(list->begin(), list->end(), color) == list->end()) list->push_back(color);
            };
            note(&m_list.cellColors, &m_list.noCellColor, fill);
            note(&m_list.fontColors, &m_list.noFontColor, font);
        }
        const Value v = c ? m_session->valueAt(m_sheet, m_row, m_col) : Value();
        if (v.isEmpty()) {
            m_list.blanks = true;
            continue;
        }
        if (v.isNumber()) {
            const QString code = book->styles().formatCode(c->style, QLocale::c());
            const jp::calc::NumberFormat format(code);
            if (format.kind() == jp::calc::FormatKind::Date) {
                ++m_dates;
                int y = 0, mo = 0, d = 0;
                if (jp::calc::serialToDate(std::floor(v.number()), book->info().dateSystem, &y, &mo, &d)) m_list.dates[y][mo].insert(d);
            } else {
                ++m_numbers;
            }
        } else {
            ++m_texts;
        }
        const QString text = m_session->displayText(m_sheet, m_row, m_col);
        const QString key = text.toCaseFolded();
        const auto found = m_at.find(key);
        if (found != m_at.end()) {
            ++m_list.items[found->second].count;
            continue;
        }
        if (int(m_list.items.size()) >= kMaxFilterItems) {
            m_list.truncated = true;
            continue;
        }
        FilterList::Item item;
        item.text = text;
        item.count = 1;
        item.rank = v.isNumber() ? 0 : v.isText() ? 1 : v.isBool() ? 2 : 3;
        item.number = v.isNumber() || v.isBool() ? v.number() : 0;
        m_at.emplace(key, m_list.items.size());
        m_list.items.push_back(std::move(item));
    }
    // done: the order of a sort, and the kind of the column
    QCollator collator(QLocale(QLocale::English, QLocale::UnitedStates));
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::stable_sort(m_list.items.begin(), m_list.items.end(), [&](const FilterList::Item &a, const FilterList::Item &b) {
        if (a.rank != b.rank) return a.rank < b.rank;
        if (a.rank == 1) return collator.compare(a.text, b.text) < 0;
        return a.number < b.number;
    });
    m_list.kind = m_dates > 0 && m_dates >= m_numbers && m_dates >= m_texts ? FilterList::Kind::Date : m_numbers >= m_texts && m_numbers > 0 ? FilterList::Kind::Number : FilterList::Kind::Text;
    m_done = true;
    return true;
}

std::vector<size_t> FilterListRun::search(const FilterList &list, const QString &text, size_t limit)
{
    std::vector<size_t> out;
    const QStringList words = text.toCaseFolded().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (size_t i = 0; i < list.items.size() && out.size() < limit; ++i) {
        const QString folded = list.items[i].text.toCaseFolded();
        bool all = true;
        for (const QString &w : words)
            if (!folded.contains(w)) {
                all = false;
                break;
            }
        if (all) out.push_back(i);
    }
    return out;
}

} // namespace jp::crunch
