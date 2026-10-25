#pragma once
// The row of sheet tabs under the grid: a tab for each sheet that is shown (hidden and very hidden sheets are kept out), the color a sheet asks for as a stripe under its name. A person
// can pick several tabs (Ctrl and Shift with the click) to do something to all of them, drag a tab to move it, open a menu on it, and rename a sheet by a double click.

#include <QColor>
#include <QPointer>
#include <QSet>
#include <QTabBar>
#include <QVector>

class QLineEdit;

namespace jp::crunch {

class SheetTabBar : public QTabBar {
    Q_OBJECT
public:
    explicit SheetTabBar(QWidget *parent = nullptr);
    // The color of a tab's stripe (invalid for none).
    void setStripe(int tab, const QColor &color);
    QColor stripe(int tab) const { return m_stripes.value(tab); }
    void clearTabs();
    // The tabs picked, in order (the tab with the cursor is always among them).
    QVector<int> selectedTabs() const;
    void selectOnly(int tab);
    void setSelectedTabs(const QSet<int> &tabs);
    // Opens a line over the tab to type the sheet's name in; `renamed` says what was typed when it is finished (Enter, or the line losing the focus), and Esc gives it up.
    void beginRename(int tab);
    bool isRenaming() const { return !m_editor.isNull(); }
    // The line the name is typed in, while there is one.
    QLineEdit *editor() const { return m_editor; }
    // True while the tabs are being moved by a drag: the tab with the cursor changes as the tabs pass each other, and that is not a person choosing a sheet.
    bool isDragging() const { return m_moving; }
    // Gives the renaming up, without a name (the sheets are being changed, or the window is closing).
    void cancelRename();

Q_SIGNALS:
    void selectionChanged();
    void contextRequested(int tab, const QPoint &globalPos);
    void renamed(int tab, const QString &name);
    // A tab was dragged from one place to another and let go: where it started, and where it is now (one signal for the whole drag, not one for each tab passed).
    void tabDragged(int from, int to);

protected:
    void paintEvent(QPaintEvent *e) override;
    QSize tabSizeHint(int index) const override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void tabRemoved(int index) override;
    void tabInserted(int index) override;

private:
    QVector<QColor> m_stripes;
    QSet<int> m_selected;
    QPointer<QLineEdit> m_editor;
    bool m_pressed = false;   // the left button went down on a tab and has not come up
    int m_dragFrom = -1;      // the place of the tab pressed, when it was
    bool m_dragMoved = false; // tabs have moved since
    bool m_moving = false;    // inside a mouse move
};

} // namespace jp::crunch
