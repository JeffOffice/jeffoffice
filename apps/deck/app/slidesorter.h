#pragma once
// The Slide Sorter view: every slide as a thumbnail in a grid that wraps with
// the window, its number under it (struck through and its thumbnail faded when
// the slide is hidden, a star beside it when the slide has a transition). Click,
// Shift+click, and Ctrl+click select slides, Ctrl+A selects them all, and the
// arrow keys move the current slide through the grid. Dragging moves the
// selected slides; Delete, Ctrl+D, and the menu delete, duplicate, hide, or add
// a slide after them, each as one undo step. Double-clicking a slide or Enter
// opens it in Normal view. Ctrl+wheel changes the thumbnails' size.
// With sections, each starts on a row of its own under a heading with its name
// and how many slides it has: a click on the heading folds the section's
// slides away (and again brings them back), right-click opens its menu, and
// slides dropped among a section's are in the section.
// A thumbnail is drawn when it comes into view (the rest while idle) and kept
// until its slide changes.

#include <QAbstractScrollArea>
#include <QHash>
#include <QImage>
#include <QJsonArray>
#include <QSet>
#include <QTimer>
#include <QVector>

namespace jp {

class Editor;

class SlideSorter : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit SlideSorter(Editor *ed, QWidget *parent = nullptr);

    int slideCount() const;
    // How many thumbnails fit across the window.
    int columns() const;
    // Where slide `slide`'s thumbnail is, in the viewport (empty for none, or
    // when its section is folded away).
    QRect thumbnailRect(int slide) const;
    // The slide at a point of the viewport, or -1.
    int slideAt(const QPoint &pos) const;
    // Where section `section`'s heading is, in the viewport (empty for none), and the section whose heading is at a point, or -1.
    QRect sectionHeaderRect(int section) const;
    int headerAt(const QPoint &pos) const;
    // The sections folded away, by their ids (the slides pane folds the same
    // ones). The setters that name one section or all tell collapsedChanged().
    bool isCollapsed(int section) const;
    void setCollapsed(int section, bool collapsed);
    void setAllCollapsed(bool collapsed);
    QSet<QString> collapsedSections() const { return m_collapsed; }
    void setCollapsedSections(const QSet<QString> &ids);
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
    // become the selection. They are in section `into` (-1: the section of
    // the slide before them, the first section for the first place).
    void moveSlides(const QList<int> &slides, int before, int into = -1);
    void deleteSelected();      // all but one slide when all are selected
    void duplicateSelected();   // each copy right after its slide
    void toggleHiddenSelected();   // hides them, or shows them when all are hidden
    void newSlideAfterSelection();   // asks for it through newSlideWanted()

Q_SIGNALS:
    void openRequested(int slide);
    // A new slide is wanted after the current one, which is the last selected.
    void newSlideWanted();
    void zoomChanged(double zoom);
    void addSectionWanted(int slide);
    void renameSectionWanted(int section);
    void removeSectionWanted(int section);
    void removeSectionAndSlidesWanted(int section);
    void moveSectionWanted(int section, int by);
    void collapsedChanged();

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
    // Where a drop would put the moved slides: before slide `before`, in section `section` (-1 without sections).
    struct Drop {
        int before = -1, section = -1;
    };
    QSize thumbSize() const;
    QSize cellSize() const;
    void ensureLayout() const;
    bool slideShown(int slide) const;     // not in a section folded away
    QPoint cellOrigin(int slide) const;   // top left, in the viewport
    void visibleRange(int *first, int *last) const;
    int neighbor(int slide, int rows) const;   // the slide `rows` rows down (negative: up) at about the same place across, or `slide`
    int nextShown(int slide, int step) const;
    void layoutChanged();
    void documentChanged();
    void sync();
    void scheduleRender();
    void renderNext();
    void prune(int first, int last);
    QByteArray keyOf(int slide) const;
    static QByteArray lookOf(const QByteArray &key);
    QList<QByteArray> slideKeys() const;
    QStringList pageIds() const;
    QList<int> validSelection() const;
    void currentChanged();
    void setCurrent(int slide);
    void selectOnly(int slide);
    void selectRange(int from, int to);
    void moveCurrent(int slide, bool extend);
    void ensureVisible(int slide);
    Drop dropAt(const QPoint &pos) const;
    QRect dropMark(const Drop &drop) const;
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
    QHash<QByteArray, QImage> m_latest;   // the last thumbnail drawn for each look, at whatever size
    bool m_dirty = true;       // slides changed while the view was hidden
    bool m_selfChange = false; // the view itself is changing the editor's current slide
    QTimer m_syncTimer, m_renderTimer, m_scrollTimer;
    // A press on a slide: it may become a drag.
    int m_pressSlide = -1;
    QPoint m_pressPos, m_lastPos;
    bool m_collapseOnRelease = false;
    bool m_dragging = false;
    Drop m_drop;
    int m_pressHeader = -1;
    QSet<QString> m_collapsed;
    int m_collapsedSerial = 0;
    // The grid as last laid out (with the content's top left at the origin, before scrolling), and what it was laid out from.
    mutable bool m_laidOut = false;
    mutable QVector<QPoint> m_origins;   // each slide's cell; far off for one in a section folded away
    mutable QVector<QRect> m_headerRects;
    mutable QVector<int> m_sectionStart, m_sectionCount;
    mutable int m_contentHeight = 0, m_laidCount = -1, m_laidColumns = -1, m_laidWidth = -1, m_laidSerial = -1;
    mutable QSize m_laidCell;
    mutable QJsonArray m_laidSections;
};

} // namespace jp
