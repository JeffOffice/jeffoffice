// The Comments pane: the sheet's threads of comments as cards (the cell, then each comment with who wrote it and when, the replies set in, and Reply, Resolve or Reopen, and Delete under
// it). The pane shows what it is given and asks for changes through its signals; the window makes them (core/editcomments.cpp), and gives the cards again.
#pragma once

#include <QString>
#include <QWidget>

#include <vector>

class QLabel;
class QScrollArea;
class QVBoxLayout;

namespace jp {

class CommentsPane : public QWidget {
    Q_OBJECT
public:
    struct Comment {
        QString author, time, text;
        int depth = 0;   // how many replies deep (the first is 0)
    };
    struct Thread {
        quint32 row = 0, col = 0;
        QString cell;
        bool resolved = false, locked = false;   // locked: the file's thread has more comments than are changed
        std::vector<Comment> comments;
        int more = 0;                            // comments left out of the card
    };
    // The most cards drawn and the most comments one card shows.
    static constexpr int kMaxCards = 300;
    static constexpr int kMaxShownComments = 40;

    explicit CommentsPane(QWidget *parent = nullptr);
    // Draws the cards. `current` is the thread the cursor is on (its card is marked), or none; `more` is how many threads are not drawn.
    void setThreads(const std::vector<Thread> &threads, qint64 currentRow, qint64 currentCol, int more);
    int cardCount() const { return m_cards; }
    QString emptyText() const;

Q_SIGNALS:
    void threadChosen(quint32 row, quint32 col);
    void replyRequested(quint32 row, quint32 col);
    void resolveRequested(quint32 row, quint32 col, bool resolved);
    void deleteRequested(quint32 row, quint32 col);
    void editRequested(quint32 row, quint32 col, int index);

protected:
    QSize sizeHint() const override { return QSize(300, 400); }

private:
    QScrollArea *m_scroll = nullptr;
    QWidget *m_body = nullptr;
    QVBoxLayout *m_layout = nullptr;
    int m_cards = 0;
    QString m_signature;
};

} // namespace jp
