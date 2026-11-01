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
    return isPictureAt(m_doc->story(), from) ? from : -1;
}

void PageView::selectPicture(int position)
{
    if (!m_doc || m_hf.active || m_note.active || !isPictureAt(m_doc->story(), position)) return;
    m_cursor.setPosition(position);
    m_cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
    caretMoved();
}

bool PageView::inlinePictureRect(int position, const PictureInfo &info, QRectF *rect)
{
    int page = 0;
    QLineF line;
    if (!caretLine(position, &page, &line) || page < 0 || page >= m_rects.size()) return false;
    QTextCursor c(m_doc->story());
    c.setPosition(position + 1);
    const double descent = QFontMetricsF(c.charFormat().font()).descent();
    const double s = scale();
    const QRectF pr = m_rects[page].translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
    *rect = QRectF(pr.left() + line.x1() * s, pr.top() + (line.y2() - descent - info.height) * s, info.width * s, info.height * s);
    return true;
}

bool PageView::pictureRect(int position, QRectF *rect)
{
    if (!m_doc || m_hf.active || m_note.active) return false;
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
        if (p < 0 || !isPictureAt(m_doc->story(), p)) continue;
        const PictureInfo info = pictureAt(*m_doc, m_doc->story(), p);
        QRectF r;
        if (info.valid && !info.floating && inlinePictureRect(p, info, &r) && r.contains(pos)) return p;
    }
    return -1;
}

bool PageView::beginPictureDrag(QMouseEvent *e)
{
    if (!m_doc || m_hf.active || m_note.active || e->button() != Qt::LeftButton) return false;
    const QPoint pos = e->position().toPoint();
    int target = selectedPicture();
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
        if ((left || right) && (top || bottom)) {
            // A corner keeps the picture's proportions: the side that moved the farther, in proportion, sets the size.
            const double kx = (d.start.width() + (right ? delta.x() : -delta.x())) / d.start.width();
            const double ky = (d.start.height() + (bottom ? delta.y() : -delta.y())) / d.start.height();
            double k = std::abs(kx - 1) > std::abs(ky - 1) ? kx : ky;
            k = std::max(k, std::max(least / d.start.width(), least / d.start.height()));
            const QSizeF size(d.start.width() * k, d.start.height() * k);
            const QPointF fixed(left ? d.start.right() : d.start.left(), top ? d.start.bottom() : d.start.top());
            r = QRectF(QPointF(left ? fixed.x() - size.width() : fixed.x(), top ? fixed.y() - size.height() : fixed.y()), size);
        } else {
            if (left) r.setLeft(std::min(d.start.left() + delta.x(), d.start.right() - least));
            if (right) r.setRight(std::max(d.start.right() + delta.x(), d.start.left() + least));
            if (top) r.setTop(std::min(d.start.top() + delta.y(), d.start.bottom() - least));
            if (bottom) r.setBottom(std::max(d.start.bottom() + delta.y(), d.start.top() + least));
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
                const QRectF pr = m_rects[m_pictureDragPage].translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
                changed = setPicturePlace(*m_doc, d.position, (d.now.left() - pr.left()) / s, (d.now.top() - pr.top()) / s);
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
            const QRectF pr = m_rects[m_pictureDragPage].translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
            place = QPointF((d.now.left() - pr.left()) / s, (d.now.top() - pr.top()) / s);
        }
        changed = setPictureBox(*m_doc, d.position, d.now.width() / s, d.now.height() / s, place);
    }
    if (!changed && !m_doc->pictureNotice.isEmpty()) Q_EMIT notice(m_doc->pictureNotice);
    if (changed) {
        selectPicture(selectAt);
        Q_EMIT contentChanged();
    }
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
                  : result == PictureResult::TooLarge ? QCoreApplication::translate("jp::write::Pictures", "That picture is too large to put in a document.")
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
                  : result == PictureResult::TooLarge ? QCoreApplication::translate("jp::write::Pictures", "That picture is too large to put in a document.")
                                                      : m_doc->pictureNotice);
    return false;
}

bool PageView::changeSelectedPicture(const std::function<bool(WriteDoc &, int)> &change)
{
    const int position = selectedPicture();
    if (position < 0 || !m_doc) return false;
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
    if (m_doc && wantsDrop(e->mimeData())) e->acceptProposedAction();
    else e->ignore();
}

void PageView::dragMoveEvent(QDragMoveEvent *e)
{
    if (m_doc && wantsDrop(e->mimeData())) e->acceptProposedAction();
    else e->ignore();
}

void PageView::dropEvent(QDropEvent *e)
{
    if (!m_doc || !wantsDrop(e->mimeData())) {
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
        QFile f(u.toLocalFile());
        if (f.size() > kMostPictureBytes || !f.open(QIODevice::ReadOnly)) {
            Q_EMIT notice(QCoreApplication::translate("jp::write::Pictures", "That picture is too large to put in a document."));
            continue;
        }
        any = insertPictureBytes(f.readAll(), QFileInfo(f).completeBaseName()) || any;
        m_cursor.setPosition(m_cursor.selectionEnd());   // (the next one goes after this one)
        ++inserted;
    }
    if (any) e->acceptProposedAction();
    else e->ignore();
}

} // namespace jp::write
