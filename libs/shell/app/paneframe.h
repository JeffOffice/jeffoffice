#pragma once
// The frame of a task pane that formats what is selected: a title with a close button, icon tabs across the top, and under them the
// page of the tab chosen, a column of sections that fold up. It holds no controls of its own; the apps' pages fill it.

#include <QFrame>
#include <QIcon>
#include <QToolButton>
#include <QVector>

class QLabel;
class QScrollArea;
class QStackedWidget;
class QVBoxLayout;

namespace jp {

// A heading that folds the controls under it away (and brings them back). The heading is a button: Space and Enter fold it, and screen
// readers are told whether it is open.
class PaneSection : public QWidget {
    Q_OBJECT
public:
    explicit PaneSection(const QString &title, QWidget *parent = nullptr);
    QString title() const { return m_title; }
    bool isExpanded() const { return m_expanded; }
    void setExpanded(bool on);
    QToolButton *header() const { return m_header; }
    // Where the controls go.
    QVBoxLayout *body() const { return m_bodyLayout; }
    QWidget *bodyWidget() const { return m_body; }

Q_SIGNALS:
    void toggled(bool expanded);

private:
    void refreshHeader();
    QString m_title;
    bool m_expanded = true;
    QToolButton *m_header;
    QWidget *m_body;
    QVBoxLayout *m_bodyLayout;
};

// One tab's page: sections one under the other.
class PanePage : public QWidget {
    Q_OBJECT
public:
    explicit PanePage(QWidget *parent = nullptr);
    PaneSection *addSection(const QString &title, bool expanded = true);
    QList<PaneSection *> sections() const { return m_sections; }
    PaneSection *section(const QString &title) const;

private:
    QVBoxLayout *m_layout;
    QList<PaneSection *> m_sections;
};

class PaneFrame : public QFrame {
    Q_OBJECT
public:
    explicit PaneFrame(QWidget *parent = nullptr);
    QString title() const;
    void setTitle(const QString &title);
    // A tab with its page (scrolled when it is taller than the pane); the tab shows only the icon, and `name` is its tool tip and what
    // screen readers say. Returns the index.
    PanePage *addPage(const QString &id, const QIcon &icon, const QString &name);
    int pageCount() const { return m_tabs.size(); }
    PanePage *page(int i) const;
    QString pageId(int i) const;
    int indexOfPage(const QString &id) const;
    QToolButton *tabButton(int i) const { return m_tabs.value(i); }
    int currentPage() const { return m_current; }
    void setCurrentPage(int i);
    // A tab can be taken away (a picture's tab while something else is selected); the current page moves to one that shows.
    void setPageVisible(int i, bool visible);
    bool isPageVisible(int i) const;
    // With text, the pane shows it in place of the tabs and pages (nothing is selected); an empty text brings them back.
    void setMessage(const QString &text);
    QString message() const;
    QToolButton *closeButton() const { return m_close; }
    // The keyboard goes to the current tab.
    void focusTabs();

Q_SIGNALS:
    void closeRequested();
    void escaped();                    // Esc in the pane: the app takes the keyboard back to the page
    void currentPageChanged(int index);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    QLabel *m_title;
    QToolButton *m_close;
    QWidget *m_tabRow;
    QStackedWidget *m_stack;
    QLabel *m_message;
    QVector<QToolButton *> m_tabs;
    QVector<QString> m_ids;
    QVector<QScrollArea *> m_scrolls;
    QVector<PanePage *> m_pages;
    int m_current = -1;
};

} // namespace jp
