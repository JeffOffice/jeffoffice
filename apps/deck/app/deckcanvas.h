#pragma once
// The slide being edited: the shared page canvas, with what a presentation
// shows on it. A placeholder has a dotted outline while it is empty, selected,
// or being typed in (never every object); Guides are dashed lines through the
// slide's center and Gridlines are dots a twelfth of an inch apart.

#include "canvas/canvas.h"

namespace jp {

class DeckCanvas : public Canvas {
public:
    using Canvas::Canvas;
    bool guides() const { return m_guides; }
    void setGuides(bool on);
    bool gridlines() const { return m_grid; }
    void setGridlines(bool on);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    bool m_guides = false, m_grid = false;
};

} // namespace jp
