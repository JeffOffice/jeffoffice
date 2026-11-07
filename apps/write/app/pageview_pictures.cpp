// The view's pictures: selecting one, the handles that change its size, dragging it to another place, and the pictures that come in from the clipboard and from a drop. The work on
// the document is core/pictureops.h; what is here is the pointer, and the drawing of the handles.

#include "app/icons.h"
#include "app/pageview.h"
#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QFontMetricsF>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QUrl>
#include <algorithm>
#include <cmath>
#include <optional>

namespace jp::write {

namespace {

constexpr double kHandleSize = 8;      // pixels
constexpr double kDragStart = 3;       // pixels a drag must go before it is a drag and not a click

QString imageFileEnding(const QString &suffix)
{
    static const QStringList endings = {QStringLiteral("png"), QStringLiteral("jpg"), QStringLiteral("jpeg"), QStringLiteral("gif"), QStringLiteral("bmp"), QStringLiteral("tif"),
                                        QStringLiteral("tiff"), QStringLiteral("webp"), QStringLiteral("svg")};
    return endings.contains(suffix.toLower()) ? suffix.toLower() : QString();
}

} // namespace

int PageView::selectedPicture() const
{
    if (!m_doc || m_cursor.isNull() || m_cursor.document() != m_doc->story() || !m_cursor.hasSelection()) return -1;
    const int from = m_cursor.selectionStart();
    if (m_cursor.selectionEnd() != from + 1) return -1;
    return isDrawnObjectAt(m_doc->story(), from) ? from : -1;
}

void PageView::selectPicture(int position)
{
    if (!m_doc || m_hf.active || m_note.active || !isDrawnObjectAt(m_doc->story(), position)) return;
    m_moreObjects.clear();
    m_cursor.setPosition(position);
    m_cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
    caretMoved();
}

QVector<int> PageView::selectedObjects() const
{
    const int first = selectedPicture();
    if (first < 0) return {};
    QVector<int> out{first};
    if (m_doc && m_moreObjectsRevision == m_doc->story()->revision())
        for (const int p : m_moreObjects)
            if (p != first && isDrawnObjectAt(m_doc->story(), p)) out.push_back(p);
    return out;
}

bool PageView::toggleObjectInSelection(int position)
{
    const int first = selectedPicture();
    if (first < 0 || !m_doc || m_hf.active || m_note.active || !isDrawnObjectAt(m_doc->story(), position) || position == first) return false;
    QVector<int> now = selectedObjects();
    now.removeAll(first);
    if (now.contains(position)) now.removeAll(position);
    else now.push_back(position);
    std::sort(now.begin(), now.end());
    m_moreObjects = now;
    m_moreObjectsRevision = m_doc->story()->revision();
    viewport()->update();
    Q_EMIT caretChanged();
    return true;
}

void PageView::selectObjects(const QVector<int> &positions)
{
    if (positions.isEmpty()) return;
    selectPicture(positions.first());
    m_moreObjects = positions.mid(1);
    m_moreObjectsRevision = m_doc ? m_doc->story()->revision() : -1;
    viewport()->update();
}

bool PageView::groupSelectedObjects()
{
    if (!m_doc || m_readOnly) return false;
    const QVector<int> positions = selectedObjects();
    if (positions.size() < 2) {
        Q_EMIT notice(tr("Select two or more objects first: click one, then hold Ctrl and click the others."));
        return false;
    }
    QVector<QRectF> rects;
    int page = -1;
    for (const int p : positions) {
        QRectF onPage;
        int at = 0;
        if (!pictureAt(*m_doc, m_doc->story(), p).floating || !floatingPictureRect(p, &onPage, &at)) {
            Q_EMIT notice(tr("An object in line with the text can't be grouped. Choose another wrap for it first."));
            return false;
        }
        if (page >= 0 && at != page) {
            Q_EMIT notice(tr("Objects on different pages can't be grouped."));
            return false;
        }
        page = at;
        rects.push_back(onPage);
    }
    m_doc->pictureNotice.clear();
    int made = -1;
    if (!groupObjects(*m_doc, positions, rects, &made)) {
        Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? tr("These objects can't be grouped. A chart, a group, and an object in line with the text can't join a group.") : m_doc->pictureNotice);
        return false;
    }
    selectPicture(made);
    Q_EMIT contentChanged();
    viewport()->update();
    Q_EMIT caretChanged();
    return true;
}

bool PageView::connectSelectedObjects(const QString &preset)
{
    if (!m_doc || m_readOnly) return false;
    const QVector<int> positions = selectedObjects();
    if (positions.size() != 2) {
        Q_EMIT notice(tr("Select two shapes first: click one, then hold Ctrl and click the other."));
        return false;
    }
    QRectF rects[2];
    int pages[2] = {0, 0};
    for (int i = 0; i < 2; ++i)
        if (!pictureAt(*m_doc, m_doc->story(), positions[i]).floating || !floatingPictureRect(positions[i], &rects[i], &pages[i])) {
            Q_EMIT notice(tr("An object in line with the text can't be joined. Choose another wrap for it first."));
            return false;
        }
    if (pages[0] != pages[1]) {
        Q_EMIT notice(tr("Shapes on different pages can't be joined."));
        return false;
    }
    m_doc->pictureNotice.clear();
    int made = -1;
    if (connectObjects(*m_doc, positions[0], rects[0], positions[1], rects[1], preset, &made) != PictureResult::Ok) {
        Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? tr("These can't be joined. A line, a chart, and an object in line with the text can't be joined.") : m_doc->pictureNotice);
        return false;
    }
    selectPicture(made);
    Q_EMIT contentChanged();
    viewport()->update();
    Q_EMIT caretChanged();
    return true;
}

void PageView::rerouteGluedConnectors(const QVector<std::pair<int, QRectF>> &moved)
{
    if (!m_doc || moved.isEmpty()) return;
    rerouteConnectors(*m_doc, moved, [this](int position, QRectF *rect) {
        int page = 0;
        return floatingPictureRect(position, rect, &page);
    });
}

bool PageView::ungroupSelectedObject()
{
    const int position = selectedPicture();
    if (!m_doc || m_readOnly || position < 0 || !isGroupAt(*m_doc, position)) return false;
    QRectF onPage;
    int page = 0;
    if (!floatingPictureRect(position, &onPage, &page)) return false;
    QVector<int> parts;
    m_doc->pictureNotice.clear();
    if (!ungroupObject(*m_doc, position, onPage.topLeft(), &parts)) {
        Q_EMIT notice(tr("This group can't be taken apart: it is turned, or has another group in it."));
        return false;
    }
    selectObjects(parts);
    Q_EMIT contentChanged();
    viewport()->update();
    Q_EMIT caretChanged();
    return true;
}

bool PageView::inlinePictureRect(int position, const PictureInfo &info, QRectF *rect)
{
    int page = 0;
    QLineF line;
    if (!caretLine(position, &page, &line) || page < 0 || page >= m_rects.size()) return false;
    const double descent = QFontMetricsF(charFormatAt(m_doc->story(), position).font()).descent();
    const double s = scale();
    const QRectF pr = m_rects[page].translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
    *rect = QRectF(pr.left() + line.x1() * s, pr.top() + (line.y2() - descent - info.height) * s, info.width * s, info.height * s);
    return true;
}

namespace {
qint64 g_pictureRects = 0;
}

qint64 PageView::pictureRectsComputedForTest() { return g_pictureRects; }

bool PageView::pictureRect(int position, QRectF *rect)
{
    if (!m_doc || m_hf.active || m_note.active) return false;
    // The rectangle is worked out again when the picture, the text, the layout, the scroll, or the zoom changed (the pointer asks for it at every move).
    const RectKey key{m_doc, position, m_doc->story()->revision(), horizontalScrollBar()->value(), verticalScrollBar()->value(), m_zoom, m_layoutSerial, m_knownPages};
    if (m_rectCache.valid && m_rectCache.key == key) {
        *rect = m_rectCache.rect;
        return true;
    }
    ++g_pictureRects;
    m_rectCache.valid = false;
    if (!pictureRectUncached(position, rect)) return false;
    m_rectCache = {true, key, *rect};
    return true;
}

bool PageView::pictureRectUncached(int position, QRectF *rect)
{
    const PictureInfo info = pictureAt(*m_doc, m_doc->story(), position);
    if (!info.valid) return false;
    if (!info.floating) return inlinePictureRect(position, info, rect);
    QRectF onPage;
    int page = 0;
    if (!floatingPictureRect(position, &onPage, &page) || page >= m_rects.size()) return false;
    const double s = scale();
    const QRectF pr = m_rects[page].translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
    *rect = QRectF(pr.left() + onPage.left() * s, pr.top() + onPage.top() * s, onPage.width() * s, onPage.height() * s);
    return true;
}

bool PageView::pictureHandlePoint(PictureHandle handle, QPointF *point)
{
    const int position = selectedPicture();
    QRectF r;
    if (position < 0) return false;
    if (m_pictureDrag.active) r = m_pictureDrag.now;
    else if (!pictureRect(position, &r)) return false;
    switch (handle) {
    case PictureHandle::TopLeft: *point = r.topLeft(); return true;
    case PictureHandle::Top: *point = QPointF(r.center().x(), r.top()); return true;
    case PictureHandle::TopRight: *point = r.topRight(); return true;
    case PictureHandle::Right: *point = QPointF(r.right(), r.center().y()); return true;
    case PictureHandle::BottomRight: *point = r.bottomRight(); return true;
    case PictureHandle::Bottom: *point = QPointF(r.center().x(), r.bottom()); return true;
    case PictureHandle::BottomLeft: *point = r.bottomLeft(); return true;
    case PictureHandle::Left: *point = QPointF(r.left(), r.center().y()); return true;
    case PictureHandle::Body: *point = r.center(); return true;
    default: return false;
    }
}

PageView::PictureHandle PageView::pictureHandleAt(const QPoint &pos)
{
    if (selectedPicture() < 0) return PictureHandle::None;
    for (PictureHandle h : {PictureHandle::TopLeft, PictureHandle::TopRight, PictureHandle::BottomRight, PictureHandle::BottomLeft, PictureHandle::Top, PictureHandle::Right, PictureHandle::Bottom,
                            PictureHandle::Left}) {
        QPointF at;
        if (pictureHandlePoint(h, &at) && std::abs(pos.x() - at.x()) <= kHandleSize / 2 + 1 && std::abs(pos.y() - at.y()) <= kHandleSize / 2 + 1) return h;
    }
    QRectF r;
    if (pictureRect(selectedPicture(), &r) && r.contains(pos)) return PictureHandle::Body;
    return PictureHandle::None;
}

int PageView::pictureAtPoint(const QPoint &pos)
{
    if (!m_doc || m_hf.active || m_note.active || m_rects.isEmpty()) return -1;
    const Hit hit = hitAt(pos);
    if (hit.page < 0) return -1;
    if (const int floating = floatingPictureAt(hit); floating >= 0) return floating;
    const int at = positionAt(hit);
    for (const int p : {at, at - 1}) {
        if (p < 0 || !isDrawnObjectAt(m_doc->story(), p)) continue;
        const PictureInfo info = pictureAt(*m_doc, m_doc->story(), p);
        QRectF r;
        if (info.valid && !info.floating && inlinePictureRect(p, info, &r) && r.contains(pos)) return p;
    }
    return -1;
}

bool PageView::beginPictureDrag(QMouseEvent *e)
{
    if (!m_doc || m_readOnly || m_hf.active || m_note.active || e->button() != Qt::LeftButton) return false;
    const QPoint pos = e->position().toPoint();
    int target = selectedPicture();
    // Ctrl+click on another object adds it to the selection (or takes it out).
    if (target >= 0 && (e->modifiers() & Qt::ControlModifier)) {
        const int other = pictureAtPoint(pos);
        if (other >= 0 && other != target) {
            toggleObjectInSelection(other);
            m_pictureDrag = PictureDrag();
            return true;
        }
    }
    PictureHandle handle = target >= 0 ? pictureHandleAt(pos) : PictureHandle::None;
    if (handle == PictureHandle::None) {
        target = pictureAtPoint(pos);
        if (target < 0) return false;
        if (e->modifiers() & Qt::ShiftModifier) return false;   // (a selection that goes on past the picture)
        selectPicture(target);
        handle = PictureHandle::Body;
    }
    QRectF r;
    const PictureInfo info = pictureAt(*m_doc, m_doc->story(), target);
    if (!pictureRect(target, &r)) return true;
    m_pictureDrag = PictureDrag();
    if (!info.editable) return true;   // (selected, not changed)
    m_pictureDrag.active = true;
    m_pictureDrag.handle = handle;
    m_pictureDrag.position = target;
    m_pictureDrag.from = pos;
    m_pictureDrag.scrollAtStart = QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
    m_pictureDrag.start = m_pictureDrag.now = r;
    m_pictureDrag.floating = info.floating;
    m_pictureDrag.info = info;
    m_pictureDragPage = -1;
    if (info.floating) {
        QRectF onPage;
        floatingPictureRect(target, &onPage, &m_pictureDragPage);
    }
    return true;
}

void PageView::updatePictureDrag(const QPoint &pos)
{
    PictureDrag &d = m_pictureDrag;
    if (!d.active) return;
    const QPointF delta = QPointF(pos) - d.from;
    if (!d.moved && std::abs(delta.x()) < kDragStart && std::abs(delta.y()) < kDragStart) return;
    d.moved = true;
    const double s = scale(), least = kLeastPictureSize * s;
    QRectF r = d.start;
    if (d.handle == PictureHandle::Body) {
        r.translate(delta);
    } else {
        const bool left = d.handle == PictureHandle::Left || d.handle == PictureHandle::TopLeft || d.handle == PictureHandle::BottomLeft;
        const bool right = d.handle == PictureHandle::Right || d.handle == PictureHandle::TopRight || d.handle == PictureHandle::BottomRight;
        const bool top = d.handle == PictureHandle::Top || d.handle == PictureHandle::TopLeft || d.handle == PictureHandle::TopRight;
        const bool bottom = d.handle == PictureHandle::Bottom || d.handle == PictureHandle::BottomLeft || d.handle == PictureHandle::BottomRight;
        if ((left || right) && (top || bottom) && d.info.kind == DrawnKind::Picture) {
            // A corner keeps the picture's proportions: the side that moved the farther, in proportion, sets the size.
            const double kx = (d.start.width() + (right ? delta.x() : -delta.x())) / d.start.width();
            const double ky = (d.start.height() + (bottom ? delta.y() : -delta.y())) / d.start.height();
            double k = std::abs(kx - 1) > std::abs(ky - 1) ? kx : ky;
            k = std::max(k, std::max(least / d.start.width(), least / d.start.height()));
            k = std::min(k, kMostPictureSize * s / std::max(d.start.width(), d.start.height()));   // (and no larger than a picture may be)
            const QSizeF size(d.start.width() * k, d.start.height() * k);
            const QPointF fixed(left ? d.start.right() : d.start.left(), top ? d.start.bottom() : d.start.top());
            r = QRectF(QPointF(left ? fixed.x() - size.width() : fixed.x(), top ? fixed.y() - size.height() : fixed.y()), size);
        } else {
            // (A shape or text box is free to take any proportions at a corner too.)
            if (left) r.setLeft(std::max(std::min(d.start.left() + delta.x(), d.start.right() - least), d.start.right() - kMostPictureSize * s));
            if (right) r.setRight(std::min(std::max(d.start.right() + delta.x(), d.start.left() + least), d.start.left() + kMostPictureSize * s));
            if (top) r.setTop(std::max(std::min(d.start.top() + delta.y(), d.start.bottom() - least), d.start.bottom() - kMostPictureSize * s));
            if (bottom) r.setBottom(std::min(std::max(d.start.bottom() + delta.y(), d.start.top() + least), d.start.top() + kMostPictureSize * s));
        }
    }
    d.now = r;
    viewport()->update();
}

void PageView::endPictureDrag(const QPoint &pos)
{
    if (!m_pictureDrag.active) return;
    updatePictureDrag(pos);
    const PictureDrag d = m_pictureDrag;
    m_pictureDrag = PictureDrag();
    viewport()->update();
    if (!d.moved || !m_doc) return;
    const double s = scale();
    m_doc->pictureNotice.clear();
    bool changed = false;
    int selectAt = d.position;
    if (d.handle == PictureHandle::Body) {
        if (d.floating) {
            if (m_pictureDragPage >= 0 && m_pictureDragPage < m_rects.size()) {
                const QRectF pr = m_rects[m_pictureDragPage].translated(-d.scrollAtStart.x(), -d.scrollAtStart.y());
                // The others selected with it move the same way, in the same step.
                const QVector<int> together = selectedObjects();
                QVector<std::pair<int, QRectF>> others;
                for (const int p : together) {
                    QRectF onPage;
                    int otherPage = 0;
                    if (p != d.position && pictureAt(*m_doc, m_doc->story(), p).floating && floatingPictureRect(p, &onPage, &otherPage)) others.push_back({p, onPage});
                }
                const WriteDoc::StepGroup step(*m_doc);
                const QRectF startOnPage((d.start.left() - pr.left()) / s, (d.start.top() - pr.top()) / s, d.start.width() / s, d.start.height() / s);
                changed = setPicturePlace(*m_doc, d.position, (d.now.left() - pr.left()) / s, (d.now.top() - pr.top()) / s, !others.isEmpty());
                const QPointF delta((d.now.left() - d.start.left()) / s, (d.now.top() - d.start.top()) / s);
                for (const auto &o : others) setPicturePlace(*m_doc, o.first, o.second.left() + delta.x(), o.second.top() + delta.y(), true);
                if (changed) {
                    QVector<std::pair<int, QRectF>> moved{{d.position, startOnPage.translated(delta)}};
                    for (const auto &o : others) moved.push_back({o.first, o.second.translated(delta)});
                    rerouteGluedConnectors(moved);
                }
                if (changed && !others.isEmpty()) {
                    QVector<int> keep = together;
                    keep.removeAll(d.position);
                    keep.push_front(d.position);
                    selectPicture(d.position);
                    m_moreObjects = keep.mid(1);
                    m_moreObjectsRevision = -2;   // (set again below, once the text has changed)
                }
            }
        } else {
            const int to = positionAt(hitAt(pos));
            if (to >= 0 && movePictureInText(*m_doc, d.position, to)) {
                changed = true;
                selectAt = to > d.position ? to - 1 : to;
            }
        }
    } else {
        std::optional<QPointF> place;
        const bool movesLeft = d.handle == PictureHandle::Left || d.handle == PictureHandle::TopLeft || d.handle == PictureHandle::BottomLeft;
        const bool movesTop = d.handle == PictureHandle::Top || d.handle == PictureHandle::TopLeft || d.handle == PictureHandle::TopRight;
        if (d.floating && (movesLeft || movesTop) && m_pictureDragPage >= 0 && m_pictureDragPage < m_rects.size()) {
            const QRectF pr = m_rects[m_pictureDragPage].translated(-d.scrollAtStart.x(), -d.scrollAtStart.y());
            place = QPointF((d.now.left() - pr.left()) / s, (d.now.top() - pr.top()) / s);
        }
        const WriteDoc::StepGroup step(*m_doc);
        changed = setPictureBox(*m_doc, d.position, d.now.width() / s, d.now.height() / s, place);
        if (changed && d.floating && m_pictureDragPage >= 0 && m_pictureDragPage < m_rects.size()) {
            const QRectF pr = m_rects[m_pictureDragPage].translated(-d.scrollAtStart.x(), -d.scrollAtStart.y());
            rerouteGluedConnectors({{d.position, QRectF((d.now.left() - pr.left()) / s, (d.now.top() - pr.top()) / s, d.now.width() / s, d.now.height() / s)}});
        }
    }
    if (!changed && !m_doc->pictureNotice.isEmpty()) Q_EMIT notice(m_doc->pictureNotice);
    if (changed) {
        const bool keepOthers = m_moreObjectsRevision == -2;
        const QVector<int> others = m_moreObjects;
        selectPicture(selectAt);
        if (keepOthers) {
            m_moreObjects = others;
            m_moreObjectsRevision = m_doc->story()->revision();
        }
        Q_EMIT contentChanged();
    }
    m_moreObjectsRevision = m_moreObjectsRevision == -2 ? -1 : m_moreObjectsRevision;
    Q_EMIT caretChanged();
}

void PageView::paintPictureHandles(QPainter &p)
{
    const int position = selectedPicture();
    if (position < 0 || m_hf.active || m_note.active) return;
    QRectF r;
    if (!pictureRect(position, &r)) return;
    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    const QColor accent = uiAccent();
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(accent, 1.5));
    p.drawRect(r);
    for (const int other : selectedObjects()) {
        QRectF o;
        if (other != position && pictureRect(other, &o)) p.drawRect(o);
    }
    if (m_pictureDrag.active && m_pictureDrag.moved) {
        p.setPen(QPen(accent, 1.5, Qt::DashLine));
        p.drawRect(m_pictureDrag.now);
    }
    const PictureInfo info = pictureAt(*m_doc, m_doc->story(), position);
    if (info.editable) {
        const QRectF b = m_pictureDrag.active ? m_pictureDrag.now : r;
        p.setBrush(Qt::white);
        p.setPen(QPen(accent, 1));
        for (const QPointF &c : {b.topLeft(), QPointF(b.center().x(), b.top()), b.topRight(), QPointF(b.right(), b.center().y()), b.bottomRight(), QPointF(b.center().x(), b.bottom()), b.bottomLeft(),
                                  QPointF(b.left(), b.center().y())})
            p.drawRect(QRectF(c.x() - kHandleSize / 2, c.y() - kHandleSize / 2, kHandleSize, kHandleSize));
    }
    p.restore();
}

// ---- inserting

bool PageView::insertPictureBytes(const QByteArray &bytes, const QString &name, PictureWrap wrap)
{
    if (!m_doc) return false;
    m_doc->pictureNotice.clear();
    QTextCursor c = m_cursor;
    const PictureResult result = insertPicture(*m_doc, c, bytes, name, wrap);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(c.position() - 1);
        Q_EMIT contentChanged();
        return true;
    }
    Q_EMIT notice(result == PictureResult::Unreadable ? QCoreApplication::translate("jp::write::Pictures", "That is not a picture JeffWrite can read.")
                  : result == PictureResult::External ? QCoreApplication::translate("jp::write::Pictures", "That SVG picture refers to files outside itself, so JeffWrite does not put it in a document.")
                  : result == PictureResult::TooLarge ? (m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "That picture is too large to put in a document.") : m_doc->pictureNotice)
                                                      : m_doc->pictureNotice);
    return false;
}

bool PageView::insertPictureImage(const QImage &image, PictureWrap wrap)
{
    if (!m_doc) return false;
    m_doc->pictureNotice.clear();
    QTextCursor c = m_cursor;
    const PictureResult result = write::insertPictureImage(*m_doc, c, image, wrap);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(c.position() - 1);
        Q_EMIT contentChanged();
        return true;
    }
    Q_EMIT notice(result == PictureResult::Unreadable ? QCoreApplication::translate("jp::write::Pictures", "There is no picture on the clipboard that JeffWrite can read.")
                  : result == PictureResult::TooLarge ? (m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "That picture is too large to put in a document.") : m_doc->pictureNotice)
                                                      : m_doc->pictureNotice);
    return false;
}

bool PageView::insertShapeObject(const QString &preset)
{
    if (!m_doc) return false;
    m_doc->pictureNotice.clear();
    QTextCursor c = m_cursor;
    const PictureResult result = insertShape(*m_doc, c, preset);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(c.position() - 1);
        Q_EMIT contentChanged();
        return true;
    }
    Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "That shape can't be put here.") : m_doc->pictureNotice);
    return false;
}

bool PageView::insertDrawingCanvasObject()
{
    if (!m_doc) return false;
    m_doc->pictureNotice.clear();
    QTextCursor c = m_cursor;
    const PictureResult result = insertDrawingCanvas(*m_doc, c);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(c.position() - 1);
        Q_EMIT contentChanged();
        return true;
    }
    Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "A drawing canvas can't be put here.") : m_doc->pictureNotice);
    return false;
}

bool PageView::insertTextArtObject(const QString &styleId)
{
    if (!m_doc) return false;
    m_doc->pictureNotice.clear();
    QTextCursor c = m_cursor;
    TextArtSpec spec;
    spec.styleId = styleId;
    spec.text = QCoreApplication::translate("jp::write::Pictures", "Your Text Here");
    const PictureResult result = insertTextArt(*m_doc, c, spec);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(c.position() - 1);
        Q_EMIT contentChanged();
        return true;
    }
    Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "That Text Art can't be put here.") : m_doc->pictureNotice);
    return false;
}

bool PageView::insertTextBoxObject(const TextBoxDesign &design)
{
    if (!m_doc) return false;
    m_doc->pictureNotice.clear();
    QTextCursor c = m_cursor;
    const PictureResult result = insertTextBox(*m_doc, c, design);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(c.position() - 1);
        Q_EMIT contentChanged();
        return true;
    }
    Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "A text box can't be put here.") : m_doc->pictureNotice);
    return false;
}

void PageView::beginTextBoxLink()
{
    const int from = selectedPicture();
    if (!m_doc || m_readOnly || from < 0 || !canLinkTextBoxFrom(*m_doc, from)) return;
    m_linkFrom = from;
    viewport()->setCursor(Qt::PointingHandCursor);
}

bool PageView::breakTextBoxLink()
{
    return changeSelectedPicture([](WriteDoc &doc, int position) { return unlinkTextBox(doc, position); });
}

bool PageView::insertChartObject(ChartForm form)
{
    if (!m_doc) return false;
    m_doc->pictureNotice.clear();
    QTextCursor c = m_cursor;
    const PictureResult result = insertChart(*m_doc, c, form);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(c.position() - 1);
        Q_EMIT contentChanged();
        return true;
    }
    Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "A chart can't be put here.") : m_doc->pictureNotice);
    return false;
}

bool PageView::changeSelectedPicture(const std::function<bool(WriteDoc &, int)> &change)
{
    const int position = selectedPicture();
    if (position < 0 || !m_doc || m_readOnly) return false;
    m_doc->pictureNotice.clear();
    const bool changed = change(*m_doc, position);
    if (!changed && !m_doc->pictureNotice.isEmpty()) Q_EMIT notice(m_doc->pictureNotice);
    if (changed) {
        selectPicture(position);
        Q_EMIT contentChanged();
    }
    viewport()->update();
    Q_EMIT caretChanged();
    return changed;
}

// ---- dropping picture files

namespace {
bool wantsDrop(const QMimeData *mime)
{
    if (!mime || !mime->hasUrls()) return false;
    for (const QUrl &u : mime->urls())
        if (u.isLocalFile() && !imageFileEnding(QFileInfo(u.toLocalFile()).suffix()).isEmpty()) return true;
    return false;
}
} // namespace

void PageView::dragEnterEvent(QDragEnterEvent *e)
{
    m_documentDrag = false;
    if (m_doc && (wantsDrop(e->mimeData()) || wantsTextDrop(e->mimeData()))) {
        e->acceptProposedAction();
        return;
    }
    // (A document file is the window's to open.)
    m_documentDrag = !documentFilesIn(e->mimeData()).isEmpty();
    if (m_documentDrag) e->acceptProposedAction();
    else e->ignore();
}

QStringList PageView::documentFilesIn(const QMimeData *mime)
{
    static const QStringList kinds = {QStringLiteral("docx"), QStringLiteral("docm"), QStringLiteral("dotx"), QStringLiteral("dotm"), QStringLiteral("doc"), QStringLiteral("dot"), QStringLiteral("rtf")};
    QStringList out;
    if (!mime || !mime->hasUrls()) return out;
    for (const QUrl &u : mime->urls())
        if (u.isLocalFile() && kinds.contains(QFileInfo(u.toLocalFile()).suffix().toLower()) && out.size() < 20) out << u.toLocalFile();
    return out;
}

void PageView::dragMoveEvent(QDragMoveEvent *e)
{
    if (m_documentDrag) {
        e->acceptProposedAction();
        return;
    }
    if (!m_doc || !(wantsDrop(e->mimeData()) || wantsTextDrop(e->mimeData()))) {
        e->ignore();
        return;
    }
    e->acceptProposedAction();
    // Near the top or the bottom edge of the window the pages scroll, so that the text can be dropped farther along.
    if (wantsTextDrop(e->mimeData())) {
        const int y = e->position().toPoint().y(), edge = 28;
        if (y < edge) verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->singleStep());
        else if (y > viewport()->height() - edge) verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->singleStep());
    }
}

void PageView::dropEvent(QDropEvent *e)
{
    if (m_documentDrag) {
        m_documentDrag = false;
        e->acceptProposedAction();
        Q_EMIT documentsDropped(documentFilesIn(e->mimeData()));
        return;
    }
    if (dropText(e)) return;
    if (!m_doc || m_readOnly || !wantsDrop(e->mimeData())) {
        e->ignore();
        return;
    }
    const int at = positionAt(hitAt(e->position().toPoint()));
    if (at >= 0 && !m_hf.active && !m_note.active) {
        m_cursor.setPosition(at);
        caretMoved();
    }
    bool any = false;
    int inserted = 0;
    for (const QUrl &u : e->mimeData()->urls()) {
        if (!u.isLocalFile() || imageFileEnding(QFileInfo(u.toLocalFile()).suffix()).isEmpty() || inserted >= 20) continue;
        QByteArray bytes;
        const PictureResult read = readPictureFile(u.toLocalFile(), &bytes);
        if (read != PictureResult::Ok) {
            Q_EMIT notice(read == PictureResult::TooLarge ? QCoreApplication::translate("jp::write::Pictures", "That picture is too large to put in a document.")
                                                          : QCoreApplication::translate("jp::write::Pictures", "%1 can't be opened as a picture.").arg(QFileInfo(u.toLocalFile()).fileName()));
            continue;
        }
        any = insertPictureBytes(bytes, QFileInfo(u.toLocalFile()).completeBaseName()) || any;
        m_cursor.setPosition(m_cursor.selectionEnd());   // (the next one goes after this one)
        ++inserted;
    }
    if (any) e->acceptProposedAction();
    else e->ignore();
}

} // namespace jp::write
