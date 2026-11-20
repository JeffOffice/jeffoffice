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
// The most characters of a note that are laid out and drawn (the note that appears when the pointer rests on its cell is cut at the same place).
constexpr int kMaxShownNoteChars = 4000;
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
    // (what is drawn of a note is cut, as the note that appears beside a cell is: a note of millions of characters is not laid out)
    for (ShownNote &n : notes)
        if (n.text.size() > kMaxShownNoteChars) n.text = n.text.left(kMaxShownNoteChars) + QChar(0x2026);
    const auto sameNote = [](const ShownNote &a, const ShownNote &b) { return a.row == b.row && a.col == b.col && a.text == b.text && a.placed == b.placed && a.box == b.box; };
    if (notes.size() == m_shownNotes.size() && std::equal(notes.begin(), notes.end(), m_shownNotes.begin(), sameNote)) return;   // (the same notes again: nothing to draw)
    m_shownNotes = std::move(notes);
    m_shownSizes.assign(m_shownNotes.size(), QSize());
    if (m_pickRow >= 0 && shownIndexAt(m_pickRow, m_pickCol) < 0) m_pickRow = m_pickCol = -1;
    viewport()->update();
}

int GridView::shownIndexAt(qint64 row, qint64 col) const
{
    for (size_t i = 0; i < m_shownNotes.size(); ++i)
        if (m_shownNotes[i].row == row && m_shownNotes[i].col == col) return int(i);
    return -1;
}

QSize GridView::shownSize(size_t index) const
{
    const GridLayout &L = m_sheet.layout();
    if (m_shownZoom != L.zoom() || m_shownSizes.size() != m_shownNotes.size()) {
        m_shownZoom = L.zoom();
        m_shownSizes.assign(m_shownNotes.size(), QSize());
    }
    QSize &size = m_shownSizes[index];
    if (!size.isValid()) {
        QFont font = viewport()->font();
        font.setPointSizeF(std::max(7.0, 9.0 * L.zoom()));
        const QFontMetrics fm(font);
        const int width = int(160 * L.zoom());
        QRect text = fm.boundingRect(QRect(0, 0, width, 100000), Qt::TextWordWrap, m_shownNotes[index].text);
        text.setHeight(std::min(text.height(), int(200 * L.zoom())));
        size = QSize(text.width() + 10, text.height() + 8);
        ++m_shownLayouts;
    }
    return size;
}

bool GridView::shownBoxRect(size_t index, QRect *out) const
{
    const GridLayout &L = m_sheet.layout();
    const ShownNote &n = m_shownNotes[index];
    bool visible = false;
    for (Pane p : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
        const PaneView v = L.pane(p);
        if (!v.isEmpty() && n.row >= v.firstRow && n.row <= v.lastRow && n.col >= v.firstCol && n.col <= v.lastCol) visible = true;
    }
    if (!visible) return false;
    const QRect cell = cellRectInLayout(n.row, n.col);
    if (cell.isEmpty()) return false;
    if (n.placed) {
        const double z = L.zoom();
        *out = QRect(cell.left() + int(std::lround(n.box.x() * z)), cell.top() + int(std::lround(n.box.y() * z)), std::max(1, int(std::lround(n.box.width() * z))), std::max(1, int(std::lround(n.box.height() * z))));
    } else {
        const QSize size = shownSize(index);
        *out = QRect(cell.right() + int(6 * L.zoom()), cell.top(), size.width(), size.height());
    }
    return true;
}

QRect GridView::shownNoteRect(qint64 row, qint64 col) const
{
    const int i = shownIndexAt(row, col);
    QRect r;
    if (i < 0 || !shownBoxRect(size_t(i), &r)) return QRect();
    if (m_pickRow == row && m_pickCol == col && (m_drag == Drag::NoteMove || m_drag == Drag::NoteResize)) return mirrored() ? mirror(m_noteRectNow) : m_noteRectNow;
    return mirrored() ? mirror(r) : r;
}

void GridView::pickShownNote(qint64 row, qint64 col)
{
    if (row == m_pickRow && col == m_pickCol) return;
    m_pickRow = row;
    m_pickCol = col;
    if (m_pickRow >= 0 && shownIndexAt(row, col) < 0) m_pickRow = m_pickCol = -1;
    viewport()->update();
}

QRect GridView::noteHandleRect(int handle) const
{
    if (m_pickRow < 0 || handle < 0 || handle > 7) return QRect();
    const int i = shownIndexAt(m_pickRow, m_pickCol);
    QRect r;
    if (i < 0 || !shownBoxRect(size_t(i), &r)) return QRect();
    if (m_drag == Drag::NoteMove || m_drag == Drag::NoteResize) r = m_noteRectNow;
    constexpr int kSize = 7;
    const int x[3] = {r.left(), r.left() + r.width() / 2, r.right()}, y[3] = {r.top(), r.top() + r.height() / 2, r.bottom()};
    static const int xi[8] = {0, 1, 2, 2, 2, 1, 0, 0}, yi[8] = {0, 0, 0, 1, 2, 2, 2, 1};
    const QRect out(x[xi[handle]] - kSize / 2, y[yi[handle]] - kSize / 2, kSize, kSize);
    return mirrored() ? mirror(out) : out;
}

bool GridView::noteBoxPress(const QPoint &lp)
{
    // (a press is in layout coordinates, as the boxes are)
    if (m_pickRow >= 0) {
        const int i = shownIndexAt(m_pickRow, m_pickCol);
        QRect box;
        if (i >= 0 && shownBoxRect(size_t(i), &box)) {
            constexpr int kSize = 7;
            const int x[3] = {box.left(), box.left() + box.width() / 2, box.right()}, y[3] = {box.top(), box.top() + box.height() / 2, box.bottom()};
            static const int xi[8] = {0, 1, 2, 2, 2, 1, 0, 0}, yi[8] = {0, 0, 0, 1, 2, 2, 2, 1};
            for (int h = 0; h < 8; ++h) {
                if (QRect(x[xi[h]] - kSize / 2 - 1, y[yi[h]] - kSize / 2 - 1, kSize + 2, kSize + 2).contains(lp)) {
                    m_drag = Drag::NoteResize;
                    m_noteHandle = h;
                    m_noteFrom = lp;
                    m_noteRect0 = m_noteRectNow = box;
                    m_noteMoved = false;
                    viewport()->update();
                    return true;
                }
            }
        }
    }
    for (size_t k = m_shownNotes.size(); k-- > 0;) {
        QRect box;
        if (!shownBoxRect(k, &box) || !box.adjusted(-2, -2, 2, 2).contains(lp)) continue;
        m_pickRow = m_shownNotes[k].row;
        m_pickCol = m_shownNotes[k].col;
        m_drag = Drag::NoteMove;
        m_noteHandle = -1;
        m_noteFrom = lp;
        m_noteRect0 = m_noteRectNow = box;
        m_noteMoved = false;
        viewport()->update();
        return true;
    }
    if (m_pickRow >= 0) {
        m_pickRow = m_pickCol = -1;
        viewport()->update();
    }
    return false;
}

void GridView::noteBoxDrag(const QPoint &lp)
{
    const QPoint d = lp - m_noteFrom;
    if (!m_noteMoved && std::abs(d.x()) < 3 && std::abs(d.y()) < 3) return;
    m_noteMoved = true;
    QRect r = m_noteRect0;
    constexpr int kLeast = 24, kLeastHigh = 16;
    if (m_drag == Drag::NoteMove) {
        r.translate(d);
    } else {
        const int h = m_noteHandle;
        if (h == 0 || h == 6 || h == 7) r.setLeft(std::min(r.left() + d.x(), r.right() - kLeast + 1));
        if (h == 2 || h == 3 || h == 4) r.setRight(std::max(r.right() + d.x(), r.left() + kLeast - 1));
        if (h == 0 || h == 1 || h == 2) r.setTop(std::min(r.top() + d.y(), r.bottom() - kLeastHigh + 1));
        if (h == 4 || h == 5 || h == 6) r.setBottom(std::max(r.bottom() + d.y(), r.top() + kLeastHigh - 1));
    }
    if (r != m_noteRectNow) {
        m_noteRectNow = r;
        viewport()->update();
    }
}

void GridView::noteBoxRelease()
{
    const bool changed = m_noteMoved && m_noteRectNow != m_noteRect0;
    const QRect now = m_noteRectNow;
    m_drag = Drag::None;
    m_noteMoved = false;
    viewport()->update();
    if (!changed || m_pickRow < 0) return;
    const QRect cell = cellRectInLayout(m_pickRow, m_pickCol);
    if (cell.isEmpty()) return;
    const double z = m_sheet.layout().zoom();
    const QRect rel(int(std::lround((now.left() - cell.left()) / z)), int(std::lround((now.top() - cell.top()) / z)), std::max(1, int(std::lround(now.width() / z))), std::max(1, int(std::lround(now.height() / z))));
    Q_EMIT noteBoxRequested(m_pickRow, m_pickCol, rel);
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
    for (size_t index = 0; index < m_shownNotes.size(); ++index) {
        const ShownNote &n = m_shownNotes[index];
        if (boxes >= kMaxBoxes) break;
        QRect box;
        if (!shownBoxRect(index, &box)) continue;
        if (n.row == m_pickRow && n.col == m_pickCol && (m_drag == Drag::NoteMove || m_drag == Drag::NoteResize)) box = m_noteRectNow;
        if (!box.adjusted(-5, -5, 5, 5).intersects(inLayout)) continue;
        ++boxes;
        painter->setPen(QPen(QColor(0x80, 0x80, 0x60), 1));
        painter->setBrush(QColor(0xFF, 0xFF, 0xE1));
        painter->drawRect(box);
        painter->setPen(QColor(0x20, 0x20, 0x20));
        painter->save();
        painter->setClipRect(box.adjusted(1, 1, -1, -1), Qt::IntersectClip);
        painter->drawText(box.adjusted(5, 4, -5, -4), Qt::TextWordWrap, n.text);
        painter->restore();
        if (n.row == m_pickRow && n.col == m_pickCol) {
            painter->setPen(QPen(QColor(0x40, 0x40, 0x40), 1));
            painter->setBrush(Qt::white);
            const int hx[3] = {box.left(), box.left() + box.width() / 2, box.right()}, hy[3] = {box.top(), box.top() + box.height() / 2, box.bottom()};
            static const int xi[8] = {0, 1, 2, 2, 2, 1, 0, 0}, yi[8] = {0, 0, 0, 1, 2, 2, 2, 1};
            for (int h = 0; h < 8; ++h) painter->drawRect(QRect(hx[xi[h]] - 3, hy[yi[h]] - 3, 6, 6));
        }
    }
    painter->restore();
}

} // namespace jp::grid
