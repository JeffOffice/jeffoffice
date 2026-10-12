#pragma once
// The slide thumbnails down the window's left side: click one to edit it,
// drag to reorder, and right-click for New, Duplicate, Delete, and Hide.

#include <QListWidget>
#include <QTimer>

namespace jp {

class Editor;

class SlidesPane : public QListWidget {
    Q_OBJECT
public:
    explicit SlidesPane(Editor *ed, QWidget *parent = nullptr);
    // Draws the thumbnails again (soon, or now).
    void refresh(bool soon = false);

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
