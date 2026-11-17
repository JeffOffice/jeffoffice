#include "canvas/editorminitoolbar.h"
#include "canvas/canvas.h"
#include <QMouseEvent>
#include <QTimer>

namespace jp {

EditorMiniToolbar::EditorMiniToolbar(Canvas *canvas, std::unique_ptr<MiniTarget> target, std::function<bool()> blocked)
    : QObject(canvas), m_canvas(canvas), m_bar(new MiniToolbar(canvas->viewport(), std::move(target))), m_blocked(std::move(blocked))
{
    m_bar->hideOnTypingAndScrolling(canvas);
    canvas->viewport()->installEventFilter(this);
    connect(canvas->editor(), &Editor::textCursorChanged, this, [this] { update(false); });
    connect(canvas->editor(), &Editor::selectionChanged, this, [this] { update(false); });
}

bool EditorMiniToolbar::eventFilter(QObject *watched, QEvent *event)
{
    // (The canvas handles the release itself first: look once it has.)
    if (watched == m_canvas->viewport() && event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton)
        QTimer::singleShot(0, this, [this] { update(true); });
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
    QWidget *view = m_canvas->viewport();
    m_bar->showBeside(QRect(view->mapFromGlobal(a.topLeft()), a.size()), QRect(view->mapFromGlobal(b.topLeft()), b.size()));
}

} // namespace jp
