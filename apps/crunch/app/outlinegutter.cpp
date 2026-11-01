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

QPair<quint32, quint32> OutlineGutter::visibleLines() const
{
    const grid::GridLayout &L = m_view->sheet().layout();
    quint32 first = 0xFFFFFFFFu, last = 0;
    for (grid::Pane which : m_columns ? std::initializer_list<grid::Pane>{grid::Pane::Left, grid::Pane::Body} : std::initializer_list<grid::Pane>{grid::Pane::Top, grid::Pane::Body}) {
        const grid::PaneView pv = L.pane(which);
        if (pv.isEmpty()) continue;
        first = std::min<quint32>(first, quint32(m_columns ? pv.firstCol : pv.firstRow));
        last = std::max<quint32>(last, quint32(m_columns ? pv.lastCol : pv.lastRow));
    }
    return {first, last};
}

bool OutlineGutter::lineSpan(quint32 index, int *start, int *size) const
{
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
    if (!m_outline) return {};
    const auto lines = visibleLines();
    if (lines.first > lines.second) return {};
    return m_outline->groups(lines.first, lines.second);
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
    // a bracket for each group, and its button
    for (const crunch::Outline::Group &g : visibleGroups()) {
        const int across = 2 + (g.level - 1) * kStep + kStep / 2;
        if (!g.collapsed) {
            // the line along the lines of the group that are in view, as far as the cells are
            int a = 0, b = 0, size = 0;
            bool any = false;
            for (quint32 i = std::max(g.first, visibleLines().first); i <= std::min(g.last, visibleLines().second); ++i) {
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

} // namespace jp
