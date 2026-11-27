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
#include <QClipboard>
#include <QMenu>
#include <QMimeData>
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
#include <functional>

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
    // (first: from here to the end of the base class's destructor, nothing watching this view may reach it; its members go before its children and its document do)
    m_markers->detach();
    disconnect(this, nullptr, this, nullptr);
    disconnect(verticalScrollBar(), nullptr, this, nullptr);
    if (m_story && m_story->doc) disconnect(m_story->doc.get(), nullptr, this, nullptr);
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
    if (old && old->doc) {
        disconnect(old->doc.get(), &QTextDocument::contentsChange, this, &StoryEdit::keepFirstBlockLaidOut);
        disconnect(old->doc.get(), &QTextDocument::contentsChanged, this, &StoryEdit::repairFirstBlock);
        disconnect(old->doc.get(), &QTextDocument::contentsChange, this, &StoryEdit::watchLongWords);
    }
    // (the wrapping is the view's, and is set before the words are laid out: they are laid out once, in the way they are going to be shown)
    const QTextOption::WrapMode wrap = story && hasLongWord(d) ? QTextOption::WrapAnywhere : QTextOption::WrapAtWordBoundaryOrAnywhere;
    if (wordWrapMode() != wrap) {
        setDocument(m_none);
        setWordWrapMode(wrap);
    }
    setDocument(d);
    if (story) {
        connect(d, &QTextDocument::contentsChange, this, &StoryEdit::keepFirstBlockLaidOut, Qt::UniqueConnection);
        connect(d, &QTextDocument::contentsChanged, this, &StoryEdit::repairFirstBlock, Qt::UniqueConnection);
        connect(d, &QTextDocument::contentsChange, this, &StoryEdit::watchLongWords, Qt::UniqueConnection);
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

// The document layout throws away the first paragraph's lines when only its paragraph mark's look changes (Bold on the first word: the change is 0 characters at 0) and lays out none of
// them again, unless some other layout work is still due: the paragraph kept its words and lost its height (Presenter View). The paragraph is laid out again here, as for any change of it.
void StoryEdit::keepFirstBlockLaidOut(int pos, int removed, int added)
{
    if (pos != 0 || removed != 0 || added != 0 || sender() != document()) return;
    m_firstMarkChanged = true;   // (the layout has not run yet: it does after this signal, and the document says so when it is done)
}

void StoryEdit::repairFirstBlock()
{
    if (!m_firstMarkChanged || sender() != document()) return;
    m_firstMarkChanged = false;
    QTextDocument *d = document();
    const QTextBlock first = d->begin();
    if (first.isValid() && first.layout() && first.layout()->lineCount() == 0 && first.length() > 1) d->markContentsDirty(0, first.length());
}

// A word longer than a line is laid out by looking over all of it for each line it takes, so the work grows with the square of its length (a file's note of one word of 20,000 letters
// took 15 seconds to show, and 50,000 letters over 100). A word of kLongRun letters or more (a link has a few hundred at most) makes the view wrap anywhere, which looks at the letters
// of a line only. Only a block longer than that is looked into, and only until such a word is found.
bool StoryEdit::hasLongWord(const QTextDocument *d)
{
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        if (b.length() <= kLongRun) continue;
        const QString text = b.text();
        int run = 0;
        for (const QChar ch : text) {
            ++s_runLooked;
            run = ch.isSpace() ? 0 : run + 1;
            if (run >= kLongRun) return true;
        }
    }
    return false;
}

// A word grown to that length by typing (the letters just before and after the change are counted, no more than the limit on each side).
void StoryEdit::watchLongWords(int pos, int removed, int added)
{
    Q_UNUSED(removed);
    if (added <= 0 || wordWrapMode() == QTextOption::WrapAnywhere || sender() != document() || document()->findBlock(pos).length() <= kLongRun) return;
    QTextDocument *d = document();
    int run = added;
    for (int i = pos - 1; i >= 0 && run < kLongRun; --i, ++run) {
        ++s_runLooked;
        if (d->characterAt(i).isSpace()) break;
    }
    for (int i = pos + added; i < d->characterCount() && run < kLongRun; ++i, ++run) {
        ++s_runLooked;
        if (d->characterAt(i).isSpace()) break;
    }
    if (run < kLongRun || m_wrapQueued) return;
    m_wrapQueued = true;
    QMetaObject::invokeMethod(this, [this] {   // (not inside the change that is being made)
        m_wrapQueued = false;
        if (m_story && hasLongWord(document())) setWordWrapMode(QTextOption::WrapAnywhere);
    }, Qt::QueuedConnection);
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

// Whether a key is one that would change the words (a letter, Backspace, Delete, Enter, Tab, and the clipboard, history, and look commands). Keys typed in the notes never drive
// anything else, such as Presenter View's show: they are the notes' own, whether the change is made or refused.
static bool wouldChangeWords(const QKeyEvent *e)
{
    const int k = e->key();
    // (Alt with a letter is a menu's access key, and Meta a shortcut, wherever the program runs but a Mac, where Alt types other letters; Ctrl with Enter or Tab is a shortcut too)
    const Qt::KeyboardModifiers mods = e->modifiers();
#ifdef Q_OS_MACOS
    const bool shortcut = mods & Qt::MetaModifier;
#else
    const bool shortcut = mods & (Qt::AltModifier | Qt::MetaModifier);
#endif
    if (shortcut) return false;
    const bool types = !e->text().isEmpty() && e->text().at(0).isPrint() && !(mods & Qt::ControlModifier);
    if (mods & Qt::ControlModifier) {
        if (k == Qt::Key_Return || k == Qt::Key_Enter || k == Qt::Key_Tab || k == Qt::Key_Backtab) return false;
    }
    return types || k == Qt::Key_Backspace || k == Qt::Key_Delete || k == Qt::Key_Return || k == Qt::Key_Enter || k == Qt::Key_Tab || k == Qt::Key_Backtab || e->matches(QKeySequence::Cut) ||
           e->matches(QKeySequence::Paste) || e->matches(QKeySequence::Undo) || e->matches(QKeySequence::Redo) || e->matches(QKeySequence::Bold) || e->matches(QKeySequence::Italic) ||
           e->matches(QKeySequence::Underline);
}

// What the keys do: the ones that move the caret or the selection are the view's own; the ones that change the words go through the Editor (so they are steps of the
// presentation's undo history, with AutoCorrect, and read the same settings as a text box); the rest are left for the window's shortcuts.
void StoryEdit::keyPressEvent(QKeyEvent *e)
{
    if (isReadOnly() || m_ed->readOnly()) {
        const int k = e->key();
        if (k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down || k == Qt::Key_Home || k == Qt::Key_End || k == Qt::Key_PageUp || k == Qt::Key_PageDown ||
            e->matches(QKeySequence::SelectAll) || e->matches(QKeySequence::Copy)) {
            QTextEdit::keyPressEvent(e);
        } else {
            // (a key that would have changed the words says so, and goes no further; the rest, Esc and a lone Ctrl, are not an attempt to edit)
            if (wouldChangeWords(e)) {
                Q_EMIT readOnlyRefused();
                e->accept();
                return;
            }
            e->ignore();
        }
        return;
    }
    // Undo, Redo, Cut, Copy, and Paste reach the view only where the window has no shortcut of its own for them (a window with a menu takes them first, see event()): Presenter View.
    // They are the same commands: one history for the whole presentation, the Editor's own clipboard commands.
    if (e->matches(QKeySequence::Undo) || e->matches(QKeySequence::Redo)) {
        if (e->matches(QKeySequence::Undo)) m_ed->undo();
        else m_ed->redo();
        e->accept();
        return;
    }
    if (e->matches(QKeySequence::Cut) || e->matches(QKeySequence::Copy) || e->matches(QKeySequence::Paste)) {
        if (takeTarget()) {
            if (e->matches(QKeySequence::Cut)) m_ed->cut();
            else if (e->matches(QKeySequence::Copy)) m_ed->copy();
            else m_ed->paste(false);
        }
        e->accept();
        return;
    }
    if (!e->text().isEmpty()) Q_EMIT typingStarted();
    const int k = e->key();
    const bool moves = k == Qt::Key_Left || k == Qt::Key_Right || k == Qt::Key_Up || k == Qt::Key_Down || k == Qt::Key_Home || k == Qt::Key_End || k == Qt::Key_PageUp ||
                       k == Qt::Key_PageDown || e->matches(QKeySequence::SelectAll);
    if (!takeTarget()) {
        if (wouldChangeWords(e)) e->accept();
        else e->ignore();
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
    // A mouse event on the view itself (not on its viewport) is on the gutter beside the words, where a list's markers are: it is the click of the line it is beside, at the line's edge.
    if ((e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease || e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonDblClick) && !m_ghost) {
        auto *m = static_cast<QMouseEvent *>(e);
        const QPointF at = m->position() - QPointF(viewport()->pos());
        if (at.x() < 0 || at.x() >= viewport()->width()) {
            if (at.y() >= 0 && at.y() < viewport()->height()) {
                const QPointF edge(std::clamp(at.x(), 0.0, double(viewport()->width() - 1)), at.y());
                QMouseEvent inside(m->type(), edge, m->scenePosition(), m->globalPosition(), m->button(), m->buttons(), m->modifiers(), m->pointingDevice());
                QApplication::sendEvent(viewport(), &inside);
            }
            e->accept();
            return true;
        }
    }

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
        if (!e->commitString().isEmpty()) Q_EMIT readOnlyRefused();
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
        // The host has no commands of its own for the words: the editing commands are the Editor's (the one history, the Editor's clipboard), enabled when they can act.
        const bool editable = !isReadOnly() && !m_ed->readOnly() && m_story;
        const bool selected = textCursor().hasSelection();
        const QMimeData *clip = QApplication::clipboard()->mimeData();
        const bool hasWords = clip && (clip->hasText() || clip->hasHtml() || clip->hasFormat(QStringLiteral("application/x-jeffpub-text")));
        auto add = [&](const QString &text, bool on, const std::function<void()> &run) { menu.addAction(text, this, run)->setEnabled(on); };
        add(tr("Undo"), editable && m_ed->undoStack()->canUndo(), [this] { m_ed->undo(); });
        add(tr("Redo"), editable && m_ed->undoStack()->canRedo(), [this] { m_ed->redo(); });
        menu.addSeparator();
        add(tr("Cut"), editable && selected, [this] { if (takeTarget()) m_ed->cut(); });
        add(tr("Copy"), selected, [this] {
            if (isReadOnly() || m_ed->readOnly() || !m_story) copy();
            else if (takeTarget()) m_ed->copy();
        });
        add(tr("Paste"), editable && hasWords, [this] { if (takeTarget()) m_ed->paste(false); });
        menu.addSeparator();
        add(tr("Select All"), m_story && document()->characterCount() > 1, [this] { selectAll(); });
    }
    menu.exec(e->globalPos());
}

} // namespace jp
