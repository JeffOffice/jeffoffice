#pragma once
// The Reviewing pane, on the right of the window (Review > Reviewing Pane): the tracked changes of the document (who, when, and what, with Accept and Reject), and its comments in
// threads (with Reply, Resolve, and Delete). A click on one shows it in the text.

#include "core/commentops.h"
#include "core/reviewops.h"

#include <QTimer>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace jp {

class WriteWindow;

class ReviewPane : public QWidget {
    Q_OBJECT
public:
    explicit ReviewPane(WriteWindow *win, QWidget *parent = nullptr);
    // Brings the lists up to date with the document (when the text, the comments, or who reviews has changed since they were made).
    void refresh();
    // Which list is shown.
    enum Page { Comments, Changes };
    void showPage(Page page);
    Page page() const;
    QTreeWidget *commentList() const { return m_comments; }
    QTreeWidget *changeList() const { return m_changes; }
    QLineEdit *replyField() const { return m_reply; }
    QComboBox *personBox() const { return m_person; }
    // The Id of the comment chosen in the list (-1 for none), and the change chosen.
    int chosenComment() const;
    // Posts the text of the reply field as a reply to the chosen comment; false where there is no comment chosen or no text.
    bool postReply();
    // How many times the lists have been made (for tests).
    int rebuildCountForTest() const { return m_rebuilds; }
    // The people whose changes are listed: all, or one.
    QString chosenPerson() const;

private:
    void rebuild();
    void showChosenChange();
    void showChosenComment();
    void resolveChange(bool accept);
    void toggleResolved();
    void deleteChosen();
    void editChosen();
    void updateButtons();
    WriteWindow *m_win;
    QTabWidget *m_tabs;
    QTreeWidget *m_comments, *m_changes;
    QLineEdit *m_reply;
    QComboBox *m_person;
    QPushButton *m_new, *m_post, *m_resolve, *m_delete, *m_prevComment, *m_nextComment;
    QPushButton *m_accept, *m_reject, *m_acceptAll, *m_rejectAll, *m_prevChange, *m_nextChange;
    const void *m_madeDoc = nullptr;
    int m_madeRevision = -1, m_madeView = -1;
    QString m_madePerson;
    int m_rebuilds = 0;
    QTimer m_idle;
    bool m_building = false;
};

} // namespace jp
