#pragma once
// The Comments pane, beside the slide: the current slide's comments as cards
// (author, date, text, the replies, a box to reply in, Resolve, Edit, and
// Delete), and New Comment at the top, which opens a box to type the comment
// in. Every change is one undo step through the Editor, so the cards follow
// the slide's comments whatever changes them, an undo included.

#include <QColor>
#include <QHash>
#include <QString>
#include <QWidget>
#include <functional>

class QBoxLayout;
class QFrame;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

namespace jp {

class Editor;
struct Page;
namespace deck {
struct Comment;
}

// The color a person's comments are drawn in (gray once they are resolved).
QColor commentColor(const QString &author, bool done = false);
// Asks for the name and initials of the person making comments when File >
// Options has none, and keeps them there. False when the person declines.
bool askCommentAuthor(QWidget *parent);

class CommentsPane : public QWidget {
    Q_OBJECT
public:
    explicit CommentsPane(Editor *ed, QWidget *parent = nullptr);

    // The comment the pane has open (its card is marked, its marker on the
    // slide too), or empty. An id that isn't on the slide being edited is none.
    QString currentComment() const { return m_current; }
    void setCurrentComment(const QString &id);
    // Opens the box that a new comment is typed in.
    void startNewComment();
    // Posts the box's text as a new comment on the slide, at a place of its own.
    void postNewComment();
    // Takes away the open comment, the slide's comments, or the presentation's, each as one step.
    void deleteCurrent();
    void deleteSlideComments();
    void deleteAllComments();
    // Opens the comment after the open one (or before it) in the presentation, on its slide.
    void visit(int direction);
    // Draws the cards again when the slide's comments are not what they show.
    void reload();

Q_SIGNALS:
    void currentCommentChanged(const QString &id);
    void closeRequested();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void changeEvent(QEvent *e) override;
    QSize sizeHint() const override { return QSize(300, 400); }

private:
    void rebuild();
    QFrame *buildCard(const deck::Comment &c);
    // A person's mark: their initials in their color.
    QLabel *badge(const QString &author, const QString &initials, bool done, int size);
    void postReply(const QString &commentId, const QString &text);
    void saveEdit(const QString &id, const QString &text);
    void setResolved(const QString &id, bool done);
    void deleteOne(const QString &id);
    void change(const QString &label, const std::function<void(Page &)> &fn);
    QPointF placeForNew() const;

    Editor *m_ed;
    QPushButton *m_newButton = nullptr, *m_postButton = nullptr, *m_cancelButton = nullptr;
    QFrame *m_draftCard = nullptr;
    QPlainTextEdit *m_draft = nullptr;
    QLabel *m_empty = nullptr;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_cards = nullptr;
    QVBoxLayout *m_cardsLayout = nullptr;
    QString m_current, m_editing, m_pageId, m_signature;
    bool m_rebuilding = false, m_scrollToCurrent = false;
    QHash<QString, QString> m_replyDrafts;   // what is typed in each comment's reply box and not yet posted
};

} // namespace jp
