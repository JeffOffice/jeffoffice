#pragma once
// The Navigation pane, on the left of the window (View > Navigation Pane): Headings (the document's outline, to click, collapse, promote, and demote), Pages (small pictures
// of the pages), and Results (the places a word is found).

#include <QColor>
#include <QPair>
#include <QTimer>
#include <QVector>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QListWidget;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QFrame;
class QTextCursor;

namespace jp {

class WriteWindow;
namespace write { struct SectionRange; }

class NavigationPane : public QWidget {
    Q_OBJECT
public:
    explicit NavigationPane(WriteWindow *win, QWidget *parent = nullptr);
    ~NavigationPane() override;
    // Brings what the visible tab shows up to date with the document and the caret (made again only when the text has changed).
    void refresh();
    QTabWidget *tabs() const { return m_tabs; }
    QTreeWidget *headings() const { return m_headings; }
    QListWidget *pages() const { return m_pages; }
    QLineEdit *searchBox() const { return m_search; }
    QListWidget *results() const { return m_results; }
    // How the results are listed: 0 as the words round each (the order of the text), 1 under the heading each is in, 2 under the page each is on.
    QComboBox *groupBox() const { return m_group; }
    // Goes to the heading of a row (what a click does).
    void goToHeading(QTreeWidgetItem *item);
    // Promotes or demotes the heading of the row and the headings under it by one level; false where a level would pass the ends.
    bool moveHeadingLevel(QTreeWidgetItem *item, int by);
    // Moves the section of the heading of the row (the heading and what is under it, to the next of its level or a higher one) to before the heading of the row numbered `before` (the end of the
    // text when it is the number of rows), keeping its level, as one step of the history; says in the status bar why when it is refused. True if it moved.
    bool moveSectionTo(QTreeWidgetItem *item, int before);
    // Moves it past the section of the same level above it or below it (the context menu's and the keys' commands).
    bool moveSectionBy(QTreeWidgetItem *item, bool up);
    // While a heading is being dragged: the number of its row (-1 when none), the number of the heading the line shows it will go before (-1 where there is no place to drop it), and the line.
    int draggedHeading() const { return m_dragFrom; }
    int dropBefore() const { return m_dropBefore; }
    QFrame *dropLine() const { return m_line; }
    // Looks at the headings of the text at once (a command that moved a section calls it, so the rows are those of the text).
    void rescan();
    // Finds the text in the box (at most kMostResults places), listing each with the words round it.
    void search();
    static constexpr int kMostResults = 500;
    static constexpr int kPause = 300;   // milliseconds after the last edit before the headings are looked at again
    // For tests: how many times the headings tree was made, how many times the headings were looked for in the text, and how many page pictures were made.
    int rebuildCountForTest() const { return m_rebuildCount; }
    int scanCountForTest() const { return m_scanCount; }
    int thumbnailsMadeForTest() const { return m_thumbnailsMade; }
    // For tests: the rows looked at by the mouse moves of drags (a move looks at the rows under the pointer and the one below it, never at the text), and the paragraphs the last drop worked on.
    qint64 dragRowsLookedAtForTest() const { return m_dragRows; }
    qint64 dragMovesForTest() const { return m_dragMoves; }
    qint64 lastDropWorkForTest() const { return m_dropWork; }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool rowsAreStale();
    void keepCaret(const QPair<int, int> &was, const write::SectionRange &range, int movedTo);
    void dragMoved(const QPoint &at);
    void endDrag(const QPoint &at, bool drop);
    void scanHeadings();
    void rebuildPages();
    void fillThumbnails();
    void markCaretHeading();
    int numberOf(QTreeWidgetItem *item);
    WriteWindow *m_win;
    QTabWidget *m_tabs;
    QTreeWidget *m_headings;
    QListWidget *m_pages, *m_results;
    QLineEdit *m_search;
    QComboBox *m_group;
    quint64 m_headingsOf = 0, m_pagesOf = 0;
    int m_revision = -1, m_pageRevision = -1;
    bool m_dying = false;
    QTimer m_timer;                                  // (the pause after the last edit, before the headings are looked at again)
    QVector<QTreeWidgetItem *> m_items;              // the tree's rows in the order of the text
    QVector<int> m_positions;                        // where each heading is now, in order
    QVector<std::pair<QString, int>> m_shown;        // the words and level of each heading the tree shows
    int m_rebuildCount = 0, m_scanCount = 0, m_thumbnailsMade = 0;
    QColor m_lineColor;
    QFrame *m_line = nullptr;                        // (the line that shows where a dragged heading will land)
    QPoint m_pressAt;
    int m_pressed = -1, m_dragFrom = -1, m_dragEnd = 0, m_dropBefore = -1;   // (the row pressed, the row dragged, the number after its section's last heading, the place the line shows)
    bool m_swallowRelease = false;
    qint64 m_dragRows = 0, m_dragMoves = 0, m_dropWork = 0;
};

} // namespace jp
