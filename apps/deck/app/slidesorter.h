#pragma once
// The Slide Sorter view: every slide as a thumbnail in a grid that wraps with
// the window, its number under it (struck through and its thumbnail faded when
// the slide is hidden, a star beside it when the slide has a transition). Click,
// Shift+click, and Ctrl+click select slides, Ctrl+A selects them all, and the
// arrow keys move the current slide through the grid. Dragging moves the
// selected slides; Delete, Ctrl+D, and the menu delete, duplicate, hide, or add
// a slide after them, each as one undo step. Double-clicking a slide or Enter
// opens it in Normal view. Ctrl+wheel changes the thumbnails' size.
// A thumbnail is drawn when it comes into view (the rest while idle) and kept
// until its slide changes.

#include <QAbstractScrollArea>
#include <QHash>
#include <QImage>
#include <QTimer>

namespace jp {

class Editor;

class SlideSorter : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit SlideSorter(Editor *ed, QWidget *parent = nullptr);

    int slideCount() const;
    // How many thumbnails fit across the window.
    int columns() const;
    // Where slide `slide`'s thumbnail is, in the viewport (empty for none).
    QRect thumbnailRect(int slide) const;
    // The slide at a point of the viewport, or -1.
    int slideAt(const QPoint &pos) const;
    // The selected slides, in order. The current slide (the editor's) is one of them.
    QList<int> selection() const { return m_sel; }
    void select(const QList<int> &slides, int current = -1);
    // How many slides have their thumbnail drawn.
    int renderedCount() const;

    // The thumbnails' size as a share of the usual one (1 is 100%).
    double zoom() const { return m_zoom; }
    void setZoom(double zoom);

    // Puts slides `slides` before slide `before` (the slide count for the
    // end), in the order they have, as one step that undoes. The moved slides
    // become the selection.
    void moveSlides(const QList<int> &slides, int before);
    void deleteSelected();      // all but one slide when all are selected
    void duplicateSelected();   // each copy right after its slide
    void toggleHiddenSelected();   // hides them, or shows them when all are hidden
    void newSlideAfterSelection();   // asks for it through newSlideWanted()

Q_SIGNALS:
    void openRequested(int slide);
    // A new slide is wanted after the current one, which is the last selected.
    void newSlideWanted();
    void zoomChanged(double zoom);

protected:
    bool event(QEvent *e) override;
    void scrollContentsBy(int dx, int dy) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    QSize thumbSize() const;
    QSize cellSize() const;
    QPoint cellOrigin(int slide) const;   // top left, in the viewport
    int rowCount() const;
    void layoutChanged();
    void documentChanged();
    void sync();
    void scheduleRender();
    void renderNext();
    void prune(int first, int last);
    QByteArray keyOf(int slide) const;
    QList<QByteArray> slideKeys() const;
    QStringList pageIds() const;
    QList<int> validSelection() const;
    void currentChanged();
    void setCurrent(int slide);
    void selectOnly(int slide);
    void selectRange(int from, int to);
    void moveCurrent(int slide, bool extend);
    void ensureVisible(int slide);
    int dropIndexAt(const QPoint &pos) const;
    QRect dropMark(int before) const;
    void finishDrag(const QPoint &pos);
    void autoScroll();
    void updateAccessibleText();

    Editor *m_ed;
    QList<int> m_sel;
    int m_anchor = 0;
    double m_zoom = 1.0;
    // The look of each slide when its thumbnail was last asked for, and the
    // thumbnails drawn, by that look (a slide that changes back, or a copy,
    // finds its thumbnail again).
    QList<QByteArray> m_keys;
    QStringList m_ids;         // the slides' ids then
    QHash<QByteArray, QImage> m_images;
    bool m_dirty = true;       // slides changed while the view was hidden
    bool m_selfChange = false; // the view itself is changing the editor's current slide
    QTimer m_syncTimer, m_renderTimer, m_scrollTimer;
    // A press on a slide: it may become a drag.
    int m_pressSlide = -1;
    QPoint m_pressPos, m_lastPos;
    bool m_collapseOnRelease = false;
    bool m_dragging = false;
    int m_dropBefore = -1;
};

} // namespace jp
