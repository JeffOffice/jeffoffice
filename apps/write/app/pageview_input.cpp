// The keyboard, the mouse, the input method, and the clipboard in the page view.

#include "app/pageview.h"

#include "core/autoops.h"
#include "core/editops.h"
#include "core/linkops.h"
#include "app/settings.h"
#include "core/homeops.h"
#include <functional>
#include <QMenu>
#include <QToolButton>
#include "core/refops.h"
#include "core/tableops.h"
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
bool s_copiedWhole = true;   // the words last copied from here were whole words (smart paste adds spaces only for those)

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
    s_copiedWhole = selectionIsWholeWords(m_cursor);
    QApplication::clipboard()->setMimeData(mime);
}

// Smart cut and paste (Options > Advanced): on unless turned off.
bool PageView::smartCutPaste() { return Settings::get().value(QStringLiteral("edit/smartCutPaste"), true).toBool(); }

void PageView::cut()
{
    if (!m_doc || !m_cursor.hasSelection()) return;
    copy();
    if (smartCutPaste()) smartRemoveSelection(*m_doc, m_cursor);
    else removeSelection(*m_doc, m_cursor);
    caretMoved();
}

void PageView::paste()
{
    if (!m_doc) return;
    const int pasteFrom = m_cursor.selectionStart(), pasteSelectionEnd = m_cursor.selectionEnd();
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime) return;
    const bool ours = mime->hasFormat(QStringLiteral("application/x-jeffwrite-fragment")) && QString::fromUtf8(mime->data(QStringLiteral("application/x-jeffwrite-fragment"))) == s_copiedText && !s_copied.isEmpty();
    std::function<void()> put;
    if (ours) {
        put = [this] { pasteClip(*m_doc, m_cursor, s_copied); };
    } else if (mime->hasImage() && !mime->hasText() && !mime->hasHtml()) {
        // A picture on the clipboard (a screenshot, say) goes in as a picture.
        const QVariant data = mime->imageData();
        if (!data.canConvert<QImage>()) return;
        insertPictureImage(data.value<QImage>());
        return;
    } else if (mime->hasText()) {
        const QString text = mime->text();
        put = [this, text] { pasteText(*m_doc, m_cursor, text); };
    } else if (mime->hasHtml()) {
        const QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(mime->html());
        put = [this, fragment] { pasteFragment(*m_doc, m_cursor, fragment); };
    } else {
        return;
    }
    if (smartCutPaste() && (!ours || s_copiedWhole)) smartPaste(*m_doc, m_cursor, put);
    else put();
    notePaste(pasteFrom, pasteSelectionEnd);
    caretMoved();
}

bool PageView::canPaste(PasteKind kind) const
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!m_doc || !mime) return false;
    const bool ours = mime->hasFormat(QStringLiteral("application/x-jeffwrite-fragment")) && !s_copied.isEmpty();
    switch (kind) {
    case PasteKind::Keep: return ours || mime->hasText() || mime->hasHtml() || mime->hasImage();
    case PasteKind::Merge: return ours || mime->hasHtml();
    case PasteKind::TextOnly: return ours || mime->hasText() || mime->hasHtml();
    case PasteKind::Picture: return mime->hasImage();
    case PasteKind::Html: return mime->hasHtml();
    }
    return false;
}

void PageView::pasteAs(PasteKind kind)
{
    if (!m_doc || !canPaste(kind)) return;
    const int pasteFrom = m_cursor.selectionStart(), pasteSelectionEnd = m_cursor.selectionEnd();
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    const bool ours = mime->hasFormat(QStringLiteral("application/x-jeffwrite-fragment")) && QString::fromUtf8(mime->data(QStringLiteral("application/x-jeffwrite-fragment"))) == s_copiedText && !s_copied.isEmpty();
    auto put = [this, ours](const std::function<void()> &what) {
        if (smartCutPaste() && (!ours || s_copiedWhole)) smartPaste(*m_doc, m_cursor, what);
        else what();
    };
    switch (kind) {
    case PasteKind::Keep:
        paste();
        return;
    case PasteKind::Merge: {
        const QTextDocumentFragment fragment = ours ? s_copied.fragment : QTextDocumentFragment::fromHtml(mime->html());
        put([&] { pasteMerged(*m_doc, m_cursor, fragment); });
        break;
    }
    case PasteKind::TextOnly: {
        const QString text = ours ? plainTextOf(s_copied.fragment) : (mime->hasText() ? mime->text() : QTextDocumentFragment::fromHtml(mime->html()).toPlainText());
        put([&] { pasteText(*m_doc, m_cursor, text); });
        break;
    }
    case PasteKind::Picture: {
        const QVariant data = mime->imageData();
        if (!data.canConvert<QImage>()) return;
        insertPictureImage(data.value<QImage>());
        return;
    }
    case PasteKind::Html: {
        const QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(mime->html());
        put([&] { pasteFragment(*m_doc, m_cursor, fragment); });
        break;
    }
    }
    notePaste(pasteFrom, pasteSelectionEnd);
    caretMoved();
}

void PageView::notePaste(int from, int selectionEnd)
{
    m_lastPaste = LastPaste{from, selectionEnd, m_cursor.position(), m_doc ? m_doc->story()->revision() : -1, m_doc ? m_doc->story() : nullptr};
}

bool PageView::pasteOptionsAvailable() const
{
    return m_doc && m_lastPaste.story == m_doc->story() && m_cursor.document() == m_doc->story() && !m_cursor.hasSelection() && m_cursor.position() == m_lastPaste.to &&
           m_doc->story()->revision() == m_lastPaste.revision && m_lastPaste.to > m_lastPaste.from;
}

bool PageView::changeLastPaste(PasteKind kind)
{
    if (!pasteOptionsAvailable() || !canPaste(kind)) return false;
    const int from = m_lastPaste.from, selectionEnd = m_lastPaste.selectionEnd;
    undo();
    QTextCursor again(m_doc->story());
    again.setPosition(from);
    again.setPosition(std::max(from, selectionEnd), QTextCursor::KeepAnchor);
    m_cursor = again;
    pasteAs(kind);
    return true;
}

void PageView::updatePasteButton()
{
    if (!m_pasteButton) {
        m_pasteButton = new QToolButton(viewport());
        m_pasteButton->setObjectName(QStringLiteral("pasteOptionsButton"));
        m_pasteButton->setText(QCoreApplication::translate("jp::write::PageView", "Paste Options"));
        m_pasteButton->setAccessibleName(QCoreApplication::translate("jp::write::PageView", "Paste Options"));
        m_pasteButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        m_pasteButton->setPopupMode(QToolButton::InstantPopup);
        m_pasteButton->setFocusPolicy(Qt::NoFocus);
        auto *menu = new QMenu(m_pasteButton);
        struct Choice {
            const char *text;
            PasteKind kind;
        };
        for (const Choice &c : {Choice{QT_TRANSLATE_NOOP("jp::write::PageView", "Keep Source Formatting"), PasteKind::Keep}, Choice{QT_TRANSLATE_NOOP("jp::write::PageView", "Merge Formatting"), PasteKind::Merge},
                                Choice{QT_TRANSLATE_NOOP("jp::write::PageView", "Keep Text Only"), PasteKind::TextOnly}}) {
            const PasteKind kind = c.kind;
            QAction *a = menu->addAction(QCoreApplication::translate("jp::write::PageView", c.text));
            a->setObjectName(QStringLiteral("pasteOption%1").arg(int(kind)));
            connect(a, &QAction::triggered, this, [this, kind] {
                changeLastPaste(kind);
                setFocus(Qt::OtherFocusReason);
            });
        }
        m_pasteButton->setMenu(menu);
        m_pasteButton->hide();
        connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { updatePasteButton(); });
    }
    QRect r;
    if (!pasteOptionsAvailable() || !caretRect(&r)) {
        m_pasteButton->hide();
        return;
    }
    m_pasteButton->adjustSize();
    const QSize size = m_pasteButton->size();
    m_pasteButton->move(std::clamp(r.right() + 6, 0, std::max(0, viewport()->width() - size.width())), std::clamp(r.bottom() + 4, 0, std::max(0, viewport()->height() - size.height())));
    m_pasteButton->show();
    m_pasteButton->raise();
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
    if (m_cursor.hasSelection() && !m_typingReplaces) {
        m_cursor.setPosition(m_cursor.selectionEnd());
    } else if (m_overtype && !m_cursor.hasSelection()) {
        // Overtype: the letters typed take the place of those after the caret, as far as the paragraph goes.
        const QTextBlock b = m_cursor.block();
        const int room = b.position() + b.length() - 1 - m_cursor.position();
        const int n = std::min<int>(room, int(text.size()));
        if (n > 0) m_cursor.setPosition(m_cursor.position() + n, QTextCursor::KeepAnchor);
    }
    Settings &st = Settings::get();
    write::AutoOptions auto_;
    auto_.correct = st.value(QStringLiteral("proof/autocorrect"), true).toBool();
    auto_.quotes = st.value(QStringLiteral("proof/smartQuotes"), true).toBool();
    auto_.format = st.value(QStringLiteral("proof/autoformat"), true).toBool();
    auto_.links = st.value(QStringLiteral("proof/autolink"), true).toBool();
    if (m_overtype) auto_ = write::AutoOptions{false, false, false, false};
    write::typeText(*m_doc, m_cursor, text, auto_);
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
        else {
            write::AutoOptions auto_;
            auto_.format = Settings::get().value(QStringLiteral("proof/autoformat"), true).toBool();
            if (!write::enterAfterRule(*m_doc, m_cursor, auto_)) insertParagraph(*m_doc, m_cursor);
        }
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
    if (e->key() == Qt::Key_Escape && m_linkFrom >= 0) {
        m_linkFrom = -1;
        viewport()->unsetCursor();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && m_tableTool != TableTool::None) {
        // (the pen or the eraser is put down; a line begun is let go)
        m_toolDrag = ToolDrag();
        setTableTool(TableTool::None);
        viewport()->update();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && m_columnDrag.active) {
        // (a pull of a border or of the table's handle is let go: nothing is changed)
        m_columnDrag.active = false;
        viewport()->update();
        e->accept();
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
    if (e->key() == Qt::Key_Insert && m_insertKeyOvertype && !(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
        m_overtype = !m_overtype;
        Q_EMIT notice(m_overtype ? tr("Overtype: typing replaces the letters after the caret. Press Insert to go back.") : tr("Insert: typing pushes the letters after the caret along."));
        e->accept();
        return;
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
    if (m_linkFrom >= 0) {
        // Create Link is waiting for the box that follows: this click names it (or, on no box that can, leaves the link).
        const int from = m_linkFrom, to = pictureAtPoint(e->position().toPoint());
        m_linkFrom = -1;
        viewport()->unsetCursor();
        if (e->button() == Qt::LeftButton && to >= 0) {
            m_doc->pictureNotice.clear();
            if (linkTextBoxes(*m_doc, from, to) == PictureResult::Ok) {
                selectPicture(from);
                Q_EMIT contentChanged();
            } else {
                Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? tr("A text box can be followed only by a text box that has no words and is in no chain yet.") : m_doc->pictureNotice);
            }
            e->accept();
            viewport()->update();
            return;
        }
    }
    if (e->button() == Qt::LeftButton && e->type() != QEvent::MouseButtonDblClick && beginPictureDrag(e)) {
        e->accept();
        return;
    }
    m_columnDrag.pending = false;
    if (m_tableTool != TableTool::None && e->button() == Qt::LeftButton && !m_hf.active && !m_note.active && !m_readOnly) {
        const Hit hit = hitAt(e->position().toPoint());
        const int at = positionAt(hit);
        if (hit.page >= 0 && at >= 0) {
            m_toolDrag = {true, hit.page, at, hit.point, hit.point};
            viewport()->update();
            e->accept();
            return;
        }
    }
    if (e->button() == Qt::LeftButton && e->type() != QEvent::MouseButtonDblClick && !m_hf.active && !m_note.active) {
        // The little triangle by a heading collapses or opens it.
        if (const int heading = headingAt(e->position().toPoint()); heading >= 0 && collapseMark(heading).adjusted(-4, -4, 4, 4).contains(e->position().toPoint())) {
            toggleHeadingCollapsed(heading);
            e->accept();
            return;
        }
        if (tableHandleAt(e->position().toPoint(), &m_columnDrag)) {
            m_columnDrag.active = true;
            viewport()->update();
            e->accept();
            return;
        }
        // A press on a border is a click (the caret goes there) until the pointer has moved a few pixels; then it is the pull of the border.
        ColumnDrag border;
        if (columnBorderAt(e->position().toPoint(), &border)) {
            m_columnDrag = border;
            m_columnDrag.pending = true;
            m_columnDrag.startPx = e->position().toPoint();
        }
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
    if (((e->modifiers() & Qt::ControlModifier) || (!m_linkNeedsCtrl && !(e->modifiers() & Qt::ShiftModifier))) && followLinkAt(at)) return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool again = now - m_lastClickTime < QApplication::doubleClickInterval() && (e->position().toPoint() - m_lastPos).manhattanLength() < 6;
    // (A double click event is the second press of a pair, whatever came before it.)
    m_clicks = again ? (m_clicks % 3) + 1 : e->type() == QEvent::MouseButtonDblClick ? 2 : 1;
    m_lastClickTime = now;
    m_lastPos = e->position().toPoint();
    // A press inside the selection may begin a drag of its text: the selection stays until the pointer has moved (or the button is let go, which puts the caret there).
    if (m_clicks == 1 && m_textDragAllowed && m_dragTextOption && !m_readOnly && !(e->modifiers() & Qt::ShiftModifier) && m_cursor.hasSelection() && !m_hf.active && !m_note.active && m_cursor.document() == m_doc->story() &&
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
    if (m_toolDrag.active) {
        const Hit hit = hitAt(e->position().toPoint());
        if (hit.page == m_toolDrag.page) m_toolDrag.to = hit.point;
        viewport()->update();
        return;
    }
    if (m_columnDrag.pending) {
        if (!(e->buttons() & Qt::LeftButton)) m_columnDrag.pending = false;
        else if ((e->position().toPoint() - m_columnDrag.startPx).manhattanLength() >= QApplication::startDragDistance()) {
            m_columnDrag.pending = false;
            m_columnDrag.active = true;
            m_dragging = false;
            m_autoscroll.stop();
        }
    }
    if (m_columnDrag.active) {
        const Hit hit = hitAt(e->position().toPoint());
        m_columnDrag.nowX = hit.point.x();
        m_columnDrag.nowY = hit.point.y();
        viewport()->update();
        return;
    }
    if (m_pictureDrag.active) {
        updatePictureDrag(e->position().toPoint());
        return;
    }
    if (m_textDrag.pending) {
        if (!(e->buttons() & Qt::LeftButton)) m_textDrag.pending = false;
        else if ((e->position().toPoint() - m_textDrag.start).manhattanLength() >= QApplication::startDragDistance()) startTextDrag();
        return;
    }
    if (m_doc && !(e->buttons() & Qt::LeftButton)) {
        const int heading = headingAt(e->position().toPoint());
        if (heading != m_collapseHover) {
            m_collapseHover = heading;
            viewport()->update();
        }
    }
    if (m_doc && !m_dragging && m_tableTool == TableTool::None && !(e->buttons() & Qt::LeftButton)) {
        // The pointer says what a press on the selected picture would do, or that a press on a column's border would pull it.
        const PictureHandle h = pictureHandleAt(e->position().toPoint());
        ColumnDrag border;
        const int shape = h == PictureHandle::None && tableHandleAt(e->position().toPoint(), &border) ? int(Qt::SizeFDiagCursor)
                          : h == PictureHandle::None && columnBorderAt(e->position().toPoint(), &border) ? int(border.row ? Qt::SplitVCursor : Qt::SplitHCursor)
                          : h == PictureHandle::None ? int(Qt::IBeamCursor) : h == PictureHandle::Body ? int(Qt::SizeAllCursor)
                          : (h == PictureHandle::TopLeft || h == PictureHandle::BottomRight) ? int(Qt::SizeFDiagCursor)
                          : (h == PictureHandle::TopRight || h == PictureHandle::BottomLeft) ? int(Qt::SizeBDiagCursor)
                          : (h == PictureHandle::Top || h == PictureHandle::Bottom) ? int(Qt::SizeVerCursor) : int(Qt::SizeHorCursor);
        if (shape != m_hoverCursorShape) {
            m_hoverCursorShape = shape;
            viewport()->setCursor(Qt::CursorShape(shape));
        }
    }
    if (m_dragging && !(e->buttons() & Qt::LeftButton)) {   // (the release was lost)
        m_dragging = false;
        m_autoscroll.stop();
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

void PageView::setTableTool(TableTool tool)
{
    if (m_tableTool == tool) return;
    m_tableTool = tool;
    m_toolDrag = ToolDrag();
    m_hoverCursorShape = -1;
    viewport()->setCursor(tool == TableTool::None ? Qt::IBeamCursor : Qt::CrossCursor);
    viewport()->update();
    Q_EMIT tableToolChanged();
}

void PageView::finishTableTool(const QPoint &viewportPos)
{
    const ToolDrag d = m_toolDrag;
    m_toolDrag = ToolDrag();
    viewport()->update();
    if (!m_doc || d.position < 0 || d.position >= m_doc->story()->characterCount()) return;
    const Hit hit = hitAt(viewportPos);
    const QPointF delta = hit.page == d.page ? hit.point - d.from : QPointF();
    const int toPosition = hit.page == d.page ? positionAt(hit) : -1;
    QTextCursor c(m_doc->story());
    c.setPosition(d.position);
    m_doc->tableNotice.clear();
    bool changed = false;
    if (m_tableTool == TableTool::Draw) {
        if (!inTable(c.block())) {
            // A rectangle drawn in the text: a table of one cell that size (as wide as drawn, at least as tall).
            if (std::abs(delta.x()) < 12 || std::abs(delta.y()) < 8) return;
            write::NewTable table;
            table.rows = table.columns = 1;
            table.columnWidth = std::min(std::abs(delta.x()), write::textWidthAt(*m_doc, d.position));
            if (write::insertTable(*m_doc, c, table)) {
                write::setRowHeight(*m_doc, c, std::abs(delta.y()), false);
                changed = true;
            }
        } else if (std::abs(delta.x()) >= 8 || std::abs(delta.y()) >= 8) {
            // A line drawn across a cell divides it: a line that goes across makes two rows, one that goes down makes two columns.
            changed = write::splitCells(*m_doc, c, std::abs(delta.x()) >= std::abs(delta.y()) ? 1 : 2, std::abs(delta.x()) >= std::abs(delta.y()) ? 2 : 1);
        }
    } else if (toPosition >= 0 && toPosition != d.position) {
        // The eraser rubs out the line between the cell it began in and the one it ended in.
        c.setPosition(toPosition, QTextCursor::KeepAnchor);
        if (inTable(m_doc->story()->findBlock(d.position)) && inTable(m_doc->story()->findBlock(toPosition))) changed = write::mergeCells(*m_doc, c);
    }
    if (!changed && !m_doc->tableNotice.isEmpty()) Q_EMIT notice(m_doc->tableNotice);
    if (changed) {
        m_cursor.setPosition(std::min(c.position(), m_doc->story()->characterCount() - 1));
        refresh();
        caretMoved();
    }
}

void PageView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_toolDrag.active) {
        finishTableTool(e->position().toPoint());
        return;
    }
    if (m_columnDrag.active) {
        const Hit hit = hitAt(e->position().toPoint());
        const double width = std::max(6.0, m_columnDrag.width + (m_columnDrag.row ? hit.point.y() - m_columnDrag.startY : hit.point.x() - m_columnDrag.startX));
        const int at = m_columnDrag.position;
        const bool whole = m_columnDrag.whole;
        m_columnDrag.active = false;
        viewport()->update();
        if (std::abs(width - m_columnDrag.width) >= 0.5 && at >= 0 && at < m_doc->story()->characterCount()) {
            QTextCursor c(m_doc->story());
            c.setPosition(at);
            if (whole) write::setTableWidth(*m_doc, c, width);
            else if (m_columnDrag.row) write::setRowHeight(*m_doc, c, width, false);
            else write::setColumnWidth(*m_doc, c, width);
            m_cursor.setPosition(std::min(at, m_doc->story()->characterCount() - 1));
            refresh();
            caretMoved();
        }
        return;
    }
    m_columnDrag.pending = false;
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
    // A barcode picture: a double click selects it and asks for its data again.
    if (m_doc && !m_readOnly && e->button() == Qt::LeftButton && !m_hf.active && !m_note.active) {
        // A barcode field (shown as its bars): the same, on the field.
        if (const int field = barcodeFieldAtPoint(e->position().toPoint()); field >= 0) {
            QTextCursor c = m_cursor;
            c.setPosition(field);
            setCursor(c);
            Q_EMIT editBarcodeFieldWanted(field);
            e->accept();
            return;
        }
        if (const int picture = pictureAtPoint(e->position().toPoint()); picture >= 0 && !pictureAt(*m_doc, m_doc->story(), picture).barcode.isEmpty()) {
            selectPicture(picture);
            Q_EMIT editBarcodeWanted();
            e->accept();
            return;
        }
    }
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
        if (!m_hf.active && !m_note.active && area == Area::Body && !(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
            // A footnote's mark: the note is edited where it stands.
            const int at = positionAt(hit);
            if (at >= 0 && openNoteAt(at)) {
                e->accept();
                return;
            }
        }
        if (!m_hf.active && !m_draft && !m_web && area != Area::Body && (leaveNote(), enterHeaderFooter(hit.page, area == Area::Footer))) {
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

bool PageView::openNoteAt(int at)
{
    if (!m_doc || m_hf.active || m_cursor.document() != m_doc->story()) return false;
    const int id = footnoteReferenceAt(*m_doc, at);
    return id >= 0 && enterFootnote(id);
}

bool PageView::followLinkAt(int at)
{
    if (!m_doc || m_hf.active || m_note.active || m_cursor.document() != m_doc->story()) return false;
    const Hyperlink link = hyperlinkAt(*m_doc, at);
    if (link.valid() && link.anchor.isEmpty() && !link.address.isEmpty()) {
        if (mayFollow(link.address)) Q_EMIT linkFollowed(link.address, link.text);
        else Q_EMIT notice(tr("JeffWrite only follows links to web and email addresses."));
        return true;
    }
    const int target = linkTargetAt(*m_doc, at);
    if (target < 0) return false;
    QTextCursor c(m_doc->story());
    c.setPosition(target);
    showCursor(c);
    return true;
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
    if (m_sideToSide && e->angleDelta().x() == 0 && e->angleDelta().y() != 0) {
        // (Pages side by side: the wheel turns them past.)
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - e->angleDelta().y());
        if (!pagesComplete() && horizontalScrollBar()->value() >= horizontalScrollBar()->maximum() - 2) startPagination();
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
