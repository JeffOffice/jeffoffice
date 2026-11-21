#include "app/splitmover.h"

#include <QApplication>
#include <QKeyEvent>
#include <QSplitter>
#include <QSplitterHandle>

namespace jp {

namespace {
// The divider being moved is drawn in the highlight color.
void mark(const SplitMover::Handle &h, bool on)
{
    if (!h.splitter) return;
    QSplitterHandle *handle = h.splitter->handle(h.index);
    handle->setProperty("moving", on);
    handle->setStyleSheet(on ? QStringLiteral("background: palette(highlight);") : QString());
}
} // namespace

SplitMover::SplitMover(QObject *parent) : QObject(parent) {}

SplitMover::~SplitMover()
{
    if (m_active) qApp->removeEventFilter(this);
}

bool SplitMover::start(const QList<QSplitter *> &splitters)
{
    stop();
    for (QSplitter *s : splitters) {
        int last = -1;
        for (int i = 0; s && i < s->count(); ++i) {
            if (!s->widget(i)->isVisibleTo(s->window())) continue;
            if (last >= 0) m_handles.push_back({s, i, last});
            last = i;
        }
    }
    if (m_handles.isEmpty()) return false;
    m_active = true;
    qApp->installEventFilter(this);
    select(0);
    return true;
}

void SplitMover::stop()
{
    if (!m_active) return;
    m_active = false;
    qApp->removeEventFilter(this);
    for (const Handle &h : std::as_const(m_handles))
        mark(h, false);
    m_handles.clear();
    Q_EMIT finished();
}

void SplitMover::select(int which)
{
    for (const Handle &h : std::as_const(m_handles))
        mark(h, false);
    m_current = (which % int(m_handles.size()) + int(m_handles.size())) % int(m_handles.size());
    mark(m_handles[m_current], true);
    Q_EMIT prompt(tr("Move Split: use the arrow keys to move the divider, Tab for the next one, and Enter when it is where you want it."));
}

void SplitMover::move(int pixels)
{
    const Handle &h = m_handles[m_current];
    if (!h.splitter) return;
    QList<int> sizes = h.splitter->sizes();
    if (h.index >= sizes.size()) return;
    // (neither pane goes below a small share of what the two have, so a divider is never lost)
    const int both = sizes[h.before] + sizes[h.index];
    const int least = std::min(both / 2, 40);
    const int first = std::clamp(sizes[h.before] + pixels, least, both - least);
    sizes[h.before] = first;
    sizes[h.index] = both - first;
    h.splitter->setSizes(sizes);
}

bool SplitMover::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        stop();
        return false;
    }
    if (event->type() != QEvent::KeyPress) return false;
    auto *key = static_cast<QKeyEvent *>(event);
    const Handle &h = m_handles[m_current];
    if (!h.splitter) {
        stop();
        return false;
    }
    const bool horizontal = h.splitter->orientation() == Qt::Horizontal;
    const int step = kStep * (key->modifiers() & Qt::ShiftModifier ? 4 : 1);
    switch (key->key()) {
    case Qt::Key_Left: if (horizontal) move(-step); break;
    case Qt::Key_Right: if (horizontal) move(step); break;
    case Qt::Key_Up: if (!horizontal) move(-step); break;
    case Qt::Key_Down: if (!horizontal) move(step); break;
    case Qt::Key_Tab: select(m_current + 1); break;
    case Qt::Key_Backtab: select(m_current - 1); break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Escape: stop(); break;
    default: return false;
    }
    return true;
}

} // namespace jp
