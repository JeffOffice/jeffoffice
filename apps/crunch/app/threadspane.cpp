#include "app/threadspane.h"

#include "app/plaintext.h"

#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace jp {

QString commentTimeShown(const QString &time, const QTimeZone &zone)
{
    QDateTime t = QDateTime::fromString(time.left(19), Qt::ISODate);
    if (!t.isValid()) return time;
    t.setTimeZone(QTimeZone(QTimeZone::UTC));   // (a file writes the time in UTC)
    return QLocale().toString(t.toTimeZone(zone), QLocale::ShortFormat);
}

CommentThreadsPane::CommentThreadsPane(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("commentsPane"));
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(m_scroll);
    setThreads({}, -1, -1, 0);
}

QString CommentThreadsPane::emptyText() const { return tr("There are no comments on this sheet. Select a cell, and choose New Comment to start a thread."); }

// The card of the cell with the cursor is marked (and the others are not): the cards are not made again for a cursor that moves.
void CommentThreadsPane::setCurrent(qint64 row, qint64 col)
{
    for (QFrame *card : findChildren<QFrame *>(QStringLiteral("commentCard"))) {
        const bool current = qint64(card->property("row").toUInt()) == row && qint64(card->property("col").toUInt()) == col;
        if (card->property("current").toBool() == current) continue;
        card->setProperty("current", current);
        card->setStyleSheet(current ? QStringLiteral("QFrame#commentCard { border: 2px solid palette(highlight); }") : QString());
    }
}

void CommentThreadsPane::setThreads(const std::vector<Thread> &threads, qint64 currentRow, qint64 currentCol, int more)
{
    ++m_calls;
    // (the old cards are let go of when the event that may be running one of their buttons is done)
    if (QWidget *old = m_scroll->takeWidget()) {
        old->hide();
        old->deleteLater();
    }
    m_body = new QWidget;
    m_layout = new QVBoxLayout(m_body);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(8);
    m_cards = 0;
    if (threads.empty()) {
        auto *empty = plainLabel(new QLabel(emptyText(), m_body));
        empty->setObjectName(QStringLiteral("commentsEmpty"));
        empty->setWordWrap(true);
        m_layout->addWidget(empty);
    }
    QWidget *previous = nullptr;   // (the buttons are reached with Tab in the order they are drawn)
    auto button = [&](QWidget *parent, const QString &text, const QString &name, const QString &accessible) {
        auto *b = new QPushButton(text, parent);
        b->setObjectName(name);
        b->setAccessibleName(accessible);
        if (previous) QWidget::setTabOrder(previous, b);
        previous = b;
        return b;
    };
    for (const Thread &t : threads) {
        const quint32 row = t.row, col = t.col;   // (what a card's buttons hold is the cell, not a copy of the thread)
        const bool resolved = t.resolved;
        auto *card = new QFrame(m_body);
        card->setObjectName(QStringLiteral("commentCard"));
        card->setProperty("cell", t.cell);
        card->setProperty("row", row);
        card->setProperty("col", col);
        card->setProperty("current", false);
        card->setAccessibleName(resolved ? tr("Comments on %1, resolved").arg(t.cell) : tr("Comments on %1").arg(t.cell));
        card->setFrameShape(QFrame::StyledPanel);
        auto *box = new QVBoxLayout(card);
        box->setContentsMargins(8, 6, 8, 6);
        box->setSpacing(4);
        auto *head = new QHBoxLayout;
        auto *cell = plainLabel(new QLabel(resolved ? tr("%1 (resolved)").arg(t.cell) : t.cell, card));
        cell->setObjectName(QStringLiteral("commentCell"));
        QFont f = cell->font();
        f.setBold(true);
        cell->setFont(f);
        head->addWidget(cell);
        head->addStretch(1);
        auto *go = button(card, tr("Go To"), QStringLiteral("commentGoTo"), tr("Go to %1").arg(t.cell));
        go->setFlat(true);
        connect(go, &QPushButton::clicked, this, [this, row, col] { Q_EMIT threadChosen(row, col); });
        head->addWidget(go);
        box->addLayout(head);
        int index = 0;
        for (const Comment &c : t.comments) {
            auto *who = plainLabel(new QLabel(c.time.isEmpty() ? c.author : tr("%1, %2").arg(c.author.isEmpty() ? tr("Unknown") : c.author, c.time), card));
            who->setObjectName(QStringLiteral("commentWho"));
            who->setContentsMargins(14 * std::min(c.depth, 3), 0, 0, 0);
            QFont w = who->font();
            w.setItalic(true);
            who->setFont(w);
            box->addWidget(who);
            auto *words = plainLabel(new QLabel(c.text.size() > 2000 ? c.text.left(2000) + QChar(0x2026) : c.text, card));
            words->setObjectName(QStringLiteral("commentText"));
            words->setWordWrap(true);
            words->setContentsMargins(14 * std::min(c.depth, 3), 0, 0, 0);
            box->addWidget(words);
            // an Edit for each comment, and a Delete for each reply (the first comment's Delete is the thread's, below)
            auto *line = new QHBoxLayout;
            line->setContentsMargins(14 * std::min(c.depth, 3), 0, 0, 0);
            auto *edit = button(card, tr("Edit"), QStringLiteral("commentEdit"), tr("Edit comment %1 on %2").arg(index + 1).arg(t.cell));
            edit->setEnabled(!t.locked);
            const int at = index;
            connect(edit, &QPushButton::clicked, this, [this, row, col, at] { Q_EMIT editRequested(row, col, at); });
            line->addWidget(edit);
            if (index > 0) {
                auto *del = button(card, tr("Delete"), QStringLiteral("commentDeleteReply"), tr("Delete comment %1 on %2").arg(index + 1).arg(t.cell));
                del->setEnabled(!t.locked);
                connect(del, &QPushButton::clicked, this, [this, row, col, at] { Q_EMIT deleteCommentRequested(row, col, at); });
                line->addWidget(del);
            }
            line->addStretch(1);
            box->addLayout(line);
            ++index;
        }
        if (t.more > 0) {
            auto *rest = plainLabel(new QLabel(tr("and %1 more").arg(t.more), card));
            rest->setObjectName(QStringLiteral("commentMore"));
            box->addWidget(rest);
        }
        auto *row2 = new QHBoxLayout;
        auto *reply = button(card, tr("Reply"), QStringLiteral("commentReply"), tr("Reply to %1").arg(t.cell));
        reply->setEnabled(!t.locked);
        connect(reply, &QPushButton::clicked, this, [this, row, col] { Q_EMIT replyRequested(row, col); });
        auto *resolve = button(card, resolved ? tr("Reopen") : tr("Resolve"), QStringLiteral("commentResolve"), resolved ? tr("Reopen thread on %1").arg(t.cell) : tr("Resolve thread on %1").arg(t.cell));
        resolve->setEnabled(!t.locked);
        connect(resolve, &QPushButton::clicked, this, [this, row, col, resolved] { Q_EMIT resolveRequested(row, col, !resolved); });
        auto *del = button(card, tr("Delete"), QStringLiteral("commentDelete"), tr("Delete the comments on %1").arg(t.cell));
        connect(del, &QPushButton::clicked, this, [this, row, col] { Q_EMIT deleteRequested(row, col); });
        row2->addWidget(reply);
        row2->addWidget(resolve);
        row2->addWidget(del);
        row2->addStretch(1);
        box->addLayout(row2);
        m_layout->addWidget(card);
        ++m_cards;
    }
    if (more > 0) {
        auto *rest = plainLabel(new QLabel(tr("and %1 more threads that are not shown").arg(more), m_body));
        rest->setObjectName(QStringLiteral("commentsMore"));
        m_layout->addWidget(rest);
    }
    m_layout->addStretch(1);
    m_scroll->setWidget(m_body);
    setCurrent(currentRow, currentCol);
}

} // namespace jp
