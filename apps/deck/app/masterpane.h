#pragma once
// Slide Master view's pane, in the place of the slide thumbnails: each slide master of the presentation as a thumbnail with its name, and
// under it, indented, its layouts. A click on a row edits that master or layout on the canvas. Right-click opens the menu the window gives it
// (Insert Slide Master, Insert Layout, Rename, Delete, and Preserve). A page no slide stands on is not marked; the Delete command says why a
// page that slides stand on stays.

#include <QHash>
#include <QListWidget>
#include <QTimer>

namespace jp {

class Editor;

class MasterPane : public QListWidget {
    Q_OBJECT
public:
    // On each row: the page's id and how far in it stands (0 for a slide master, 1 for a layout).
    static constexpr int IdRole = Qt::UserRole + 1;
    static constexpr int DepthRole = Qt::UserRole + 2;

    explicit MasterPane(Editor *ed, QWidget *parent = nullptr);
    // Draws the thumbnails again (soon, or now): only the pages that changed.
    void refresh(bool soon = false);
    // Shows the page the editor is editing as the current row, without drawing the thumbnails again.
    void followCurrent();
    // The pages in the order of the rows, and what a row says.
    QStringList ids() const;
    int depthOf(const QString &id) const;
    QString nameOf(const QString &id) const;
    QString currentId() const;
    // Edits the page.
    void selectPage(const QString &id);
    // Where the page's thumbnail is, in the pane's viewport.
    QRect thumbnailRect(const QString &id) const;
    // The commands of the right-click menu (a null action is a separator).
    void setMenuActions(const QList<QAction *> &actions);

private:
    void rebuild();
    int rowOf(const QString &id) const;
    Editor *m_ed;
    QTimer m_timer;
    bool m_updating = false;
    // What each page looked like when its thumbnail was drawn.
    QHash<QString, QByteArray> m_seen;
};

} // namespace jp
