#pragma once
// The slide being edited: the shared page canvas, with what a presentation
// shows on it. A placeholder has a dotted outline while it is empty, selected,
// or being typed in (never every object); Guides are dashed lines through the
// slide's center and Gridlines are dots a twelfth of an inch apart. While
// comments are shown, each of the slide's comments has a marker where its place
// is: a click opens the comment, and dragging it moves the comment.

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
    bool commentsShown() const { return m_comments; }
    void setCommentsShown(bool on);
    // The comment the markers mark as open (the one the Comments pane has).
    QString activeComment() const { return m_activeComment; }
    void setActiveComment(const QString &id);
    // The comment whose marker is at a point of the view, or empty.
    QString commentAt(const QPointF &view) const;

Q_SIGNALS:
    void commentClicked(const QString &id);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    bool viewportEvent(QEvent *e) override;

private:
    QRectF commentMarker(const QPointF &page) const;
    bool m_guides = false, m_grid = false, m_comments = false;
    QString m_activeComment;
    QString m_dragId;                  // the comment whose marker is being dragged
    QPointF m_dragFrom, m_dragOffset, m_dragAt;
    bool m_dragMoved = false;
    EditorMedia *m_media = nullptr;
};

} // namespace jp
