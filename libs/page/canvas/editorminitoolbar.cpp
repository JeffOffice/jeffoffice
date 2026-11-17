#include "canvas/editorminitoolbar.h"
#include "canvas/canvas.h"
#include <QMouseEvent>

namespace jp {

EditorMiniToolbar::EditorMiniToolbar(Canvas *canvas, std::unique_ptr<MiniTarget> target, std::function<bool()> blocked, std::function<void()> refresh)
    : QObject(canvas), m_canvas(canvas), m_bar(new MiniToolbar(canvas->viewport(), std::move(target))), m_blocked(std::move(blocked)), m_refresh(std::move(refresh))
{
    m_bar->hideOnTypingAndScrolling(canvas);
    canvas->viewport()->installEventFilter(this);
    connect(canvas->editor(), &Editor::textCursorChanged, this, [this] { update(false); });
    connect(canvas->editor(), &Editor::selectionChanged, this, [this] { update(false); });
    // A change of the text moves the words (the bar follows or goes); a change of the view, the page, or the file leaves it where no words are.
    connect(canvas->editor(), &Editor::changed, this, [this] { update(false); });
    connect(canvas->editor(), &Editor::pageChanged, this, [this] { m_bar->hide(); });
    connect(canvas->editor(), &Editor::documentReplaced, this, [this] { m_bar->hide(); });
    connect(canvas, &Canvas::zoomChanged, this, [this] { m_bar->hide(); });
}

bool EditorMiniToolbar::eventFilter(QObject *watched, QEvent *event)
{
    // (The canvas handles the release itself first: look once it has.)
    if (watched == m_canvas->viewport() && event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton)
        QMetaObject::invokeMethod(this, [this] { update(true); }, Qt::QueuedConnection);
    return false;
}

void EditorMiniToolbar::update(bool show)
{
    Editor *ed = m_canvas->editor();
    if (!ed->isEditingText() || !ed->cursor().hasSelection() || (!show && !m_bar->isVisible()) || (m_blocked && m_blocked())) {
        m_bar->hide();
        return;
    }
    const QTextCursor &c = ed->cursor();
    const QRect a = m_canvas->caretScreenRect(c.selectionStart()), b = m_canvas->caretScreenRect(c.selectionEnd());
    if (a.isNull() || b.isNull()) {
        m_bar->hide();
        return;
    }
    if (show && m_refresh) m_refresh();   // (every new selection: the words it holds may look different from the last)
    QWidget *view = m_canvas->viewport();
    m_bar->showBeside(QRect(view->mapFromGlobal(a.topLeft()), a.size()), QRect(view->mapFromGlobal(b.topLeft()), b.size()));
}

} // namespace jp
