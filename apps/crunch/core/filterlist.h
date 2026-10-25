#pragma once
// What a filter's drop-down lists for a column: the distinct values the cells under the button show (with how many cells show each, and blank cells), for a column of dates the years,
// months, and days that occur, and the colors the cells have. It is made a slice at a time (FilterListRun, stepped by the window between events) and holds at most kMaxFilterItems
// values; a column with more says so.

#include "core/editsession.h"

#include <QString>

#include <map>
#include <set>
#include <vector>

namespace jp::crunch {

constexpr int kMaxFilterItems = 5000;

struct FilterList {
    struct Item {
        QString text;          // what the cells show
        qint64 count = 0;
        int rank = 0;          // 0 a number, 1 text, 2 true or false, 3 an error: the order the list has them in
        double number = 0;
    };
    std::vector<Item> items;   // in the order of a sort
    bool blanks = false;       // some cell shows nothing
    qint64 cells = 0;          // cells looked at
    bool truncated = false;    // there are more values than the list holds
    // the kind of column: most of its cells are numbers, dates, or text
    enum class Kind { Text, Number, Date } kind = Kind::Text;
    // a column of dates: the years with their months and days
    std::map<int, std::map<int, std::set<int>>> dates;
    // the colors of the cells under the button (cell colors, font colors), and whether some have none
    std::vector<jp::office::sheet::ColorSpec> cellColors, fontColors;
    bool noCellColor = false, noFontColor = false;
};

class FilterListRun {
public:
    // The values of the column `column` (from the first column of the filter's range) under the buttons of the sheet's filter.
    FilterListRun(const EditSession *session, int sheet, const jp::office::sheet::AutoFilter &filter, quint32 column);
    // Looks at about `budget` more cells (0: all); true when done.
    bool step(qint64 budget);
    bool done() const { return m_done; }
    const FilterList &list() const { return m_list; }
    // Items and dates whose text matches (a search in the list: all the words, in any case), at most `limit`.
    static std::vector<size_t> search(const FilterList &list, const QString &text, size_t limit = 100000);

private:
    const EditSession *m_session;
    int m_sheet;
    quint32 m_col, m_row, m_last;
    FilterList m_list;
    std::map<QString, size_t> m_at;      // folded text -> place in the items (before they are sorted)
    qint64 m_dates = 0, m_numbers = 0, m_texts = 0;
    bool m_done = false;
};

} // namespace jp::crunch
