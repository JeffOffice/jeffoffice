#pragma once
// The Navigation pane, on the left of the window (View > Navigation Pane): Headings (the document's outline, to click, collapse, promote, and demote), Pages (small pictures
// of the pages), and Results (the places a word is found).

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

namespace jp {

class WriteWindow;

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
    // Finds the text in the box (at most kMostResults places), listing each with the words round it.
    void search();
    static constexpr int kMostResults = 500;
    static constexpr int kPause = 300;   // milliseconds after the last edit before the headings are looked at again
    // For tests: how many times the headings tree was made, how many times the headings were looked for in the text, and how many page pictures were made.
    int rebuildCountForTest() const { return m_rebuildCount; }
    int scanCountForTest() const { return m_scanCount; }
    int thumbnailsMadeForTest() const { return m_thumbnailsMade; }

private:
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
};

} // namespace jp
