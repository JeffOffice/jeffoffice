// The keyboard, the mouse, the input method, and the clipboard in the page view.

#include "app/pageview.h"

#include "core/editops.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <algorithm>

namespace jp::write {

namespace {

// The copy made in this program (from any of its documents): pasting it again keeps every format, and carries pictures and hyperlinks over.
Clip s_copied;
QString s_copiedText;

} // namespace

// ---- the editing commands

void PageView::undo()
{
    if (!m_doc) return;
    if (m_hf.active && m_doc->isPendingHeaderFooter(m_hf.story)) leaveHeaderFooter();   // (an empty one shown to be typed in has no step to take back)
    QTextCursor at;
    if (QTextDocument *story = m_doc->undo(&at)) followHistory(story, at);
}

void PageView::redo()
{
    if (!m_doc) return;
    if (m_hf.active && m_doc->isPendingHeaderFooter(m_hf.story)) leaveHeaderFooter();
    QTextCursor at;
    if (QTextDocument *story = m_doc->redo(&at)) followHistory(story, at);
}

void PageView::copy()
{
    if (!m_doc || !m_cursor.hasSelection()) return;
    const Clip clip = copySelection(*m_doc, m_cursor);
    const QTextDocumentFragment &f = clip.fragment;
    auto *mime = new QMimeData;
    // (A field is the text it shows on the page being edited, or the one the caret is on.)
    FieldContext shown;
    shown.doc = m_doc->flow().fields;
    {
        flow::Paginator &pg = m_doc->pager();
        const int page = std::clamp(currentPage(), 0, std::max(0, pg.pagesKnown() - 1));
        shown.pageNumber = pg.pagesKnown() > 0 ? pg.page(page).number : 1;
        shown.pageCount = std::max(1, pg.pageCount());
    }
    const QString text = plainTextOf(f, &shown);
    mime->setText(text);
    mime->setHtml(f.toHtml());
    mime->setData(QStringLiteral("application/x-jeffwrite-fragment"), text.toUtf8());
    s_copied = clip;
    s_copiedText = text;
    QApplication::clipboard()->setMimeData(mime);
}

void PageView::cut()
{
    if (!m_doc || !m_cursor.hasSelection()) return;
    copy();
    removeSelection(*m_doc, m_cursor);
    caretMoved();
}

void PageView::paste()
{
    if (!m_doc) return;
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime) return;
    if (mime->hasFormat(QStringLiteral("application/x-jeffwrite-fragment")) && QString::fromUtf8(mime->data(QStringLiteral("application/x-jeffwrite-fragment"))) == s_copiedText && !s_copied.isEmpty())
        pasteClip(*m_doc, m_cursor, s_copied);
    else if (mime->hasText())
        pasteText(*m_doc, m_cursor, mime->text());
    else if (mime->hasHtml())
        pasteFragment(*m_doc, m_cursor, QTextDocumentFragment::fromHtml(mime->html()));
    else
        return;
    caretMoved();
}

void PageView::selectAll()
{
    if (!m_doc) return;
    m_cursor.select(QTextCursor::Document);
    caretMoved();
}

void PageView::insertText(const QString &text)
{
    if (!m_doc) return;
    write::insertText(*m_doc, m_cursor, text);
    caretMoved();
}

// ---- the keyboard

void PageView::typeKey(int key, Qt::KeyboardModifiers m)
{
    if (!m_doc) return;
    m_movedFrom = m_cursor.position();
    const bool shift = m & Qt::ShiftModifier, ctrl = m & Qt::ControlModifier;
    const QTextCursor::MoveMode mode = shift ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
    switch (key) {
    case Qt::Key_Left:
        if (ctrl) m_cursor.movePosition(QTextCursor::PreviousWord, mode);
        else moveCursor(m_cursor, QTextCursor::PreviousCharacter, shift);
        caretMoved();
        break;
    case Qt::Key_Right:
        if (ctrl) m_cursor.movePosition(QTextCursor::NextWord, mode);
        else moveCursor(m_cursor, QTextCursor::NextCharacter, shift);
        caretMoved();
        break;
    case Qt::Key_Up:
    case Qt::Key_Down: {
        const int dir = key == Qt::Key_Up ? -1 : 1;
        if (ctrl) {
            m_cursor.movePosition(dir < 0 ? QTextCursor::PreviousBlock : QTextCursor::NextBlock, mode);
            caretMoved();
        } else {
            const int to = moveVertical(m_cursor.position(), dir);
            m_cursor.setPosition(to, mode);
            caretMoved(true);
        }
        break;
    }
    case Qt::Key_Home:
        if (ctrl) m_cursor.movePosition(QTextCursor::Start, mode);
        else m_cursor.setPosition(lineEdge(m_cursor.position(), false), mode);
        caretMoved();
        break;
    case Qt::Key_End:
        if (ctrl) m_cursor.movePosition(QTextCursor::End, mode);
        else m_cursor.setPosition(lineEdge(m_cursor.position(), true), mode);
        caretMoved();
        break;
    case Qt::Key_PageUp:
    case Qt::Key_PageDown: {
        const int dir = key == Qt::Key_PageUp ? -1 : 1;
        QRect r;
        if (!caretRect(&r)) break;
        const int step = std::max(40, viewport()->height() - 60);
        const int before = verticalScrollBar()->value();
        verticalScrollBar()->setValue(before + dir * step);
        const int moved = verticalScrollBar()->value() - before;
        // (The caret has moved up the window by what the window scrolled; it goes on by the rest of the step.)
        const Hit hit = hitAt(QPoint(r.x(), std::clamp(r.center().y() + dir * step - moved, 0, viewport()->height() - 1)));
        const int at = positionAt(hit);
        if (at >= 0) m_cursor.setPosition(at, mode);
        if (dir > 0 && moved == 0 && !pagesComplete()) paginateAll();
        caretMoved(true);
        break;
    }
    case Qt::Key_Backspace:
        deleteBack(*m_doc, m_cursor, ctrl);
        caretMoved();
        break;
    case Qt::Key_Delete:
        deleteForward(*m_doc, m_cursor, ctrl);
        caretMoved();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (ctrl) insertPageBreak(*m_doc, m_cursor);
        else if (shift) write::insertText(*m_doc, m_cursor, QString(QChar(QChar::LineSeparator)));
        else insertParagraph(*m_doc, m_cursor);
        caretMoved();
        break;
    case Qt::Key_Tab:
        insertTab(*m_doc, m_cursor, false);
        caretMoved();
        break;
    case Qt::Key_Backtab:
        insertTab(*m_doc, m_cursor, true);
        caretMoved();
        break;
    case Qt::Key_Escape:
        if (m_hf.active) {
            leaveHeaderFooter();
            break;
        }
        if (m_cursor.hasSelection()) {
            m_cursor.clearSelection();
            caretMoved();
        }
        break;
    default:
        break;
    }
}

void PageView::keyPressEvent(QKeyEvent *e)
{
    if (!m_doc) {
        e->ignore();
        return;
    }
    switch (e->key()) {
    case Qt::Key_Left: case Qt::Key_Right: case Qt::Key_Up: case Qt::Key_Down: case Qt::Key_Home: case Qt::Key_End: case Qt::Key_PageUp: case Qt::Key_PageDown:
    case Qt::Key_Backspace: case Qt::Key_Delete: case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_Tab: case Qt::Key_Backtab: case Qt::Key_Escape:
        typeKey(e->key(), e->modifiers());
        e->accept();
        return;
    default:
        break;
    }
    // Text: not a shortcut (Ctrl alone), and not a control character.
    const QString text = e->text();
    const bool shortcut = (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) && !(e->modifiers() & Qt::AltModifier);
    if (!text.isEmpty() && !shortcut && text[0].isPrint()) {
        insertText(text);
        e->accept();
        return;
    }
    e->ignore();
}

void PageView::inputMethodEvent(QInputMethodEvent *e)
{
    if (!m_doc) return;
    if (!e->commitString().isEmpty()) insertText(e->commitString());
    e->accept();
}

QVariant PageView::inputMethodQuery(Qt::InputMethodQuery q) const
{
    switch (q) {
    case Qt::ImEnabled: return m_doc != nullptr;
    case Qt::ImCursorRectangle: {
        QRect r;
        if (const_cast<PageView *>(this)->caretRect(&r)) return r;
        return QRect();
    }
    case Qt::ImCursorPosition: return m_cursor.positionInBlock();
    case Qt::ImSurroundingText: return m_cursor.block().text();
    case Qt::ImCurrentSelection: return m_cursor.selectedText();
    default: return QVariant();
    }
}

// ---- the mouse

void PageView::mousePressEvent(QMouseEvent *e)
{
    if (!m_doc) return;
    setFocus(Qt::MouseFocusReason);
    m_movedFrom = -1;   // (a click has no way it went)
    const Hit hit = hitAt(e->position().toPoint());
    if (m_hf.active) {
        // A click in the text does nothing while a header or footer is edited (a double click leaves); one in a header's or a footer's area goes to it.
        const Area area = areaOf(hit);
        if (area == Area::Body || !takeFurnitureAt(hit, area == Area::Footer)) return;
    }
    const int at = positionAt(hit);
    if (at < 0) return;
    if (e->button() == Qt::RightButton) {
        if (!m_cursor.hasSelection() || at < m_cursor.selectionStart() || at > m_cursor.selectionEnd()) {
            m_cursor.setPosition(at);
            caretMoved();
        }
        m_contextAt = at;   // (what the menu is for, whether or not the selection stayed)
        return;
    }
    if (e->button() != Qt::LeftButton) return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool again = now - m_lastClickTime < QApplication::doubleClickInterval() && (e->position().toPoint() - m_lastPos).manhattanLength() < 6;
    // (A double click event is the second press of a pair, whatever came before it.)
    m_clicks = again ? (m_clicks % 3) + 1 : e->type() == QEvent::MouseButtonDblClick ? 2 : 1;
    m_lastClickTime = now;
    m_lastPos = e->position().toPoint();
    if (m_clicks == 3) {
        m_cursor.setPosition(at);
        m_cursor.movePosition(QTextCursor::StartOfBlock);
        m_cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    } else if (m_clicks == 2) {
        m_cursor.setPosition(at);
        m_cursor.select(QTextCursor::WordUnderCursor);
    } else if (e->modifiers() & Qt::ShiftModifier) {
        m_cursor.setPosition(at, QTextCursor::KeepAnchor);
    } else {
        m_cursor.setPosition(at);
    }
    m_dragging = true;
    m_autoscroll.start();
    caretMoved();
}

void PageView::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_doc || !m_dragging || !(e->buttons() & Qt::LeftButton)) return;
    const int at = positionAt(hitAt(e->position().toPoint()));
    if (at < 0) return;
    if (m_clicks == 2) {
        // By words.
        QTextCursor w(m_cursor.document());
        w.setPosition(at);
        w.select(QTextCursor::WordUnderCursor);
        const int anchor = m_cursor.anchor() < m_cursor.position() ? m_cursor.selectionStart() : m_cursor.selectionEnd();
        Q_UNUSED(anchor);
        m_cursor.setPosition(at > m_cursor.anchor() ? w.selectionEnd() : w.selectionStart(), QTextCursor::KeepAnchor);
    } else {
        m_cursor.setPosition(at, QTextCursor::KeepAnchor);
    }
    m_movedFrom = m_cursor.anchor();   // (a drag goes on or back from where it began)
    caretMoved();
}

void PageView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    m_dragging = false;
    m_autoscroll.stop();
}

void PageView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (m_doc && e->button() == Qt::LeftButton) {
        const Hit hit = hitAt(e->position().toPoint());
        const Area area = areaOf(hit);
        if (m_hf.active && area == Area::Body) {
            // In the text of a page: the header or footer is left, and the caret goes where the click was.
            leaveHeaderFooter();
            const int at = positionAt(hit);
            if (at >= 0) {
                m_cursor.setPosition(at);
                caretMoved();
            }
            e->accept();
            return;
        }
        if (!m_hf.active && area != Area::Body && enterHeaderFooter(hit.page, area == Area::Footer)) {
            const int at = positionAt(hit);
            if (at >= 0) {
                m_cursor.setPosition(at);
                caretMoved();
            }
            e->accept();
            return;
        }
    }
    // (A double click is a second press: the press handler counts it.)
    mousePressEvent(e);
}

void PageView::contextMenuEvent(QContextMenuEvent *e)
{
    if (!m_doc) return;
    if (e->reason() == QContextMenuEvent::Mouse) {
        e->accept();
        Q_EMIT contextMenuWanted(e->globalPos());
        m_contextAt = -1;
        return;
    }
    m_contextAt = -1;   // (from the keyboard it is the caret's)
    QRect r;
    caretRect(&r);
    e->accept();
    Q_EMIT contextMenuWanted(viewport()->mapToGlobal(r.bottomLeft()));
}

void PageView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        const double steps = e->angleDelta().y() / 120.0;
        if (steps != 0) setZoom(std::round(m_zoom * 10 + steps) / 10);
        e->accept();
        return;
    }
    QAbstractScrollArea::wheelEvent(e);
    if (!pagesComplete() && verticalScrollBar()->value() >= verticalScrollBar()->maximum() - 2) startPagination();
}

void PageView::startAutoscroll() { m_autoscroll.start(); }

} // namespace jp::write
