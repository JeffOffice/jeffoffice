#include "app/storyedit.h"

#include "app/editor.h"
#include "app/listmarkers.h"
#include "app/minitoolbar.h"
#include "app/spelling.h"
#include "text/textprops.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QFocusEvent>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QPointer>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <cmath>
#include <algorithm>
#include <array>

namespace jp {

StoryEdit::StoryEdit(Editor *ed, QWidget *parent) : QTextEdit(parent), m_ed(ed)
{
    m_none = new QTextDocument(this);
    setDocument(m_none);
    setAcceptRichText(true);
    setAcceptDrops(false);
    setUndoRedoEnabled(false);   // (the presentation's one undo history keeps the steps)
    setTabChangesFocus(false);
    connect(this, &QTextEdit::cursorPositionChanged, this, &StoryEdit::followView);
    connect(this, &QTextEdit::selectionChanged, this, &StoryEdit::followView);
    connect(this, &QTextEdit::textChanged, this, &StoryEdit::markWords);
    connect(ed, &Editor::textCursorChanged, this, &StoryEdit::followEditor);
    connect(ed, &Editor::viewChanged, this, &StoryEdit::markWords);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &StoryEdit::markWords);
    m_markers = new ListMarkers(this, [this](int left, int right) { setViewportMargins(left, 0, right, 0); }, [this]() -> const ColorScheme * { return m_ed->doc() ? &m_ed->doc()->colors : nullptr; });
}

StoryEdit::~StoryEdit()
{
    // (the story, and the document the editor still has, are let go before this view's own document goes)
    if (m_story) {
        setDocument(m_none);
        if (m_story->doc) {
            m_story->doc->setDocumentLayout(nullptr);
            m_story->doc->setPageSize(m_page);
            m_story->doc->setDefaultTextOption(m_option);
            m_story->doc->setDefaultFont(m_font);
            m_story->doc->setIndentWidth(m_indent);
        }
    }
}

void StoryEdit::setStory(const QString &storyId)
{
    m_id = storyId;
    std::shared_ptr<Story> found;
    // Only a view that is shown has the story's document: the document has one layout and one width, and a hidden view that kept it would give its own to the one shown.
    if (Document *d = m_ed->doc(); d && !storyId.isEmpty() && !m_yield && isVisibleTo(window())) found = d->stories.value(storyId);
    if (found.get() == m_story.get()) return;
    bind(found);
}

void StoryEdit::bind(const std::shared_ptr<Story> &story)
{
    const QTextCursor was = textCursor();
    const bool kept = m_story && story && story->id == m_story->id;   // (the same words in a document that replaced the old: an undo)
    const int anchor = was.anchor(), pos = was.position();
    std::shared_ptr<Story> old = m_story;
    const QSizeF oldPage = m_page;
    const QTextOption oldOption = m_option;
    const QFont oldFont = m_font;
    const qreal oldIndent = m_indent;
    m_syncing = true;
    m_story = story;
    QTextDocument *d = story ? story->doc.get() : m_none;
    if (story) {
        m_page = d->pageSize();
        m_option = d->defaultTextOption();
        m_font = d->defaultFont();
        m_indent = d->indentWidth();
    }
    setDocument(d);
    if (story) {
        d->setDefaultFont(m_font);   // (the view's own font is never the story's)
        applyScale();
    }
    d->setUndoRedoEnabled(false);
    if (old && old->doc) {
        // (the old document gets back what it had: no layout, and its own sizes)
        old->doc->setDocumentLayout(nullptr);
        old->doc->setPageSize(oldPage);
        old->doc->setDefaultTextOption(oldOption);
        old->doc->setDefaultFont(oldFont);
        old->doc->setIndentWidth(oldIndent);
    }
    QTextCursor c(d);
    const int last = std::max(0, d->characterCount() - 1);
    if (kept) {
        c.setPosition(std::clamp(anchor, 0, last));
        c.setPosition(std::clamp(pos, 0, last), QTextCursor::KeepAnchor);
    } else {
        c.movePosition(QTextCursor::Start);   // (a story shown for the first time, or after another one: its words from the top)
    }
    setTextCursor(c);
    if (!kept) {
        m_took = false;   // (the person typed in the view that showed the words before, not in this one)
        verticalScrollBar()->setValue(0);
        horizontalScrollBar()->setValue(0);
    }
    m_syncing = false;
    markWords();
    m_markers->refresh();
    if (isTarget() && isTyper()) followEditor();
    Q_EMIT storyShown();
}

void StoryEdit::setDisplayScale(double scale)
{
    scale = std::clamp(scale, 0.05, 16.0);
    if (qFuzzyCompare(scale, m_scale)) return;
    m_scale = scale;
    applyScale();
    m_markers->setScale(m_scale);
    markWords();
}

// The layout sizes the words (points) by the resolution of its paint device: a device of the scaled resolution shows them larger or smaller, with nothing in the story changed.
// What is measured in pixels (the indent of a list) is scaled with them.
void StoryEdit::applyScale()
{
    if (!m_story || !m_story->doc) return;
    QTextDocument *d = m_story->doc.get();
    QAbstractTextDocumentLayout *lay = d->documentLayout();
    if (!lay) return;
    if (qFuzzyCompare(m_scale, 1.0)) {
        lay->setPaintDevice(viewport());
        d->setIndentWidth(m_indent);
    } else {
        const int dpi = std::max(1, int(std::lround(viewport()->logicalDpiY() * m_scale)));
        m_device = QImage(1, 1, QImage::Format_Mono);
        m_device.setDotsPerMeterX(int(std::lround(dpi / 0.0254)));
        m_device.setDotsPerMeterY(int(std::lround(dpi / 0.0254)));
        lay->setPaintDevice(&m_device);
        d->setIndentWidth(m_indent);   // (the layout scales an indent by the device's resolution itself: scaled here too, a list would be indented twice as much)
    }
    d->markContentsDirty(0, d->characterCount());
    m_markers->setScale(m_scale);
}

void StoryEdit::setGhost(bool ghost)
{
    if (m_ghost == ghost) return;
    m_ghost = ghost;
    m_markers->setShown(!ghost);
    viewport()->update();
}

void StoryEdit::paintEvent(QPaintEvent *e)
{
    if (m_ghost) return;
    QTextEdit::paintEvent(e);
}

void StoryEdit::wheelEvent(QWheelEvent *e)
{
    if (m_ghost) {
        e->ignore();
        return;
    }
    QTextEdit::wheelEvent(e);
}

void StoryEdit::setYielding(bool yield)
{
    if (m_yield == yield) return;
    m_yield = yield;
    if (yield) bind(nullptr);
    else setStory(m_id);
}

// The view the person is typing in: it has the keyboard, or took the Editor's text edit and has kept it. The other views of the same words (which show them in turn, or at once) do not
// follow the caret: a note opens at its top in each, wherever the caret is.
bool StoryEdit::isTyper() const { return hasFocus() || m_took; }

bool StoryEdit::isTarget() const
{
    return m_story && m_ed->editingStoryOnly() && m_ed->textTarget().storyId == m_id && m_ed->cursor().document() == m_story->doc.get();
}

bool StoryEdit::takeTarget()
{
    if (isReadOnly() || m_ed->readOnly() || m_ed->closed()) return false;
    if (!m_story) {
        const QString made = makeStory();
        if (made.isEmpty()) return false;
        setStory(made);
        if (!m_story) return false;
    }
    if (isTarget()) {
        m_took = true;
        return true;
    }
    const QTextCursor c = textCursor();
    m_syncing = true;
    m_ed->beginStoryEdit(m_id, c.position());
    m_syncing = false;
    if (!isTarget()) return false;
    m_ed->setCursor(c);   // (with its selection)
    m_took = true;
    return true;
}

void StoryEdit::flushTyping() { m_ed->flushTyping(); }
bool StoryEdit::hasTyping() const { return isTarget() && m_ed->typing(); }

void StoryEdit::followView()
{
    if (m_syncing || !isTarget()) return;
    m_syncing = true;
    m_ed->setCursor(textCursor());
    m_syncing = false;
    markWords();   // (the word with the caret is the one being typed: its mark waits)
}

void StoryEdit::followEditor()
{
    if (m_syncing || !isTarget() || !isTyper()) return;
    const QTextCursor &c = m_ed->cursor();
    const QTextCursor mine = textCursor();
    if (c.position() != mine.position() || c.anchor() != mine.anchor()) {
        m_syncing = true;
        setTextCursor(c);
        m_syncing = false;
    }
    ensureCursorVisible();
    markWords();
}

void StoryEdit::setExtraMarks(const QVector<Mark> &marks)
{
    if (marks == m_marks) return;
    m_marks = marks;
    ++m_markVersion;
    markWords();
}

QVector<StoryEdit::Mark> StoryEdit::spellingMarks(int from, int to) const
{
    QVector<Mark> out;
    if (!m_story) return out;
    const bool typing = isTarget() && !m_ed->cursor().hasSelection();
    const int at = typing ? m_ed->cursor().position() : -1;
    const QVector<QPair<int, int>> all = m_ed->misspelledIn(m_id);   // (in order)
    auto it = std::lower_bound(all.cbegin(), all.cend(), from, [](const QPair<int, int> &m, int f) { return m.second < f; });
    for (; it != all.cend() && it->first <= to; ++it) {
        if (at >= it->first && at <= it->second) continue;   // (the word still being typed is not flagged)
        out.push_back(Mark{it->first, it->second, QColor(220, 40, 40), QTextCharFormat::SpellCheckUnderline});
    }
    return out;
}

// The marks of the words in view (and a screenful above and below): the work of a key does not grow with the length of the story. It is done once for a state (a key that
// moves the caret, changes the words, and ends a step asks for it several times).
void StoryEdit::markWords()
{
    if (m_syncing) return;
    const int margin = 400;
    const int viewTop = cursorForPosition(QPoint(0, -margin)).block().position();
    const QTextBlock lastBlock = cursorForPosition(QPoint(viewport()->width(), std::max(viewport()->height(), margin) + margin)).block();
    const int viewEnd = lastBlock.position() + lastBlock.length();
    const bool typing = isTarget() && !m_ed->cursor().hasSelection();
    const std::array<qint64, 9> key = {qint64(reinterpret_cast<quintptr>(document())), qint64(document()->revision()), m_story ? qint64(m_story->serial) : -1, qint64(m_markVersion),
                                       qint64(typing ? m_ed->cursor().position() : -1), qint64(viewTop), qint64(viewEnd), qint64(m_ed->view.spelling), qint64(spellingRevision())};
    if (key == m_markKey) return;
    m_markKey = key;
    ++s_markRuns;
    QList<QTextEdit::ExtraSelection> marks;
    auto add = [&](const Mark &m, int last) {
        if (m.start < 0 || m.end > last || m.end <= m.start) return;
        QTextEdit::ExtraSelection x;
        x.cursor = QTextCursor(document());
        x.cursor.setPosition(m.start);
        x.cursor.setPosition(m.end, QTextCursor::KeepAnchor);
        x.format.setUnderlineStyle(m.style);
        x.format.setUnderlineColor(m.color);
        marks << x;
    };
    const int last = document()->characterCount() - 1;
    // Links, as the printed page draws them: the scheme's link color and an underline, unless the words name a color of their own.
    static const ColorScheme standard;
    const QColor linkColor = (m_ed->doc() ? m_ed->doc()->colors : standard).slot(Hyperlink);
    for (QTextBlock b = document()->findBlock(viewTop); b.isValid() && b.position() <= viewEnd; b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            if (!f.charFormat().isAnchor() || !f.charFormat().stringProperty(tp::ColorRefP).isEmpty()) continue;
            QTextEdit::ExtraSelection x;
            x.cursor = QTextCursor(document());
            x.cursor.setPosition(f.position());
            x.cursor.setPosition(f.position() + f.length(), QTextCursor::KeepAnchor);
            x.format.setForeground(linkColor);
            x.format.setFontUnderline(true);
            marks << x;
        }
    const QVector<Mark> spelling = spellingMarks(viewTop, viewEnd);
    for (const Mark &m : spelling) add(m, last);
    for (const Mark &m : m_marks)
        if (m.end >= viewTop && m.start <= viewEnd) add(m, last);
    s_marksBuilt += int(marks.size());
    setExtraSelections(marks);
}

// What the keys do: the ones that move the caret or the selection are the view's own; the ones that change the words go through the Editor (so they are steps of the
// presentation's undo history, with AutoCorrect, and read the same settings as a text box); the rest are left for the window's shortcuts.
void StoryEdit::keyPressEvent(QKeyEvent *e)
{
    if (isReadOnly() || m_ed->readOnly()) {
        const int k = e->key();
        if (k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down || k == Qt::Key_Home || k == Qt::Key_End || k == Qt::Key_PageUp || k == Qt::Key_PageDown ||
            e->matches(QKeySequence::SelectAll) || e->matches(QKeySequence::Copy))
            QTextEdit::keyPressEvent(e);
        else
            e->ignore();
        return;
    }
    if (!e->text().isEmpty()) Q_EMIT typingStarted();
    const int k = e->key();
    const bool moves = k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down || k == Qt::Key_Home || k == Qt::Key_End || k == Qt::Key_PageUp ||
                       k == Qt::Key_PageDown || e->matches(QKeySequence::SelectAll);
    if (!takeTarget()) {
        e->ignore();
        return;
    }
    if (moves) {
        m_ed->flushTyping();   // (the caret moves on: what was typed is a step, and the next change is another)
        QTextEdit::keyPressEvent(e);
        return;
    }
    if (m_ed->handleEditKey(e)) {
        e->accept();
        return;
    }
    e->ignore();
}

bool StoryEdit::event(QEvent *e)
{

    // The window's own shortcuts (Cut, Copy, Paste, Undo, Redo) are the commands of the whole presentation: this view does not take them for itself.
    if (e->type() == QEvent::ShortcutOverride && !isReadOnly() && !m_ed->readOnly()) {
        auto *k = static_cast<QKeyEvent *>(e);
        if (k->matches(QKeySequence::Undo) || k->matches(QKeySequence::Redo) || k->matches(QKeySequence::Cut) || k->matches(QKeySequence::Copy) || k->matches(QKeySequence::Paste)) {
            e->ignore();
            return true;
        }
    }
    return QTextEdit::event(e);
}

// The mouse reaches the view through its viewport: a press takes the caret for the notes (and closes the typing before it), and a release that ends a selection shows the bar.
bool StoryEdit::viewportEvent(QEvent *e)
{
    const bool press = e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick, release = e->type() == QEvent::MouseButtonRelease;
    if (press) {
        if (m_mini) m_mini->hide();
        m_ed->flushTyping();
        if (!isReadOnly() && !m_ed->readOnly() && m_story) takeTarget();
    }
    const bool done = QTextEdit::viewportEvent(e);
    if (release && m_mini && static_cast<QMouseEvent *>(e)->button() == Qt::LeftButton && textCursor().hasSelection() && !isReadOnly() && !m_ed->readOnly()) {
        QTextCursor a(document()), b(document());
        a.setPosition(textCursor().selectionStart());
        b.setPosition(textCursor().selectionEnd());
        QWidget *host = m_mini->parentWidget();
        auto rectOf = [&](const QTextCursor &c) {
            const QRect r = cursorRect(c);
            return QRect(viewport()->mapTo(host, r.topLeft()), r.size());
        };
        m_mini->showBeside(rectOf(a), rectOf(b));
    }
    return done;
}

MiniToolbar *StoryEdit::setMiniTarget(std::unique_ptr<MiniTarget> target)
{
    if (m_mini) return m_mini;
    QWidget *host = window();
    m_mini = new MiniToolbar(host, std::move(target));
    m_mini->hideOnTypingAndScrolling(this);
    connect(m_ed, &Editor::pageChanged, m_mini, &QWidget::hide);
    connect(m_ed, &Editor::documentReplaced, m_mini, &QWidget::hide);
    return m_mini;
}

void StoryEdit::inputMethodEvent(QInputMethodEvent *e)
{
    if (isReadOnly() || m_ed->readOnly()) {
        e->ignore();
        return;
    }
    if (!e->commitString().isEmpty() && takeTarget()) {
        Q_EMIT typingStarted();
        m_ed->commitText(e->commitString());
        e->accept();
        return;
    }
    QTextEdit::inputMethodEvent(e);
}

// (pasting and dropping go through the Editor's own paste, from the window's command; the view's own way would put in what the Editor does not know of)
void StoryEdit::insertFromMimeData(const QMimeData *) {}
bool StoryEdit::canInsertFromMimeData(const QMimeData *) const { return false; }
void StoryEdit::dropEvent(QDropEvent *e) { e->ignore(); }

void StoryEdit::showEvent(QShowEvent *e)
{
    QTextEdit::showEvent(e);
    setStory(m_id);   // (the story's document takes this view's width)
}

void StoryEdit::hideEvent(QHideEvent *e)
{
    QTextEdit::hideEvent(e);
    if (!isVisibleTo(window())) bind(nullptr);   // (the document is let go, with the sizes it had)
}

void StoryEdit::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);
    markWords();
}

void StoryEdit::focusInEvent(QFocusEvent *e)
{
    Q_EMIT typingStarted();
    QTextEdit::focusInEvent(e);
    if (e->reason() != Qt::PopupFocusReason) takeTarget();
}

void StoryEdit::focusOutEvent(QFocusEvent *e)
{
    m_ed->flushTyping();   // (what was typed is one step, saved when the caret leaves)
    QTextEdit::focusOutEvent(e);
}

void StoryEdit::contextMenuEvent(QContextMenuEvent *e)
{
    m_ed->flushTyping();
    QMenu menu(this);
    const QTextCursor at = cursorForPosition(e->pos());
    if (!isReadOnly() && !m_ed->readOnly() && m_story) {
        // A right-click in a word that is not in the selection puts the caret there (as in a text box), so the menu is of that word.
        if (!textCursor().hasSelection() || at.position() < textCursor().selectionStart() || at.position() > textCursor().selectionEnd()) {
            setTextCursor(at);
            takeTarget();
        }
    }
    fillMenu(menu, at.position());
    if (menu.isEmpty()) {
        QMenu *standard = createStandardContextMenu(e->pos());
        standard->setParent(&menu);
        menu.addActions(standard->actions());
    }
    menu.exec(e->globalPos());
}

} // namespace jp
