// The keyboard, the mouse, the input method, and the clipboard in the page view.

#include "app/pageview.h"

#include "core/editops.h"
#include "core/refops.h"
#include "core/writeprops.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDateTime>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QPointer>
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
    m_pictureDrag = PictureDrag();   // (a drag of a picture is over: what it would have done is not done)
    viewport()->update();
    if (m_hf.active && m_doc->isPendingHeaderFooter(m_hf.story)) leaveHeaderFooter();   // (an empty one shown to be typed in has no step to take back)
    QTextCursor at;
    const int picture = selectedPicture();
    if (QTextDocument *story = m_doc->undo(&at)) {
        followHistory(story, at);
        // (A picture that was selected stays selected through a step that changed it.)
        if (picture >= 0 && selectedPicture() < 0 && m_cursor.position() >= picture && m_cursor.position() <= picture + 1) selectPicture(picture);
    }
}

void PageView::redo()
{
    if (!m_doc) return;
    m_pictureDrag = PictureDrag();   // (a drag of a picture is over: what it would have done is not done)
    viewport()->update();
    if (m_hf.active && m_doc->isPendingHeaderFooter(m_hf.story)) leaveHeaderFooter();
    QTextCursor at;
    const int picture = selectedPicture();
    if (QTextDocument *story = m_doc->redo(&at)) {
        followHistory(story, at);
        // (A picture that was selected stays selected through a step that changed it.)
        if (picture >= 0 && selectedPicture() < 0 && m_cursor.position() >= picture && m_cursor.position() <= picture + 1) selectPicture(picture);
    }
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
    else if (mime->hasImage() && !mime->hasText() && !mime->hasHtml()) {
        // A picture on the clipboard (a screenshot, say) goes in as a picture.
        const QVariant data = mime->imageData();
        if (!data.canConvert<QImage>()) return;
        insertPictureImage(data.value<QImage>());
        return;
    } else if (mime->hasText())
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
            // Up from a note's first line and Down from its last leave it, for the text, just after its reference.
            if (m_note.active && !shift && !m_cursor.hasSelection() && noteLineAtEdge(dir)) {
                leaveNote(true);
                break;
            }
            const int to = moveVertical(m_cursor.position(), dir);
            // Up from the first line of an endnote and Down from its last leave it, for the text just after its reference.
            if (!m_note.active && !m_hf.active && !shift && !m_cursor.hasSelection()) {
                const EndnoteSpan span = endnoteSpanAt(m_doc->story(), m_cursor.position());
                int id = 0;
                if (span.valid && span.editable && (to < span.textFrom || to > span.textTo || to == m_cursor.position()) && m_doc->endnoteIdOf(m_doc->story()->findBlock(m_cursor.position()), &id)) {
                    leaveEndnote(id);
                    break;
                }
            }
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
        if (m_note.active) {
            leaveNote(true);
            break;
        }
        if (int id = 0; !m_hf.active && !m_cursor.hasSelection() && inEndnoteWords(m_cursor) && m_doc->endnoteIdOf(m_doc->story()->findBlock(m_cursor.position()), &id)) {
            leaveEndnote(id);
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
    if (m_readOnly) {
        // (only the keys that move the caret)
        switch (e->key()) {
        case Qt::Key_Left: case Qt::Key_Right: case Qt::Key_Up: case Qt::Key_Down: case Qt::Key_Home: case Qt::Key_End: case Qt::Key_PageUp: case Qt::Key_PageDown:
            typeKey(e->key(), e->modifiers());
            e->accept();
            return;
        default:
            e->ignore();
            return;
        }
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
    if (!m_doc || m_readOnly) return;
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
    if (e->button() == Qt::LeftButton && e->type() != QEvent::MouseButtonDblClick && beginPictureDrag(e)) {
        e->accept();
        return;
    }
    const Hit hit = hitAt(e->position().toPoint());
    if (m_hf.active) {
        // A click in the text does nothing while a header or footer is edited (a double click leaves); one in a header's or a footer's area goes to it.
        const Area area = areaOf(hit);
        if (area == Area::Body || !takeFurnitureAt(hit, area == Area::Footer)) return;
    } else {
        // A click in a footnote's text puts the caret there, in the note; a click anywhere else is in the text, and leaves the note it was in.
        QTextDocument *noteStory = nullptr;
        int noteId = 0, noteNumber = 0;
        if (noteAt(hit, &noteStory, &noteId, &noteNumber)) {
            if (!enterNote(noteStory, noteId, hit.page, noteNumber)) return;
        } else if (m_note.active) {
            leaveNote();
        }
    }
    int at = positionAt(hit);
    if (at < 0) return;
    // A click in the second or a later copy of an endnote (which the pages show, and no more) is a click at the same place of its first copy, the note's own.
    if (!m_note.active && !m_hf.active && m_cursor.document() == m_doc->story()) at = placeInFirstCopy(at);
    if (e->button() == Qt::RightButton) {
        // A right click on a picture selects it, and the menu is for it.
        if (const int picture = pictureAtPoint(e->position().toPoint()); picture >= 0 && picture != selectedPicture()) {
            selectPicture(picture);
            m_contextAt = picture;
            return;
        }
        if (!m_cursor.hasSelection() || at < m_cursor.selectionStart() || at > m_cursor.selectionEnd()) {
            m_cursor.setPosition(at);
            caretMoved();
        }
        m_contextAt = at;   // (what the menu is for, whether or not the selection stayed)
        return;
    }
    if (e->button() != Qt::LeftButton) return;
    // Ctrl+click on a link inside the text (a reference or a table of contents line with its link, or a link to a bookmark) goes to what it names.
    if ((e->modifiers() & Qt::ControlModifier) && !m_hf.active && !m_note.active && m_cursor.document() == m_doc->story()) {
        const int target = linkTargetAt(*m_doc, at);
        if (target >= 0) {
            QTextCursor c(m_doc->story());
            c.setPosition(target);
            showCursor(c);
            return;
        }
    }
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool again = now - m_lastClickTime < QApplication::doubleClickInterval() && (e->position().toPoint() - m_lastPos).manhattanLength() < 6;
    // (A double click event is the second press of a pair, whatever came before it.)
    m_clicks = again ? (m_clicks % 3) + 1 : e->type() == QEvent::MouseButtonDblClick ? 2 : 1;
    m_lastClickTime = now;
    m_lastPos = e->position().toPoint();
    // A press inside the selection may begin a drag of its text: the selection stays until the pointer has moved (or the button is let go, which puts the caret there).
    if (m_clicks == 1 && m_textDragAllowed && !m_readOnly && !(e->modifiers() & Qt::ShiftModifier) && m_cursor.hasSelection() && !m_hf.active && !m_note.active && m_cursor.document() == m_doc->story() &&
        at >= m_cursor.selectionStart() && at < m_cursor.selectionEnd() && !isLocked(m_cursor.block())) {
        m_textDrag.pending = true;
        m_textDrag.start = e->position().toPoint();
        return;
    }
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
    if (m_pictureDrag.active) {
        updatePictureDrag(e->position().toPoint());
        return;
    }
    if (m_textDrag.pending) {
        if (!(e->buttons() & Qt::LeftButton)) m_textDrag.pending = false;
        else if ((e->position().toPoint() - m_textDrag.start).manhattanLength() >= QApplication::startDragDistance()) startTextDrag();
        return;
    }
    if (m_doc && !m_dragging && !(e->buttons() & Qt::LeftButton)) {
        // The pointer says what a press on the selected picture would do.
        const PictureHandle h = pictureHandleAt(e->position().toPoint());
        const int shape = h == PictureHandle::None ? int(Qt::IBeamCursor) : h == PictureHandle::Body ? int(Qt::SizeAllCursor)
                          : (h == PictureHandle::TopLeft || h == PictureHandle::BottomRight) ? int(Qt::SizeFDiagCursor)
                          : (h == PictureHandle::TopRight || h == PictureHandle::BottomLeft) ? int(Qt::SizeBDiagCursor)
                          : (h == PictureHandle::Top || h == PictureHandle::Bottom) ? int(Qt::SizeVerCursor) : int(Qt::SizeHorCursor);
        if (shape != m_hoverCursorShape) {
            m_hoverCursorShape = shape;
            viewport()->setCursor(Qt::CursorShape(shape));
        }
    }
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
    if (m_pictureDrag.active) {
        endPictureDrag(e->position().toPoint());
        return;
    }
    if (m_textDrag.pending) {
        // A click inside the selection, without a pull, puts the caret there.
        m_textDrag.pending = false;
        const int at = positionAt(hitAt(e->position().toPoint()));
        if (at >= 0) {
            m_cursor.setPosition(at);
            caretMoved();
        }
        if (e->button() == Qt::LeftButton) Q_EMIT selectionMade();
        return;
    }
    m_dragging = false;
    m_autoscroll.stop();
    if (e->button() == Qt::LeftButton) Q_EMIT selectionMade();
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
        if (!m_hf.active && area != Area::Body && (leaveNote(), enterHeaderFooter(hit.page, area == Area::Footer))) {
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

// ---- dragging text

namespace {
constexpr char kTextDragMime[] = "application/x-jeffwrite-text";
}

QMimeData *PageView::prepareTextDrag()
{
    if (!m_doc || !m_cursor.hasSelection() || m_hf.active || m_note.active || m_cursor.document() != m_doc->story()) return nullptr;
    m_textDrag.selection = m_cursor;
    m_textDrag.active = true;
    m_textDrag.done = false;
    auto *mime = new QMimeData;
    mime->setText(plainTextOf(selectionFragment(m_cursor)));
    mime->setData(QString::fromLatin1(kTextDragMime), QByteArray::number(qulonglong(m_doc->id())));
    return mime;
}

void PageView::startTextDrag()
{
    QPointer<PageView> self(this);
    m_textDrag.pending = false;
    std::unique_ptr<QMimeData> mime(prepareTextDrag());
    if (!mime) return;
    m_dragging = false;
    m_autoscroll.stop();
    auto *drag = new QDrag(viewport());
    drag->setMimeData(mime.release());
    const Qt::DropAction result = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
    if (!self) return;
    // A target outside this view took the text as a move: it leaves here.
    if (result == Qt::MoveAction && !m_textDrag.done && m_doc && m_textDrag.selection.hasSelection() && m_textDrag.selection.document() == m_doc->story()) {
        QTextCursor cut = m_textDrag.selection;
        removeSelection(*m_doc, cut);
        m_cursor = cut;
        caretMoved();
    }
    m_textDrag = TextDrag();
}

bool PageView::wantsTextDrop(const QMimeData *mime) const
{
    return m_doc && !m_readOnly && !m_hf.active && !m_note.active && mime && (mime->hasFormat(QString::fromLatin1(kTextDragMime)) || (mime->hasText() && !mime->hasUrls()));
}

bool PageView::dropText(QDropEvent *e)
{
    const QMimeData *mime = e->mimeData();
    if (!wantsTextDrop(mime)) return false;
    const int at = positionAt(hitAt(e->position().toPoint()));
    if (at < 0) {
        e->ignore();
        return true;
    }
    if (mime->hasFormat(QString::fromLatin1(kTextDragMime)) && m_textDrag.active && m_textDrag.selection.hasSelection()) {
        Qt::KeyboardModifiers copyKeys = Qt::ControlModifier;
#ifdef Q_OS_MACOS
        copyKeys |= Qt::AltModifier;
#endif
        const bool copy = (e->modifiers() & copyKeys) || e->possibleActions() == Qt::CopyAction;
        QTextCursor moved = m_textDrag.selection;
        if (moveSelectedText(*m_doc, moved, at, copy)) {
            m_cursor = moved;
            caretMoved();
            m_textDrag.done = true;
            e->setDropAction(copy ? Qt::CopyAction : Qt::MoveAction);
            e->accept();
        } else {
            e->ignore();
        }
        return true;
    }
    QTextCursor c = m_cursor;
    c.setPosition(at);
    pasteText(*m_doc, c, mime->text());
    m_cursor = c;
    caretMoved();
    e->acceptProposedAction();
    return true;
}

} // namespace jp::write
