#include "app/commentspane.h"

#include "app/plaintext.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace jp {

CommentsPane::CommentsPane(QWidget *parent) : QWidget(parent)
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

QString CommentsPane::emptyText() const { return tr("There are no comments on this sheet. Select a cell, and choose New Comment to start a thread."); }

void CommentsPane::setThreads(const std::vector<Thread> &threads, qint64 currentRow, qint64 currentCol, int more)
{
    // (nothing is drawn again when the cards would be the same)
    QString signature = QString::number(currentRow) + QLatin1Char(',') + QString::number(currentCol) + QLatin1Char(',') + QString::number(more);
    for (const Thread &t : threads) {
        signature += QLatin1Char('|') + t.cell + (t.resolved ? QLatin1Char('r') : QLatin1Char('o')) + (t.locked ? QLatin1Char('l') : QLatin1Char('u')) + QString::number(t.more);
        for (const Comment &c : t.comments) signature += QLatin1Char('\x1f') + c.author + QLatin1Char('\x1e') + c.time + QLatin1Char('\x1e') + c.text + QLatin1Char('\x1e') + QString::number(c.depth);
    }
    if (m_body && signature == m_signature) return;
    m_signature = signature;
    delete m_scroll->takeWidget();
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
    for (const Thread &t : threads) {
        auto *card = new QFrame(m_body);
        card->setObjectName(QStringLiteral("commentCard"));
        card->setProperty("cell", t.cell);
        card->setFrameShape(QFrame::StyledPanel);
        const bool current = qint64(t.row) == currentRow && qint64(t.col) == currentCol;
        card->setProperty("current", current);
        if (current) card->setStyleSheet(QStringLiteral("QFrame#commentCard { border: 2px solid palette(highlight); }"));
        auto *box = new QVBoxLayout(card);
        box->setContentsMargins(8, 6, 8, 6);
        box->setSpacing(4);
        auto *head = new QHBoxLayout;
        auto *cell = plainLabel(new QLabel(t.resolved ? tr("%1 (resolved)").arg(t.cell) : t.cell, card));
        cell->setObjectName(QStringLiteral("commentCell"));
        QFont f = cell->font();
        f.setBold(true);
        cell->setFont(f);
        head->addWidget(cell);
        head->addStretch(1);
        auto *go = new QPushButton(tr("Go To"), card);
        go->setObjectName(QStringLiteral("commentGoTo"));
        go->setFlat(true);
        connect(go, &QPushButton::clicked, this, [this, t] { Q_EMIT threadChosen(t.row, t.col); });
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
            ++index;
        }
        if (t.more > 0) {
            auto *rest = plainLabel(new QLabel(tr("and %1 more").arg(t.more), card));
            rest->setObjectName(QStringLiteral("commentMore"));
            box->addWidget(rest);
        }
        auto *row = new QHBoxLayout;
        auto *reply = new QPushButton(tr("Reply"), card);
        reply->setObjectName(QStringLiteral("commentReply"));
        reply->setEnabled(!t.locked);
        connect(reply, &QPushButton::clicked, this, [this, t] { Q_EMIT replyRequested(t.row, t.col); });
        auto *resolve = new QPushButton(t.resolved ? tr("Reopen") : tr("Resolve"), card);
        resolve->setObjectName(QStringLiteral("commentResolve"));
        resolve->setEnabled(!t.locked);
        connect(resolve, &QPushButton::clicked, this, [this, t] { Q_EMIT resolveRequested(t.row, t.col, !t.resolved); });
        auto *del = new QPushButton(tr("Delete"), card);
        del->setObjectName(QStringLiteral("commentDelete"));
        connect(del, &QPushButton::clicked, this, [this, t] { Q_EMIT deleteRequested(t.row, t.col); });
        row->addWidget(reply);
        row->addWidget(resolve);
        row->addWidget(del);
        row->addStretch(1);
        box->addLayout(row);
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
}

} // namespace jp
