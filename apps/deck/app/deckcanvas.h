#pragma once
// The slide being edited: the shared page canvas, with what a presentation
// shows on it. A placeholder has a dotted outline while it is empty, selected,
// or being typed in (never every object); Guides are dashed lines through the
// slide's center and Gridlines are dots a twelfth of an inch apart. While
// comments are shown, the shared canvas draws a marker for each of the slide's comments.
// While the Animations tab or the Animation Pane shows, a small number sits by each
// shape that is animated (the click its effect plays at).

#include "canvas/canvas.h"

namespace jp {

class EditorMedia;

class DeckCanvas : public Canvas {
    Q_OBJECT
public:
    explicit DeckCanvas(Editor *ed, QWidget *parent = nullptr);
    // The movies and sounds on the slide: their control bars, and playing them in place.
    EditorMedia *media() const { return m_media; }
    bool guides() const { return m_guides; }
    void setGuides(bool on);
    bool gridlines() const { return m_grid; }
    void setGridlines(bool on);
    // A number by a shape of the slide: its top left in the page's coordinates, and what to say. None to take them away.
    struct AnimationBadge {
        QPointF at;
        QString text;
        bool picked = false;
        int slot = 0;     // which of the shape's effects, counting leftward
    };
    void setAnimationBadges(const QVector<AnimationBadge> &badges);
    const QVector<AnimationBadge> &animationBadges() const { return m_badges; }
    // Where the menu opens when the keyboard asks for it (Shift+F10, the Menu key), on the screen: below the text cursor,
    // else in the middle of the selection, else in the middle of the view.
    QPoint menuAnchor() const;

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    bool viewportEvent(QEvent *e) override;

private:
    void paintMasterPlaceholders();
    bool m_guides = false, m_grid = false;
    QVector<AnimationBadge> m_badges;
    EditorMedia *m_media = nullptr;
};

} // namespace jp
