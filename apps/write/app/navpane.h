#pragma once
// The Navigation pane, on the left of the window (View > Navigation Pane, Ctrl+F): Headings (the document's outline, to click, collapse, promote, and demote), Pages (small pictures
// of the pages), and Results (the places a word is found).

#include <QWidget>

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
    // Goes to the heading of a row (what a click does).
    void goToHeading(QTreeWidgetItem *item);
    // Promotes or demotes the heading of the row and the headings under it by one level; false where a level would pass the ends.
    bool moveHeadingLevel(QTreeWidgetItem *item, int by);
    // Finds the text in the box (at most kMostResults places), listing each with the words round it.
    void search();
    static constexpr int kMostResults = 500;

private:
    void rebuildHeadings();
    void rebuildPages();
    void markCaretHeading();
    WriteWindow *m_win;
    QTabWidget *m_tabs;
    QTreeWidget *m_headings;
    QListWidget *m_pages, *m_results;
    QLineEdit *m_search;
    quint64 m_headingsOf = 0, m_pagesOf = 0;
    int m_revision = -1, m_pageRevision = -1;
    bool m_dying = false;
};

} // namespace jp
