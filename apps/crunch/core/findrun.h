#pragma once
// Looking for text in the cells of a workbook, a little at a time: Find All collects the cells that match, and Find Next the one that comes after a place (wrapping round to the start).
// A search is a FindRun that the window steps a few thousand cells at a time between events (so that a sheet of a million cells never holds the window), and that a person can give up;
// a test steps it by counted budgets. The cells are those the sheet has (a sheet is searched cell by cell as the store holds them, never by its rows and columns), the work is counted
// in cells looked at, and the hits are at most kMaxFindHits.

#include "core/editsession.h"
#include "sheet/notes.h"
#include "sheet/textmatch.h"

#include <QHash>
#include <QString>

#include <map>
#include <tuple>
#include <vector>

namespace jp::crunch {

constexpr qint64 kMaxFindHits = 100000;

struct FindOptions {
    jp::office::sheet::TextPattern pattern;
    enum class Scope { Sheet, Workbook } scope = Scope::Sheet;
    enum class Order { ByRows, ByColumns } order = Order::ByRows;
    enum class LookIn { Formulas, Values, Notes } lookIn = LookIn::Formulas;
    int sheet = 0;                          // the sheet in view (the only one for Scope::Sheet)
    jp::office::sheet::CellRange within;    // if valid: only these cells (of `sheet`)
};

struct FoundCell {
    int sheet = 0;
    quint32 row = 0, col = 0;
    QString text;      // what was searched in the cell (its formula or entry, what it shows, or its note), cut to 200 characters
    bool operator==(const FoundCell &o) const { return sheet == o.sheet && row == o.row && col == o.col; }
};

class FindRun {
public:
    enum class Want { All, Next };
    // Find All, or Find Next after `from` (the cell with the cursor; the match after it in the order, else the first of all). The session must outlive the run and not change the
    // workbook while it goes on (the window gives it up when anything is edited).
    FindRun(const EditSession *session, const FindOptions &options, Want want = Want::All, const FoundCell &from = FoundCell());

    // Looks at about `budget` more cells (a cell not there costs nothing; 0 is no limit). True when the search is over.
    bool step(qint64 budget);
    bool done() const { return m_done; }
    qint64 cellsLooked() const { return m_looked; }
    // Find All: the cells, in the order of the search (sheets in the order of the tabs; by rows or by columns), when done. Find Next: the one cell, when done and found.
    const std::vector<FoundCell> &hits() const { return m_hits; }
    bool found() const { return !m_hits.empty(); }
    bool truncated() const { return m_truncated; }
    // Why nothing was searched (a pattern that is not usable, or notes that cannot be looked in), else empty.
    QString problem() const { return m_problem; }

private:
    struct Sweep {
        int sheet = 0;
        bool started = false;
        quint32 row = 0, col = 0;     // where the walk of the sheet's cells stands (the last cell looked at)
        bool finished = false;
        std::map<quint64, QString> notes;
        bool notesRead = false;
    };
    using Key = std::tuple<int, quint32, quint32>;
    Key keyOf(int sheetPos, quint32 row, quint32 col) const;
    int positionOf(int sheet) const;
    void look(int sheet, quint32 row, quint32 col, const std::map<quint64, QString> *notes);
    bool walkSheet(Sweep &sw, qint64 budget, qint64 *looked);
    void finish();
    QString textOf(int sheet, quint32 row, quint32 col, const std::map<quint64, QString> *notes) const;

    const EditSession *m_session;
    FindOptions m_options;
    Want m_want;
    jp::office::sheet::TextMatcher m_matcher;
    std::vector<int> m_sheets;
    size_t m_at = 0;
    Sweep m_sweep;
    FoundCell m_from;
    bool m_haveFrom = false;
    // Next: the best match after the start in the order, and the first of all
    bool m_haveAfter = false, m_haveFirst = false;
    FoundCell m_after, m_first;
    std::vector<FoundCell> m_hits;
    bool m_done = false, m_truncated = false;
    qint64 m_looked = 0;
    QString m_problem;
};

} // namespace jp::crunch
