#pragma once
// The margins of the outline: beside the rows (and above the columns) a bracket for each group and, beside the line that sums the group up, a button that collapses or expands it, with
// the level buttons (1, 2, 3, ...) that show the outline down to a level. The widgets only draw and tell where they were clicked; the window changes the sheet.

#include "core/outline.h"

#include <QWidget>

#include <memory>

namespace jp::grid {
class GridView;
}

namespace jp {

class OutlineGutter : public QWidget {
    Q_OBJECT
public:
    // `columns`: above the columns (else beside the rows). The gutter lines up with the cells of `view`.
    OutlineGutter(bool columns, grid::GridView *view, QWidget *parent = nullptr);
    // The outline to draw; none, or one with no levels, hides the widget. The size follows how deep the outline goes.
    void setOutline(std::shared_ptr<const crunch::Outline> outline);
    // The width (beside the rows) or height (above the columns) it takes for an outline `levels` deep.
    static int thicknessFor(int levels) { return levels > 0 ? 14 * (levels + 1) + 4 : 0; }

    // What is at a point of the widget, and where each button is (for the tests, and the clicks).
    struct Hit {
        enum class Kind { None, Group, Level } kind = Kind::None;
        crunch::Outline::Group group;
        int level = 0;   // for a level button: 1 for the first
    };
    Hit hitAt(const QPoint &point) const;
    QRect groupButton(const crunch::Outline::Group &group) const;   // empty when the group's summary line is not in view
    QRect levelButton(int level) const;                              // level 1 for the first
    // The groups the widget is drawing a bracket for now.
    std::vector<crunch::Outline::Group> visibleGroups() const;
    // The lines looked at while drawing and hit-testing, and the times the groups were worked out (counted, for tests).
    qint64 workDone() const { return m_work; }
    qint64 groupsBuilt() const { return m_groupsBuilt; }
    void resetWork() { m_work = 0; }

Q_SIGNALS:
    void groupClicked(quint32 first, quint32 last, int level, bool collapsed, qint64 summary);
    void levelClicked(int level);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    // The place of a line of the sheet along the gutter (its top edge and size), in this widget's pixels; false when it is not in view.
    bool lineSpan(quint32 index, int *start, int *size) const;
    // The lines in view, a range for each part of the grid (the frozen lines, and the lines that scroll): each is looked at by itself, so a view far down a sheet with the first line frozen has two short
    // ranges and not one from the first line to where it is.
    std::vector<std::pair<quint32, quint32>> visibleRanges() const;
    std::vector<crunch::Outline::Group> groupsIn(quint32 from, quint32 to) const;
    int along(const QPoint &p) const { return m_columns ? p.x() : p.y(); }
    QRect box(int across, int alongStart, int alongSize) const;   // a rectangle given as the position across the gutter and the span along it

    mutable qint64 m_work = 0, m_groupsBuilt = 0;
    bool m_columns;
    grid::GridView *m_view;
    std::shared_ptr<const crunch::Outline> m_outline;
    std::vector<crunch::Outline::Group> m_groups;   // every group of the outline, made once when it is set: by level, then by place
    std::vector<size_t> m_levelStart;               // where each level begins in the list
};

} // namespace jp
