#include "app/commentspane.h"

#include "app/editor.h"
#include "app/icons.h"
#include "app/settings.h"
#include "core/comments.h"
#include "core/deck.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QToolButton>
#include <QVBoxLayout>

namespace jp {

namespace {

const char *const kIdProperty = "commentId";

QString say(const char *text) { return QCoreApplication::translate("CommentsPane", text); }

QString whenText(const QDateTime &t) { return t.isValid() ? QLocale().toString(t.toLocalTime(), QLocale::ShortFormat) : QString(); }

QString gray() { return uiDark() ? QStringLiteral("#9AA1AD") : QStringLiteral("#5F6773"); }

} // namespace

QColor commentColor(const QString &author, bool done)
{
    static const QColor colors[8] = {QColor(0x1A, 0x5F, 0xD6), QColor(0xC2, 0x4E, 0x12), QColor(0x2E, 0x7D, 0x4F), QColor(0x7E, 0x3A, 0xA6),
                                     QColor(0xB3, 0x2D, 0x2E), QColor(0x0E, 0x7C, 0x8C), QColor(0x9A, 0x6B, 0x00), QColor(0x4F, 0x5F, 0x73)};
    if (done) return QColor(0x8A, 0x8F, 0x98);
    quint32 sum = 0;
    for (const QChar c : author) sum = sum * 31 + c.unicode();
    return colors[sum % 8];
}

bool askCommentAuthor(QWidget *parent)
{
    if (deck::commentAuthor().known()) return true;
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("commentAuthorDialog"));
    dlg.setWindowTitle(say("Your Name"));
    auto *form = new QFormLayout(&dlg);
    auto *intro = new QLabel(say("Comments show who wrote them. Enter your name and initials; File > Options keeps them."), &dlg);
    intro->setWordWrap(true);
    form->addRow(intro);
    // The name the system knows the person by is offered.
    auto *name = new QLineEdit(Settings::get().userName(), &dlg);
    name->setObjectName(QStringLiteral("commentAuthorName"));
    auto *initials = new QLineEdit(deck::initialsFor(name->text()), &dlg);
    initials->setObjectName(QStringLiteral("commentAuthorInitials"));
    form->addRow(say("Name:"), name);
    form->addRow(say("Initials:"), initials);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(buttons);
    auto typedInitials = std::make_shared<bool>(false);
    auto update = [=] { buttons->button(QDialogButtonBox::Ok)->setEnabled(!name->text().trimmed().isEmpty()); };
    QObject::connect(name, &QLineEdit::textChanged, &dlg, [=](const QString &t) {
        if (!*typedInitials) initials->setText(deck::initialsFor(t));
        update();
    });
    QObject::connect(initials, &QLineEdit::textEdited, &dlg, [=] { *typedInitials = true; });
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    update();
    if (dlg.exec() != QDialog::Accepted) return false;
    deck::setCommentAuthor({name->text(), initials->text()});
    return deck::commentAuthor().known();
}

CommentsPane::CommentsPane(Editor *ed, QWidget *parent) : QWidget(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("commentsPane"));
    setAccessibleName(tr("Comments"));
    setMinimumWidth(240);
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet(QStringLiteral("#commentsPane{background:%1; border-left:1px solid %2;}").arg(uiPanel().name(), uiLine().name()));

    auto *column = new QVBoxLayout(this);
    column->setContentsMargins(8, 8, 8, 8);
    column->setSpacing(8);
    auto *head = new QHBoxLayout;
    auto *title = new QLabel(tr("Comments"), this);
    QFont bold = title->font();
    bold.setBold(true);
    title->setFont(bold);
    head->addWidget(title, 1);
    auto *close = new QToolButton(this);
    close->setObjectName(QStringLiteral("commentsClose"));
    close->setIcon(icon(QStringLiteral("x")));
    close->setAutoRaise(true);
    close->setToolTip(tr("Close"));
    close->setAccessibleName(tr("Close"));
    connect(close, &QToolButton::clicked, this, &CommentsPane::closeRequested);
    head->addWidget(close);
    column->addLayout(head);

    m_newButton = new QPushButton(icon(QStringLiteral("message-square-plus")), tr("New Comment"), this);
    m_newButton->setObjectName(QStringLiteral("commentNew"));
    connect(m_newButton, &QPushButton::clicked, this, [this] {
        if (askCommentAuthor(this)) startNewComment();
    });
    column->addWidget(m_newButton);

    // The box a new comment is typed in.
    m_draftCard = new QFrame(this);
    m_draftCard->setObjectName(QStringLiteral("commentDraftCard"));
    auto *draftColumn = new QVBoxLayout(m_draftCard);
    draftColumn->setContentsMargins(0, 0, 0, 0);
    m_draft = new QPlainTextEdit(m_draftCard);
    m_draft->setObjectName(QStringLiteral("commentDraft"));
    m_draft->setPlaceholderText(tr("Type a comment"));
    m_draft->setAccessibleName(tr("New comment"));
    m_draft->setFixedHeight(84);
    m_draft->installEventFilter(this);
    draftColumn->addWidget(m_draft);
    auto *draftButtons = new QHBoxLayout;
    draftButtons->addStretch(1);
    m_postButton = new QPushButton(tr("Post"), m_draftCard);
    m_postButton->setObjectName(QStringLiteral("commentPost"));
    m_postButton->setProperty("primary", true);
    m_postButton->setEnabled(false);
    m_cancelButton = new QPushButton(tr("Cancel"), m_draftCard);
    m_cancelButton->setObjectName(QStringLiteral("commentDraftCancel"));
    draftButtons->addWidget(m_postButton);
    draftButtons->addWidget(m_cancelButton);
    draftColumn->addLayout(draftButtons);
    m_draftCard->hide();
    column->addWidget(m_draftCard);
    connect(m_draft, &QPlainTextEdit::textChanged, this, [this] { m_postButton->setEnabled(!m_draft->toPlainText().trimmed().isEmpty()); });
    connect(m_postButton, &QPushButton::clicked, this, &CommentsPane::postNewComment);
    connect(m_cancelButton, &QPushButton::clicked, this, [this] {
        m_draft->clear();
        m_draftCard->hide();
        m_empty->setVisible(m_cardsLayout->count() <= 1);
    });

    m_empty = new QLabel(tr("There are no comments on this slide."), this);
    m_empty->setObjectName(QStringLiteral("commentsEmpty"));
    m_empty->setWordWrap(true);
    m_empty->setStyleSheet(QStringLiteral("color:%1;").arg(gray()));
    column->addWidget(m_empty);

    m_scroll = new QScrollArea(this);
    m_scroll->setObjectName(QStringLiteral("commentsList"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->viewport()->setAutoFillBackground(false);
    m_cards = new QWidget();
    m_cards->setObjectName(QStringLiteral("commentCards"));
    m_cardsLayout = new QVBoxLayout(m_cards);
    m_cardsLayout->setContentsMargins(1, 1, 1, 1);
    m_cardsLayout->setSpacing(8);
    m_scroll->setWidget(m_cards);
    m_cards->setAutoFillBackground(false);   // (setWidget turns it on)
    column->addWidget(m_scroll, 1);

    connect(ed, &Editor::changed, this, &CommentsPane::reload);
    connect(ed, &Editor::pageChanged, this, &CommentsPane::reload);
    connect(ed, &Editor::documentReplaced, this, &CommentsPane::reload);
    reload();
}

void CommentsPane::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    // A new style sheet changes the palette in turn: only a change of colors needs the cards drawn again.
    if (e->type() == QEvent::PaletteChange) {
        const QString sheet = QStringLiteral("#commentsPane{background:%1; border-left:1px solid %2;}").arg(uiPanel().name(), uiLine().name());
        if (sheet == styleSheet()) return;
        setStyleSheet(sheet);
        m_signature.clear();
        reload();
    }
}

bool CommentsPane::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_draft && e->type() == QEvent::KeyPress) {
        auto *k = static_cast<QKeyEvent *>(e);
        if (k->key() == Qt::Key_Escape) {
            m_cancelButton->click();
            return true;
        }
        if ((k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter) && (k->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))) {
            m_postButton->click();
            return true;
        }
    } else if (e->type() == QEvent::MouseButtonPress && o->objectName() == QLatin1String("commentCard")) {
        // A click on a card opens it, once the click is over with the card.
        const QString id = o->property(kIdProperty).toString();
        QPointer<CommentsPane> self(this);
        QMetaObject::invokeMethod(this, [self, id] {
            if (self) self->setCurrentComment(id);
        }, Qt::QueuedConnection);
    }
    return QWidget::eventFilter(o, e);
}

void CommentsPane::startNewComment()
{
    m_draftCard->show();
    m_empty->hide();
    m_postButton->setEnabled(!m_draft->toPlainText().trimmed().isEmpty());
    m_draft->setFocus();
}

void CommentsPane::change(const QString &label, const std::function<void(Page &)> &fn)
{
    Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    if (!d || i < 0 || i >= d->pages.size()) return;
    m_ed->beginChange(label);
    fn(*d->pages[i]);
    m_ed->endChange();
}

QPointF CommentsPane::placeForNew() const
{
    const Document *d = m_ed->doc();
    const QSizeF size = d->pageSize();
    // Beside the object that is selected, else the first free place along the top of the slide.
    if (const Item *it = m_ed->single())
        return QPointF(std::clamp(it->rect.right(), 0.0, std::max(0.0, size.width() - 24)), std::clamp(it->rect.top(), 0.0, std::max(0.0, size.height() - 24)));
    const QVector<deck::Comment> existing = deck::commentsOf(*d->pages[m_ed->currentPage()]);
    for (int k = 0; k < 40; ++k) {
        const QPointF p(10 + 24 * (k % 20), 10 + 24 * (k / 20));
        bool taken = false;
        for (const deck::Comment &c : existing) taken = taken || (qAbs(c.at.x() - p.x()) < 20 && qAbs(c.at.y() - p.y()) < 20);
        if (!taken) return p;
    }
    return QPointF(10, 10);
}

void CommentsPane::postNewComment()
{
    const QString text = m_draft->toPlainText().trimmed();
    if (text.isEmpty() || !askCommentAuthor(this)) return;
    const deck::CommentAuthor who = deck::commentAuthor();
    const QPointF at = placeForNew();
    QString id;
    change(tr("New Comment"), [&](Page &slide) { id = deck::addComment(slide, who, text, at); });
    m_draft->clear();
    m_draftCard->hide();
    setCurrentComment(id);
}

void CommentsPane::postReply(const QString &commentId, const QString &text)
{
    const QString t = text.trimmed();
    if (t.isEmpty() || !askCommentAuthor(this)) return;
    const deck::CommentAuthor who = deck::commentAuthor();
    m_replyDrafts.remove(commentId);
    m_current = commentId;
    change(tr("Reply to Comment"), [&](Page &slide) { deck::addReply(slide, commentId, who, t); });
}

void CommentsPane::saveEdit(const QString &id, const QString &text)
{
    if (text.trimmed().isEmpty()) return;
    m_editing.clear();
    change(tr("Edit Comment"), [&](Page &slide) { deck::editComment(slide, id, text.trimmed()); });
    reload();
}

void CommentsPane::setResolved(const QString &id, bool done)
{
    m_current = id;
    change(done ? tr("Resolve Comment") : tr("Reopen Comment"), [&](Page &slide) { deck::resolveComment(slide, id, done); });
}

void CommentsPane::deleteOne(const QString &id)
{
    const Document *d = m_ed->doc();
    const bool comment = d && m_ed->currentPage() >= 0 && m_ed->currentPage() < d->pages.size() && deck::findComment(*d->pages[m_ed->currentPage()], id);
    change(comment ? tr("Delete Comment") : tr("Delete Reply"), [&](Page &slide) { deck::deleteComment(slide, id); });
}

void CommentsPane::deleteCurrent()
{
    if (!m_current.isEmpty()) deleteOne(m_current);
}

void CommentsPane::deleteSlideComments()
{
    change(tr("Delete Comments"), [](Page &slide) { deck::deleteComments(slide); });
}

void CommentsPane::deleteAllComments()
{
    Document *d = m_ed->doc();
    if (!d) return;
    m_ed->beginChange(tr("Delete Comments"));
    deck::deleteAllComments(*d);
    m_ed->endChange();
}

void CommentsPane::visit(int direction)
{
    const Document *d = m_ed->doc();
    if (!d) return;
    const QVector<deck::CommentRef> all = deck::allComments(*d);
    const int n = int(all.size());
    if (!n) return;
    const int here = m_ed->currentPage();
    int target = -1;
    for (int i = 0; i < n && target < 0; ++i)
        if (all[i].slide == here && all[i].id == m_current) target = (i + (direction < 0 ? -1 : 1) + n) % n;
    if (target < 0 && direction > 0) {
        target = 0;
        for (int i = 0; i < n; ++i)
            if (all[i].slide >= here) {
                target = i;
                break;
            }
    } else if (target < 0) {
        target = n - 1;
        for (int i = n - 1; i >= 0; --i)
            if (all[i].slide < here) {
                target = i;
                break;
            }
    }
    const deck::CommentRef ref = all[target];
    if (ref.slide != here) m_ed->setCurrentPage(ref.slide);
    setCurrentComment(ref.id);
}

void CommentsPane::setCurrentComment(const QString &id)
{
    const Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    QString want = id;
    if (!want.isEmpty() && !(d && i >= 0 && i < d->pages.size() && deck::findComment(*d->pages[i], want))) want.clear();
    if (want == m_current) return;
    m_current = want;
    m_scrollToCurrent = !want.isEmpty();
    reload();
    Q_EMIT currentCommentChanged(m_current);
}

void CommentsPane::reload()
{
    if (m_rebuilding) return;
    const Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    QString pageId;
    QJsonArray comments;
    if (d && i >= 0 && i < d->pages.size()) {
        pageId = d->pages[i]->id;
        comments = d->pages[i]->props.value(QStringLiteral("comments")).toArray();
    }
    if (pageId != m_pageId) {
        // Another slide: what was open, being edited, or half typed belonged to the other.
        m_pageId = pageId;
        m_editing.clear();
        m_replyDrafts.clear();
        m_draft->clear();
        m_draftCard->hide();
        if (!m_current.isEmpty()) {
            m_current.clear();
            Q_EMIT currentCommentChanged(QString());
        }
    }
    QSet<QString> ids;
    for (const QJsonValue &v : comments) {
        const QJsonObject c = v.toObject();
        ids << c.value(QStringLiteral("id")).toString();
        for (const QJsonValue &r : c.value(QStringLiteral("replies")).toArray()) ids << r.toObject().value(QStringLiteral("id")).toString();
    }
    if (!m_current.isEmpty() && !ids.contains(m_current)) {
        m_current.clear();
        Q_EMIT currentCommentChanged(QString());
    }
    if (!m_editing.isEmpty() && !ids.contains(m_editing)) m_editing.clear();
    const QString signature = pageId + QLatin1Char('\n') + QString::fromUtf8(QJsonDocument(comments).toJson(QJsonDocument::Compact)) + QLatin1Char('\n') + m_current + QLatin1Char('\n') + m_editing;
    if (signature == m_signature) return;
    m_signature = signature;
    rebuild();
}

QLabel *CommentsPane::badge(const QString &author, const QString &initials, bool done, int size)
{
    auto *b = new QLabel(initials.left(3).toUpper(), m_cards);
    b->setObjectName(QStringLiteral("commentBadge"));
    b->setTextFormat(Qt::PlainText);
    b->setAlignment(Qt::AlignCenter);
    b->setFixedSize(size, size);
    b->setStyleSheet(QStringLiteral("QLabel{background:%1; color:white; border-radius:%2px; font-weight:bold; font-size:%3px;}")
                         .arg(commentColor(author, done).name()).arg(size / 2).arg(std::max(8, size * 2 / 5)));
    return b;
}

QFrame *CommentsPane::buildCard(const deck::Comment &c)
{
    auto *card = new QFrame(m_cards);
    card->setObjectName(QStringLiteral("commentCard"));
    card->setProperty(kIdProperty, c.id);
    card->installEventFilter(this);
    const bool current = c.id == m_current;
    card->setStyleSheet(QStringLiteral("QFrame#commentCard{background:%1; border:%2px solid %3; border-radius:8px;} QPushButton{padding:4px 8px; min-width:0px;}")
                            .arg(uiCard().name()).arg(current ? 2 : 1).arg((current ? uiAccent() : uiLine()).name()));
    auto *col = new QVBoxLayout(card);
    col->setContentsMargins(10, 8, 10, 8);
    col->setSpacing(6);

    // A line of text or, while it is being edited, the box it is edited in.
    auto body = [&](QBoxLayout *into, QWidget *parent, const QString &id, const QString &text, const char *labelName) {
        if (m_editing != id) {
            auto *label = new QLabel(text, parent);
            label->setObjectName(QLatin1String(labelName));
            label->setTextFormat(Qt::PlainText);
            label->setWordWrap(true);
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            if (c.done) label->setStyleSheet(QStringLiteral("color:%1;").arg(gray()));
            into->addWidget(label);
            return;
        }
        auto *editor = new QPlainTextEdit(text, parent);
        editor->setObjectName(QStringLiteral("commentEditor"));
        editor->setFixedHeight(76);
        into->addWidget(editor);
        auto *row = new QHBoxLayout;
        row->addStretch(1);
        auto *save = new QPushButton(tr("Save"), parent);
        save->setObjectName(QStringLiteral("commentSave"));
        save->setProperty("primary", true);
        auto *cancel = new QPushButton(tr("Cancel"), parent);
        cancel->setObjectName(QStringLiteral("commentCancel"));
        row->addWidget(save);
        row->addWidget(cancel);
        into->addLayout(row);
        save->setEnabled(!text.trimmed().isEmpty());
        connect(editor, &QPlainTextEdit::textChanged, save, [editor, save] { save->setEnabled(!editor->toPlainText().trimmed().isEmpty()); });
        connect(save, &QPushButton::clicked, this, [this, id, editor] { saveEdit(id, editor->toPlainText()); });
        connect(cancel, &QPushButton::clicked, this, [this] {
            m_editing.clear();
            reload();
        });
        QMetaObject::invokeMethod(editor, [editor] { editor->setFocus(); }, Qt::QueuedConnection);
    };

    auto *head = new QHBoxLayout;
    head->setSpacing(8);
    head->addWidget(badge(c.author, c.initials, c.done, 28), 0, Qt::AlignTop);
    auto *who = new QVBoxLayout;
    who->setSpacing(0);
    auto *author = new QLabel(c.author, card);
    author->setObjectName(QStringLiteral("commentAuthor"));
    author->setTextFormat(Qt::PlainText);
    QFont bold = author->font();
    bold.setBold(true);
    author->setFont(bold);
    author->setMinimumWidth(1);   // a long name is cut off, not made to widen the card
    auto *date = new QLabel(whenText(c.date), card);
    date->setMinimumWidth(1);
    date->setObjectName(QStringLiteral("commentDate"));
    date->setStyleSheet(QStringLiteral("color:%1;").arg(gray()));
    who->addWidget(author);
    who->addWidget(date);
    head->addLayout(who, 1);
    if (c.done) {
        auto *tag = new QLabel(tr("Resolved"), card);
        tag->setObjectName(QStringLiteral("commentResolved"));
        tag->setStyleSheet(QStringLiteral("color:%1;").arg(gray()));
        head->addWidget(tag, 0, Qt::AlignTop);
    }
    col->addLayout(head);
    body(col, card, c.id, c.text, "commentText");

    for (const deck::CommentReply &r : c.replies) {
        auto *box = new QWidget(card);
        box->setObjectName(QStringLiteral("commentReply"));
        auto *rc = new QVBoxLayout(box);
        rc->setContentsMargins(18, 0, 0, 0);
        rc->setSpacing(2);
        auto *rh = new QHBoxLayout;
        rh->setSpacing(6);
        rh->addWidget(badge(r.author, r.initials, c.done, 20), 0, Qt::AlignTop);
        auto *ra = new QLabel(r.author, box);
        ra->setObjectName(QStringLiteral("commentReplyAuthor"));
        ra->setTextFormat(Qt::PlainText);
        ra->setFont(bold);
        ra->setMinimumWidth(1);
        auto *rd = new QLabel(whenText(r.date), box);
        rd->setMinimumWidth(1);
        rd->setStyleSheet(QStringLiteral("color:%1;").arg(gray()));
        auto *rw = new QVBoxLayout;
        rw->setSpacing(0);
        rw->addWidget(ra);
        rw->addWidget(rd);
        rh->addLayout(rw, 1);
        auto small = [&](const char *name, const char *iconName, const QString &tip) {
            auto *b = new QToolButton(box);
            b->setObjectName(QLatin1String(name));
            b->setIcon(icon(QLatin1String(iconName)));
            b->setAutoRaise(true);
            b->setToolTip(tip);
            b->setAccessibleName(tip);
            rh->addWidget(b, 0, Qt::AlignTop);
            return b;
        };
        const QString rid = r.id, cid = c.id;
        connect(small("commentReplyChange", "pencil", tr("Edit Reply")), &QToolButton::clicked, this, [this, rid, cid] {
            m_current = cid;
            m_editing = rid;
            reload();
        });
        connect(small("commentReplyDelete", "trash-2", tr("Delete Reply")), &QToolButton::clicked, this, [this, rid, cid] {
            m_current = cid;
            deleteOne(rid);
        });
        rc->addLayout(rh);
        body(rc, box, r.id, r.text, "commentReplyText");
        col->addWidget(box);
    }

    // The box to reply in.
    auto *reply = new QHBoxLayout;
    auto *edit = new QLineEdit(m_replyDrafts.value(c.id), card);
    edit->setObjectName(QStringLiteral("commentReplyEdit"));
    edit->setPlaceholderText(tr("Reply"));
    edit->setAccessibleName(tr("Reply"));
    edit->setProperty(kIdProperty, c.id);
    auto *send = new QPushButton(tr("Reply"), card);
    send->setObjectName(QStringLiteral("commentReplyPost"));
    send->setEnabled(!edit->text().trimmed().isEmpty());
    reply->addWidget(edit, 1);
    reply->addWidget(send);
    col->addLayout(reply);
    const QString id = c.id;
    connect(edit, &QLineEdit::textChanged, send, [this, id, edit, send](const QString &t) {
        m_replyDrafts.insert(id, t);
        send->setEnabled(!t.trimmed().isEmpty());
    });
    connect(edit, &QLineEdit::returnPressed, this, [this, id, edit] { postReply(id, edit->text()); });
    connect(send, &QPushButton::clicked, this, [this, id, edit] { postReply(id, edit->text()); });

    // Resolve (or Reopen) in words; Edit and Delete as the replies have them, so the row has room in any language.
    auto *actions = new QHBoxLayout;
    auto *resolve = new QPushButton(c.done ? tr("Reopen") : tr("Resolve"), card);
    resolve->setObjectName(QStringLiteral("commentResolve"));
    auto iconButton = [&](const char *name, const char *iconName, const QString &tip) {
        auto *b = new QToolButton(card);
        b->setObjectName(QLatin1String(name));
        b->setIcon(icon(QLatin1String(iconName)));
        b->setAutoRaise(true);
        b->setToolTip(tip);
        b->setAccessibleName(tip);
        return b;
    };
    QToolButton *editButton = iconButton("commentEdit", "pencil", tr("Edit"));
    QToolButton *del = iconButton("commentDelete", "trash-2", tr("Delete"));
    actions->addWidget(resolve);
    actions->addStretch(1);
    actions->addWidget(editButton);
    actions->addWidget(del);
    col->addLayout(actions);
    const bool done = c.done;
    connect(resolve, &QPushButton::clicked, this, [this, id, done] { setResolved(id, !done); });
    connect(editButton, &QToolButton::clicked, this, [this, id] {
        m_current = id;
        m_editing = id;
        reload();
    });
    connect(del, &QToolButton::clicked, this, [this, id] { deleteOne(id); });
    return card;
}

void CommentsPane::rebuild()
{
    m_rebuilding = true;
    // The reply box that has the focus keeps it, and its place in the text.
    QString focusComment;
    int cursor = 0;
    if (auto *edit = qobject_cast<QLineEdit *>(m_cards->focusWidget()); edit && edit->hasFocus() && edit->objectName() == QLatin1String("commentReplyEdit")) {
        focusComment = edit->property(kIdProperty).toString();
        cursor = edit->cursorPosition();
    }
    // The old cards are set aside, not deleted: one of them may be what a click is being handled in.
    while (QLayoutItem *item = m_cardsLayout->takeAt(0)) {
        if (QWidget *w = item->widget()) {
            w->hide();
            w->setParent(nullptr);
            w->deleteLater();
        }
        delete item;
    }
    const Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    const QVector<deck::Comment> list = d && i >= 0 && i < d->pages.size() ? deck::commentsOf(*d->pages[i]) : QVector<deck::Comment>();
    QSet<QString> ids;
    QFrame *currentCard = nullptr;
    for (const deck::Comment &c : list) {
        QFrame *card = buildCard(c);
        m_cardsLayout->addWidget(card);
        ids << c.id;
        if (c.id == m_current) currentCard = card;
    }
    m_cardsLayout->addStretch(1);
    for (auto it = m_replyDrafts.begin(); it != m_replyDrafts.end();) it = ids.contains(it.key()) ? std::next(it) : m_replyDrafts.erase(it);
    m_empty->setVisible(list.isEmpty() && !m_draftCard->isVisible());
    if (!focusComment.isEmpty())
        for (QLineEdit *edit : m_cards->findChildren<QLineEdit *>(QStringLiteral("commentReplyEdit")))
            if (edit->property(kIdProperty).toString() == focusComment) {
                edit->setFocus();
                edit->setCursorPosition(cursor);
            }
    if (m_scrollToCurrent && currentCard) {
        QPointer<QFrame> card(currentCard);
        QPointer<CommentsPane> self(this);
        QMetaObject::invokeMethod(this, [self, card] {
            if (self && card) self->m_scroll->ensureWidgetVisible(card);
        }, Qt::QueuedConnection);
    }
    m_scrollToCurrent = false;
    m_rebuilding = false;
}

} // namespace jp
