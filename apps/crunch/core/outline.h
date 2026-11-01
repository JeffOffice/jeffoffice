#pragma once
// The outline of a sheet's rows or columns: lines (rows or columns) with a level from 1 to 7 are grouped, a group being a run of lines of that level or deeper; the line beside the run (after
// it, or before it when the sheet's summary lines are above or to the left) is the group's summary, which carries the button that collapses or expands the group. A collapsed group has its lines
// hidden and its summary marked collapsed. This works out which groups there are and what to change in the lines to collapse or expand one, to show down to a level, and to group or ungroup
// lines; it changes nothing itself (EditSession::changeOutline does).

#include <QtGlobal>

#include <vector>

namespace jp::crunch {

constexpr int kMaxOutlineLevel = 7;

// The state of one line that has any: the lines with no level, shown, and not collapsed are not listed.
struct OutlineLine {
    quint32 index = 0;
    quint8 level = 0;
    bool hidden = false, collapsed = false;
};

// What to do to a line: -1 leaves the part as it is.
struct OutlineChange {
    quint32 index = 0;
    int level = -1, hidden = -1, collapsed = -1;
};

class Outline {
public:
    // `lines` are in order of index; `count` is the number of lines the sheet has; `summaryAfter` is where the summary lines are (below the rows, to the right of the columns).
    Outline(std::vector<OutlineLine> lines, quint32 count, bool summaryAfter);

    struct Group {
        quint32 first = 0, last = 0;   // the lines of the group
        int level = 1;                 // 1 for the outermost
        qint64 summary = -1;           // the line that carries the button, or -1 when the group reaches the edge of the sheet
        bool collapsed = false;        // the summary says the group is collapsed
    };

    quint8 levelOf(quint32 index) const;
    bool hiddenOf(quint32 index) const;
    bool collapsedOf(quint32 index) const;
    // The deepest level any line has (0 for none).
    int deepest() const { return m_deepest; }
    // Every group that has a line in [from, to], outer ones first, then by place.
    std::vector<Group> groups(quint32 from, quint32 to) const;

    // The changes that collapse the group (its lines hidden, its summary marked), and that expand it (its lines shown, except those of a group inside it that is collapsed).
    std::vector<OutlineChange> collapse(const Group &group) const;
    std::vector<OutlineChange> expand(const Group &group) const;
    // The changes that show only the lines of a level less than `button` (the buttons 1, 2, 3 of the corner: 1 shows the lines of no level only).
    std::vector<OutlineChange> showButton(int button) const;
    // The changes that give the lines first..last one level more, or one less (to the deepest or the plainest the outline has); lines that get no level are shown, and a summary whose group
    // is gone is no longer marked collapsed.
    std::vector<OutlineChange> group(quint32 first, quint32 last) const;
    std::vector<OutlineChange> ungroup(quint32 first, quint32 last) const;

private:
    const OutlineLine *find(quint32 index) const;
    std::vector<OutlineLine> m_lines;
    quint32 m_count;
    bool m_after;
    int m_deepest = 0;
};

} // namespace jp::crunch
