#pragma once
// The slide thumbnails down the window's left side, each with its number at
// its left: click one to edit it, Up and Down move between them, drag to
// reorder, and right-click for New, Duplicate, Delete, and Hide. The current
// slide has an accent outline and a hidden slide's number is struck through
// and its thumbnail faded.

#include <QListWidget>
#include <QTimer>

namespace jp {

class Editor;

class SlidesPane : public QListWidget {
    Q_OBJECT
public:
    // On each item: whether its slide is hidden.
    static constexpr int HiddenRole = Qt::UserRole + 1;

    explicit SlidesPane(Editor *ed, QWidget *parent = nullptr);
    // Draws the thumbnails again (soon, or now).
    void refresh(bool soon = false);
    // Where row `row`'s thumbnail is, in the pane's viewport.
    QRect thumbnailRect(int row) const;

Q_SIGNALS:
    void newSlideWanted();
    void duplicateWanted();
    void deleteWanted();
    void hideWanted();

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void dropEvent(QDropEvent *e) override;

private:
    void rebuild();
    Editor *m_ed;
    QTimer m_timer;
    bool m_updating = false;
};

} // namespace jp
