#pragma once
// The Reviewing Pane of JeffDeck's Compare: the changes the other presentation has (one row each, listed lazily, at most review::kMaxChanges), with
// Accept and Reject for the one chosen, Previous and Next, Accept All and Reject All, filters by slide, author, and state, and the words of a text
// change set out word by word (the shared word diff, libs/text/review/textdiff.h). A second page lists the comments of the presentation. The pane
// holds no presentation: the window owns the comparison and gives the pane the list to show (refresh() after it changes), and the window does what
// the buttons ask, each as one undo step.

#include "text/review.h"

#include <QAbstractListModel>
#include <QWidget>
#include <functional>

class QComboBox;
class QLabel;
class QListView;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QTabBar;
class QTextBrowser;

namespace jp {

// The rows of the list: indices into the change list that the filters let through. Nothing about a row is made until the view asks for it.
class DeckReviewModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit DeckReviewModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}
    enum Roles { ChangeIndexRole = Qt::UserRole + 1, StateRole };
    void setList(const review::ChangeList *list);
    // The rows again, for the filters (empty: any); `state` -1 is any, else a review::State.
    void setFilter(const QString &container, const QString &author, int state);
    // After states or the list changed: the rows are made again and the view is told.
    void reload();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override { return parent.isValid() ? 0 : int(m_rows.size()); }
    QVariant data(const QModelIndex &index, int role) const override;
    int rowOfChange(int change) const;
    int changeOfRow(int row) const { return row >= 0 && row < m_rows.size() ? m_rows[row] : -1; }

private:
    const review::ChangeList *m_list = nullptr;
    QString m_container, m_author;
    int m_state = -1;
    QVector<int> m_rows;
};

class DeckReviewPane : public QWidget {
    Q_OBJECT
public:
    explicit DeckReviewPane(QWidget *parent = nullptr);

    struct Source {
        const review::ChangeList *list = nullptr;
        std::function<QString(const QString &container)> slideLabel;   // the words for a slide's id in the filter
        std::function<bool(int change, QString *before, QString *after)> texts;   // the whole text before and after a text change
    };
    void setSource(const Source &source);
    const review::ChangeList *sourceList() const { return m_src.list; }
    // The comments page: one row each, "slide: author: text".
    struct CommentRow {
        int slide = 0;
        QString id, author, text;
    };
    void setComments(const QVector<CommentRow> &rows);
    // Draws again what the list holds (states changed, or the list).
    void refresh();

    // The change chosen (an index into the list), or -1; choosing one is what the window follows to its slide.
    int currentChange() const;
    void setCurrentChange(int change);
    // The change after the chosen one (before it) that is pending, among the rows the filters show, wrapping round; when none is pending, the next row.
    void step(bool forward);
    int visibleCount() const;
    QListView *list() const { return m_list; }
    // The page of the pane that shows: 0 the changes, 1 the comments.
    int page() const;
    void setPage(int page);

Q_SIGNALS:
    void acceptRequested(int change);
    void rejectRequested(int change);
    void acceptAllRequested();
    void rejectAllRequested();
    void changeChosen(int change);
    void commentChosen(const QString &id, int slide);
    void pageChanged(int page);
    void closeRequested();

private:
    void filtersChanged();
    void showDetail();
    void updateButtons();
    void rebuildFilters();

    Source m_src;
    DeckReviewModel *m_model = nullptr;
    QTabBar *m_tabs = nullptr;
    QStackedWidget *m_pages = nullptr;
    QLabel *m_status = nullptr;
    QComboBox *m_slide = nullptr, *m_author = nullptr, *m_state = nullptr;
    QListView *m_list = nullptr;
    QTextBrowser *m_detail = nullptr;
    QPushButton *m_accept = nullptr, *m_reject = nullptr, *m_prev = nullptr, *m_next = nullptr, *m_acceptAll = nullptr, *m_rejectAll = nullptr;
    QListWidget *m_comments = nullptr;
    bool m_rebuilding = false;
};

} // namespace jp
