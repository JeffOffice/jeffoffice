// The grid view's notes: a small mark in the corner of each cell that has one, and the notes that are shown all the time, drawn in a box beside their cells.

#include "grid/gridview.h"


#include <QFontMetrics>
#include <QPainter>
#include <QPolygonF>
#include <QTextOption>

#include <algorithm>

namespace jp::grid {

namespace {
// The most boxes drawn in one paint (the ones in view; a sheet can show thousands of notes, and a paint should cost the ones a person can read).
constexpr int kMaxBoxes = 200;
}

void GridView::setNoteCells(std::vector<quint64> sortedKeys)
{
    if (sortedKeys == m_noteCells) return;
    m_noteCells = std::move(sortedKeys);
    viewport()->update();
}

void GridView::setShownNotes(std::vector<ShownNote> notes)
{
    if (notes.size() > 5000) notes.resize(5000);
    m_shownNotes = std::move(notes);
    viewport()->update();
}

void GridView::paintNotes(QPainter *painter, const QRect &clip)
{
    if (m_noteCells.empty() && m_shownNotes.empty()) return;
    const GridLayout &L = m_sheet.layout();
    const QRect inLayout = mirror(clip);
    painter->save();
    if (mirrored()) {
        painter->translate(viewport()->width(), 0);
        painter->scale(-1, 1);
    }
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0xC0, 0x1E, 0x1E));
    for (Pane p : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
        const PaneView v = L.pane(p);
        if (v.isEmpty()) continue;
        // (the notes of the rows in view: found in the sorted list, not by looking at every note)
        auto it = std::lower_bound(m_noteCells.begin(), m_noteCells.end(), quint64(std::max<qint64>(0, v.firstRow)) << 32);
        const quint64 end = (quint64(std::max<qint64>(0, v.lastRow)) + 1) << 32;
        for (; it != m_noteCells.end() && *it < end; ++it) {
            const qint64 row = qint64(*it >> 32), col = qint64(*it & 0xFFFFFFFFu);
            if (col < v.firstCol || col > v.lastCol) continue;
            const QRect cell = L.rangeRectIn(p, row, col, row, col) & v.rect;
            if (cell.isEmpty() || !cell.intersects(inLayout)) continue;
            const int size = std::clamp(cell.height() / 3, 4, 8);
            painter->drawPolygon(QPolygonF({QPointF(cell.right() + 1 - size, cell.top()), QPointF(cell.right() + 1, cell.top()), QPointF(cell.right() + 1, cell.top() + size)}));
        }
    }
    // the notes that are shown
    painter->setRenderHint(QPainter::Antialiasing, false);
    int boxes = 0;
    QFont font = painter->font();
    font.setPointSizeF(std::max(7.0, 9.0 * L.zoom()));
    painter->setFont(font);
    const QFontMetrics fm(font);
    for (const ShownNote &n : m_shownNotes) {
        if (boxes >= kMaxBoxes) break;
        bool visible = false;
        for (Pane p : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
            const PaneView v = L.pane(p);
            if (!v.isEmpty() && n.row >= v.firstRow && n.row <= v.lastRow && n.col >= v.firstCol && n.col <= v.lastCol) visible = true;
        }
        if (!visible) continue;
        const QRect cell = cellRectInLayout(n.row, n.col);
        if (cell.isEmpty()) continue;
        const int width = int(160 * L.zoom());
        QRect text = fm.boundingRect(QRect(0, 0, width, 100000), Qt::TextWordWrap, n.text);
        text.setHeight(std::min(text.height(), int(200 * L.zoom())));
        QRect box(cell.right() + int(6 * L.zoom()), cell.top(), text.width() + 10, text.height() + 8);
        if (!box.intersects(inLayout)) continue;
        ++boxes;
        painter->setPen(QPen(QColor(0x80, 0x80, 0x60), 1));
        painter->setBrush(QColor(0xFF, 0xFF, 0xE1));
        painter->drawRect(box);
        painter->setPen(QColor(0x20, 0x20, 0x20));
        painter->drawText(box.adjusted(5, 4, -5, -4), Qt::TextWordWrap, n.text);
    }
    painter->restore();
}

} // namespace jp::grid
