#include "app/outlinegutter.h"

#include "grid/gridview.h"

#include <QMouseEvent>
#include <QPainter>

#include <algorithm>

namespace jp {

namespace {
constexpr int kStep = 14, kBox = 11;
}

OutlineGutter::OutlineGutter(bool columns, grid::GridView *view, QWidget *parent) : QWidget(parent), m_columns(columns), m_view(view)
{
    setObjectName(columns ? QStringLiteral("outlineGutterColumns") : QStringLiteral("outlineGutterRows"));
    setAccessibleName(columns ? tr("Outline of the columns") : tr("Outline of the rows"));
    setAccessibleDescription(tr("Buttons that show or hide the groups of the %1, and the level buttons that show the outline down to a level.").arg(columns ? tr("columns") : tr("rows")));
    hide();
}

void OutlineGutter::setOutline(std::shared_ptr<const crunch::Outline> outline)
{
    m_outline = std::move(outline);
    m_groups.clear();
    m_levelStart.clear();
    if (m_outline && m_outline->deepest() > 0) {
        ++m_groupsBuilt;
        m_groups = m_outline->groups(0, 0xFFFFFFFFu);
        for (size_t i = 0; i < m_groups.size(); ++i)
            while (m_levelStart.size() < size_t(m_groups[i].level)) m_levelStart.push_back(i);
        m_levelStart.push_back(m_groups.size());
    }
    const int levels = m_outline ? m_outline->deepest() : 0;
    const int t = thicknessFor(levels);
    if (m_columns) setFixedHeight(std::max(t, 0));
    else setFixedWidth(std::max(t, 0));
    setVisible(levels > 0);
    update();
}

// A rectangle from the place across the gutter (the distance from its edge nearest the cells' outside, along the other axis) and the span along it.
QRect OutlineGutter::box(int across, int alongStart, int alongSize) const
{
    return m_columns ? QRect(alongStart, across, alongSize, kBox) : QRect(across, alongStart, kBox, alongSize);
}

std::vector<std::pair<quint32, quint32>> OutlineGutter::visibleRanges() const
{
    const grid::GridLayout &L = m_view->sheet().layout();
    std::vector<std::pair<quint32, quint32>> out;
    for (grid::Pane which : m_columns ? std::initializer_list<grid::Pane>{grid::Pane::Left, grid::Pane::Body} : std::initializer_list<grid::Pane>{grid::Pane::Top, grid::Pane::Body}) {
        const grid::PaneView pv = L.pane(which);
        if (pv.isEmpty()) continue;
        out.emplace_back(quint32(m_columns ? pv.firstCol : pv.firstRow), quint32(m_columns ? pv.lastCol : pv.lastRow));
    }
    return out;
}

// The groups with a line in [from, to]: in each level the groups are in order and do not overlap, so the first is found by halving.
std::vector<crunch::Outline::Group> OutlineGutter::groupsIn(quint32 from, quint32 to) const
{
    std::vector<crunch::Outline::Group> out;
    for (size_t level = 0; level + 1 < m_levelStart.size(); ++level) {
        const auto begin = m_groups.begin() + qptrdiff(m_levelStart[level]), end = m_groups.begin() + qptrdiff(m_levelStart[level + 1]);
        auto it = std::partition_point(begin, end, [from](const crunch::Outline::Group &g) { return g.last < from; });
        for (; it != end && it->first <= to; ++it) out.push_back(*it);
    }
    return out;
}

bool OutlineGutter::lineSpan(quint32 index, int *start, int *size) const
{
    ++m_work;
    // (the cell at the crossing of the line with the first line in view of the other axis; a hidden line has none)
    const grid::GridLayout &L = m_view->sheet().layout();
    const grid::PaneView body = L.pane(grid::Pane::Body), other = L.pane(m_columns ? grid::Pane::Top : grid::Pane::Left);
    const qint64 across = m_columns ? (body.isEmpty() ? (other.isEmpty() ? 0 : other.firstRow) : body.firstRow) : (body.isEmpty() ? (other.isEmpty() ? 0 : other.firstCol) : body.firstCol);
    const QRect r = m_columns ? m_view->cellRect(across, index) : m_view->cellRect(index, across);
    if (r.isEmpty()) return false;
    const QPoint origin = mapFromGlobal(m_view->viewport()->mapToGlobal(r.topLeft()));
    *start = along(origin);
    *size = m_columns ? r.width() : r.height();
    return true;
}

std::vector<crunch::Outline::Group> OutlineGutter::visibleGroups() const
{
    std::vector<crunch::Outline::Group> out;
    if (!m_outline) return out;
    for (const auto &range : visibleRanges())
        for (const crunch::Outline::Group &g : groupsIn(range.first, range.second)) {
            const bool seen = std::any_of(out.begin(), out.end(), [&](const crunch::Outline::Group &o) { return o.level == g.level && o.first == g.first; });
            if (!seen) out.push_back(g);
        }
    return out;
}

QRect OutlineGutter::groupButton(const crunch::Outline::Group &g) const
{
    if (g.summary < 0) return QRect();
    int start = 0, size = 0;
    if (!lineSpan(quint32(g.summary), &start, &size)) return QRect();
    const int across = 2 + (g.level - 1) * kStep + (kStep - kBox) / 2;
    const int length = std::min(size, kBox);
    // (centered on the summary line, or at its near end when the line is shorter than the button)
    return box(across + (m_columns ? 0 : 0), start + (size - length) / 2, length < kBox ? length : kBox).adjusted(0, 0, 0, 0);
}

QRect OutlineGutter::levelButton(int level) const
{
    // (the buttons for the levels are in the band before the first line: along the top of the gutter beside the rows, down the left of the one above the columns)
    const int n = level - 1;
    return m_columns ? QRect(2, 2 + n * kStep, kBox, kBox) : QRect(2 + n * kStep, 2, kBox, kBox);
}

OutlineGutter::Hit OutlineGutter::hitAt(const QPoint &p) const
{
    Hit hit;
    if (!m_outline) return hit;
    for (int level = 1; level <= m_outline->deepest() + 1; ++level)
        if (levelButton(level).contains(p)) {
            hit.kind = Hit::Kind::Level;
            hit.level = level;
            return hit;
        }
    for (const crunch::Outline::Group &g : visibleGroups())
        if (groupButton(g).contains(p)) {
            hit.kind = Hit::Kind::Group;
            hit.group = g;
            return hit;
        }
    return hit;
}

void OutlineGutter::mousePressEvent(QMouseEvent *e)
{
    const Hit hit = hitAt(e->position().toPoint());
    if (hit.kind == Hit::Kind::Level) Q_EMIT levelClicked(hit.level);
    else if (hit.kind == Hit::Kind::Group) Q_EMIT groupClicked(hit.group.first, hit.group.last, hit.group.level, hit.group.collapsed, hit.group.summary);
    e->accept();
}

void OutlineGutter::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), palette().window());
    if (!m_outline || m_outline->deepest() == 0) return;
    const QColor line = palette().color(QPalette::Mid), ink = palette().color(QPalette::WindowText), face = palette().color(QPalette::Base);
    p.setPen(line);
    p.setFont(font());
    // the buttons for the levels
    for (int level = 1; level <= m_outline->deepest() + 1; ++level) {
        const QRect r = levelButton(level);
        p.setBrush(face);
        p.setPen(line);
        p.drawRect(r.adjusted(0, 0, -1, -1));
        p.setPen(ink);
        p.drawText(r, Qt::AlignCenter, QString::number(level));
    }
    // a bracket for each group, along the lines of each part of the grid that it has, and its button
    std::vector<crunch::Outline::Group> drawn;
    for (const auto &range : visibleRanges()) {
        for (const crunch::Outline::Group &g : groupsIn(range.first, range.second)) {
            const int across = 2 + (g.level - 1) * kStep + kStep / 2;
            if (!g.collapsed) {
                int a = 0, b = 0, size = 0;
                bool any = false;
                for (quint32 i = std::max(g.first, range.first); i <= std::min(g.last, range.second); ++i) {
                    int start = 0;
                    if (!lineSpan(i, &start, &size)) continue;
                    if (!any) a = start;
                    b = start + size;
                    any = true;
                }
                if (any) {
                    p.setPen(QPen(line, 1));
                    if (m_columns) p.drawLine(a, across, b, across);
                    else p.drawLine(across, a, across, b);
                }
            }
            const bool seen = std::any_of(drawn.begin(), drawn.end(), [&](const crunch::Outline::Group &o) { return o.level == g.level && o.first == g.first; });
            if (seen) continue;
            drawn.push_back(g);
            const QRect r = groupButton(g);
            if (r.isEmpty()) continue;
            p.setBrush(face);
            p.setPen(line);
            p.drawRect(r.adjusted(0, 0, -1, -1));
            p.setPen(ink);
            const QPoint c = r.center();
            p.drawLine(c.x() - 3, c.y(), c.x() + 3, c.y());   // the bar of a minus
            if (g.collapsed) p.drawLine(c.x(), c.y() - 3, c.x(), c.y() + 3);   // the cross-bar of a plus
        }
    }
}

} // namespace jp
