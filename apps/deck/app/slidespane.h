#pragma once
// The slide thumbnails down the window's left side, each with its number at
// its left: click one to edit it, Up and Down move between them, drag to
// reorder, and right-click for New, Duplicate, Delete, Hide, and Add Section.
// The current slide has an accent outline and a hidden slide's number is struck
// through and its thumbnail faded.
//
// A presentation with sections has a heading row before each section's
// slides, with its name and how many slides it has. A click on it folds the
// section's slides away (and again brings them back), and right-click opens
// its menu. A slide dropped under a heading is in that section.

#include <QJsonArray>
#include <QListWidget>
#include <QSet>
#include <QTimer>
#include <QVector>

namespace jp {

class Editor;

class SlidesPane : public QListWidget {
    Q_OBJECT
public:
    // On each row: whether its slide is hidden; the slide it shows (-1 on a
    // section's heading); and on a heading the section's index, how many
    // slides it has, and whether it is folded away.
    static constexpr int HiddenRole = Qt::UserRole + 1;
    static constexpr int SlideRole = Qt::UserRole + 2;
    static constexpr int SectionRole = Qt::UserRole + 3;
    static constexpr int CountRole = Qt::UserRole + 4;
    static constexpr int CollapsedRole = Qt::UserRole + 5;

    explicit SlidesPane(Editor *ed, QWidget *parent = nullptr);
    // Draws the thumbnails again (soon, or now).
    void refresh(bool soon = false);
    // Shows the editor's current slide as the current one, without drawing
    // the thumbnails again (unless slides were added or removed, or the
    // sections changed). A slide in a section that is folded away opens it.
    void followCurrent();
    // Where slide `slide`'s thumbnail is, in the pane's viewport (empty when
    // its section is folded away).
    QRect thumbnailRect(int slide) const;

    // The rows: a heading for each section, then its slides.
    bool isHeaderRow(int row) const;
    int slideOfRow(int row) const;      // -1 for a heading, or no such row
    int rowOfSlide(int slide) const;    // -1 for no such slide
    int headerRow(int section) const;   // -1 for no such section
    QRect headerRect(int section) const;
    // The sections folded away, by their ids (the slide sorter folds the same
    // ones). The setters that name one section or all tell collapsedChanged().
    bool isCollapsed(int section) const;
    void setCollapsed(int section, bool collapsed);
    void setAllCollapsed(bool collapsed);
    QSet<QString> collapsedSections() const { return m_collapsed; }
    void setCollapsedSections(const QSet<QString> &ids);

    // Makes slide `from` slide `to` (what a drop does), in section `into` (-1:
    // the one it lands in), as one step that undoes.
    void moveSlide(int from, int to, int into = -1);

Q_SIGNALS:
    void newSlideWanted();
    void duplicateWanted();
    void deleteWanted();
    void hideWanted();
    void addSectionWanted(int slide);
    void renameSectionWanted(int section);
    void removeSectionWanted(int section);
    void removeSectionAndSlidesWanted(int section);
    void moveSectionWanted(int section, int by);
    void collapsedChanged();

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

private:
    void rebuild();
    bool slideHidden(int slide) const;   // in a section folded away
    Editor *m_ed;
    QTimer m_timer;
    bool m_updating = false;
    QSet<QString> m_collapsed;
    // The rows as last built: what each shows, and the sections' first rows.
    QVector<int> m_slideOfRow, m_rowOfSlide, m_headerRows;
    QJsonArray m_shownSections;
    int m_pressedHeader = -1;
};

} // namespace jp
