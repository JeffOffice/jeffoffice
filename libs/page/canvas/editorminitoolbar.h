#pragma once
// The mini toolbar for text typed on a page (a slide's or a publication's text box, shape, or table cell): shown above the words when a
// mouse selection of them ends, following the selection while it stays, and gone with it, with a key that types, with Escape, and with
// scrolling. The bar is the shell's; this finds the words and the place.

#include "core/export.h"
#include "app/minitoolbar.h"
#include <QObject>
#include <QPointer>
#include <functional>

namespace jp {

class Canvas;
class Editor;

class JO_COMMON_EXPORT EditorMiniToolbar : public QObject {
    Q_OBJECT
public:
    // `blocked`: asked before the bar is shown (a window that can be read-only says so here). `refresh`: called as the bar appears, so that its buttons show the selection's state at once.
    EditorMiniToolbar(Canvas *canvas, std::unique_ptr<MiniTarget> target, std::function<bool()> blocked = nullptr, std::function<void()> refresh = nullptr);
    MiniToolbar *bar() const { return m_bar.data(); }
    // Shown when the mouse selection that has ended holds words; `show` false only keeps a bar that is up with the selection.
    void update(bool show);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Canvas *m_canvas;
    // (the canvas's view and the bar on it go before this object does, while the canvas is half taken down: everything checks these
    // first and never asks the canvas then. Nov 17: a Deck window closed in a test crashed now and then, "free(): invalid pointer")
    QPointer<QWidget> m_view;
    QPointer<MiniToolbar> m_bar;
    std::function<bool()> m_blocked;
    std::function<void()> m_refresh;
};

} // namespace jp
