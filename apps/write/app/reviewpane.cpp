#include "app/reviewpane.h"

#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/editops.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTextDocument>
#include <QTreeWidget>
#include <QVBoxLayout>
#include "app/plaintext.h"
#include <algorithm>

namespace jp {

namespace {

constexpr int kMostListed = 2000;   // (the changes and comments the lists make rows for; a document with more has the rest reached by Next and Previous)

QString kindName(const write::Revision &r)
{
    switch (r.kind) {
    case write::Revision::Insertion: return QCoreApplication::translate("jp::ReviewPane", "Inserted");
    case write::Revision::Deletion: return QCoreApplication::translate("jp::ReviewPane", "Deleted");
    case write::Revision::MovedFrom: return QCoreApplication::translate("jp::ReviewPane", "Moved from");
    case write::Revision::MovedTo: return QCoreApplication::translate("jp::ReviewPane", "Moved to");
    case write::Revision::Format: return QCoreApplication::translate("jp::ReviewPane", "Formatted");
    case write::Revision::ParagraphFormat: return QCoreApplication::translate("jp::ReviewPane", "Formatted paragraph");
    case write::Revision::MarkInserted: return QCoreApplication::translate("jp::ReviewPane", "Inserted paragraph");
    case write::Revision::MarkDeleted: return QCoreApplication::translate("jp::ReviewPane", "Deleted paragraph");
    }
    return QString();
}

QString dateText(const QDateTime &d) { return d.isValid() ? QLocale().toString(d.toLocalTime(), QLocale::ShortFormat) : QString(); }

} // namespace

ReviewPane::ReviewPane(WriteWindow *win, QWidget *parent) : QWidget(parent), m_win(win)
{
    setObjectName(QStringLiteral("reviewPane"));
    auto *v = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName(QStringLiteral("reviewTabs"));
    v->addWidget(m_tabs, 1);

    // Comments.
    auto *cp = new QWidget(m_tabs);
    auto *cv = new QVBoxLayout(cp);
    cv->setContentsMargins(4, 4, 4, 4);
    m_comments = new QTreeWidget(cp);
    m_comments->setObjectName(QStringLiteral("commentList"));
    m_comments->setHeaderHidden(true);
    m_comments->setRootIsDecorated(true);
    m_comments->setWordWrap(true);
    m_comments->setUniformRowHeights(false);
    m_comments->setAccessibleName(tr("Comments"));
    cv->addWidget(m_comments, 1);
    m_reply = new QLineEdit(cp);
    m_reply->setObjectName(QStringLiteral("replyField"));
    m_reply->setPlaceholderText(tr("Reply"));
    m_reply->setAccessibleName(tr("Reply"));
    cv->addWidget(m_reply);
    auto *r1 = new QHBoxLayout;
    m_post = new QPushButton(tr("Post Reply"), cp);
    m_post->setObjectName(QStringLiteral("postReplyButton"));
    m_resolve = new QPushButton(tr("Resolve"), cp);
    m_resolve->setObjectName(QStringLiteral("resolveButton"));
    m_delete = new QPushButton(tr("Delete"), cp);
    m_delete->setObjectName(QStringLiteral("deleteCommentButton"));
    r1->addWidget(m_post);
    r1->addWidget(m_resolve);
    r1->addWidget(m_delete);
    cv->addLayout(r1);
    auto *r2 = new QHBoxLayout;
    m_new = new QPushButton(tr("New Comment"), cp);
    m_new->setObjectName(QStringLiteral("newCommentButton"));
    m_prevComment = new QPushButton(tr("Previous"), cp);
    m_prevComment->setObjectName(QStringLiteral("previousCommentButton"));
    m_nextComment = new QPushButton(tr("Next"), cp);
    m_nextComment->setObjectName(QStringLiteral("nextCommentButton"));
    r2->addWidget(m_new);
    r2->addWidget(m_prevComment);
    r2->addWidget(m_nextComment);
    cv->addLayout(r2);
    m_tabs->addTab(cp, tr("Comments"));

    // Changes.
    auto *hp = new QWidget(m_tabs);
    auto *hv = new QVBoxLayout(hp);
    hv->setContentsMargins(4, 4, 4, 4);
    m_person = new QComboBox(hp);
    m_person->setObjectName(QStringLiteral("personBox"));
    m_person->setAccessibleName(tr("Reviewer"));
    hv->addWidget(m_person);
    m_changes = new QTreeWidget(hp);
    m_changes->setObjectName(QStringLiteral("changeList"));
    m_changes->setColumnCount(3);
    m_changes->setHeaderLabels({tr("Change"), tr("Reviewer"), tr("Date")});
    m_changes->setRootIsDecorated(false);
    m_changes->setAccessibleName(tr("Changes"));
    hv->addWidget(m_changes, 1);
    auto *h1 = new QHBoxLayout;
    m_accept = new QPushButton(tr("Accept"), hp);
    m_accept->setObjectName(QStringLiteral("acceptButton"));
    m_reject = new QPushButton(tr("Reject"), hp);
    m_reject->setObjectName(QStringLiteral("rejectButton"));
    m_prevChange = new QPushButton(tr("Previous"), hp);
    m_prevChange->setObjectName(QStringLiteral("previousChangeButton"));
    m_nextChange = new QPushButton(tr("Next"), hp);
    m_nextChange->setObjectName(QStringLiteral("nextChangeButton"));
    h1->addWidget(m_accept);
    h1->addWidget(m_reject);
    h1->addWidget(m_prevChange);
    h1->addWidget(m_nextChange);
    hv->addLayout(h1);
    auto *h2 = new QHBoxLayout;
    m_acceptAll = new QPushButton(tr("Accept All"), hp);
    m_acceptAll->setObjectName(QStringLiteral("acceptAllButton"));
    m_rejectAll = new QPushButton(tr("Reject All"), hp);
    m_rejectAll->setObjectName(QStringLiteral("rejectAllButton"));
    h2->addWidget(m_acceptAll);
    h2->addWidget(m_rejectAll);
    hv->addLayout(h2);
    m_tabs->addTab(hp, tr("Changes"));

    m_idle.setSingleShot(true);
    m_idle.setInterval(150);
    connect(&m_idle, &QTimer::timeout, this, &ReviewPane::refresh);
    connect(m_comments, &QTreeWidget::itemSelectionChanged, this, [this] {
        updateButtons();
        showChosenComment();
    });
    connect(m_comments, &QTreeWidget::itemDoubleClicked, this, [this] { editChosen(); });
    connect(m_changes, &QTreeWidget::itemSelectionChanged, this, [this] {
        updateButtons();
        showChosenChange();
    });
    connect(m_person, &QComboBox::currentIndexChanged, this, [this] { refresh(); });
    connect(m_reply, &QLineEdit::returnPressed, this, [this] { postReply(); });
    connect(m_post, &QPushButton::clicked, this, [this] { postReply(); });
    connect(m_resolve, &QPushButton::clicked, this, [this] { toggleResolved(); });
    connect(m_delete, &QPushButton::clicked, this, [this] { deleteChosen(); });
    connect(m_new, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("review.newComment"))->trigger(); });
    connect(m_prevComment, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("review.previousComment"))->trigger(); });
    connect(m_nextComment, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("review.nextComment"))->trigger(); });
    connect(m_accept, &QPushButton::clicked, this, [this] { resolveChange(true); });
    connect(m_reject, &QPushButton::clicked, this, [this] { resolveChange(false); });
    connect(m_prevChange, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("review.previousChange"))->trigger(); });
    connect(m_nextChange, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("review.nextChange"))->trigger(); });
    connect(m_acceptAll, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("review.acceptAll"))->trigger(); });
    connect(m_rejectAll, &QPushButton::clicked, this, [this] { m_win->act(QStringLiteral("review.rejectAll"))->trigger(); });
    updateButtons();
}

void ReviewPane::showPage(Page page) { m_tabs->setCurrentIndex(page == Comments ? 0 : 1); }

ReviewPane::Page ReviewPane::page() const { return m_tabs->currentIndex() == 0 ? Comments : Changes; }

QString ReviewPane::chosenPerson() const { return m_person->currentData().toString(); }

int ReviewPane::chosenComment() const
{
    const QList<QTreeWidgetItem *> sel = m_comments->selectedItems();
    return sel.isEmpty() ? -1 : sel.first()->data(0, Qt::UserRole).toInt();
}

void ReviewPane::refresh()
{
    write::WriteDoc *doc = m_win->document();
    if (!doc) {
        m_comments->clear();
        m_changes->clear();
        m_madeDoc = nullptr;
        return;
    }
    // Made again only when the text (or who reviews, or the way changes are shown) is not what the lists were made from.
    const int revision = doc->story()->revision();
    if (m_madeDoc == doc && m_madeRevision == revision && m_madePerson == chosenPerson() && m_madeView == doc->revisionView()) return;
    rebuild();
}

void ReviewPane::rebuild()
{
    write::WriteDoc *doc = m_win->document();
    if (!doc || m_building) return;
    m_building = true;
    ++m_rebuilds;
    m_madeDoc = doc;
    m_madeRevision = doc->story()->revision();
    m_madeView = doc->revisionView();
    m_madePerson = chosenPerson();
    const int chosen = chosenComment();
    // The people.
    {
        const QSignalBlocker block(m_person);
        const QString was = m_person->currentData().toString();
        m_person->clear();
        m_person->addItem(tr("All Reviewers"), QString());
        for (const QString &p : write::revisionAuthors(doc->story())) m_person->addItem(p, p);
        const int at = m_person->findData(was);
        m_person->setCurrentIndex(std::max(0, at));
    }
    const QString person = chosenPerson();
    m_madePerson = person;
    // The changes.
    m_changes->clear();
    int listed = 0;
    for (const write::Revision &r : write::revisionsIn(doc->story(), 0, -1, kMostListed + 1)) {
        if (!person.isEmpty() && r.author != person) continue;
        if (++listed > kMostListed) break;
        auto *item = new QTreeWidgetItem(m_changes);
        const QString words = write::revisionText(doc->story(), r, 60);
        item->setText(0, words.isEmpty() ? kindName(r) : kindName(r) + QStringLiteral(": ") + words);
        item->setText(1, r.author);
        item->setText(2, dateText(r.date));
        item->setData(0, Qt::UserRole, r.from);
        item->setData(0, Qt::UserRole + 1, r.to);
        item->setData(0, Qt::UserRole + 2, int(r.kind));
        item->setData(0, Qt::UserRole + 3, r.id);
    }
    // The comments, replies under the comment they answer.
    m_comments->clear();
    const QVector<write::Comment> all = write::comments(*doc);
    QHash<int, QTreeWidgetItem *> items;
    auto make = [&](const write::Comment &c, QTreeWidgetItem *parent) {
        auto *item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(m_comments);
        QString head = c.author;
        if (c.date.isValid()) head += QStringLiteral("  ") + dateText(c.date);
        if (c.done) head += QStringLiteral("  (") + tr("Resolved") + QLatin1Char(')');
        item->setText(0, head + QLatin1Char('\n') + c.text);
        item->setData(0, Qt::UserRole, c.id);
        item->setData(0, Qt::UserRole + 1, c.from);
        item->setData(0, Qt::UserRole + 2, c.to);
        item->setData(0, Qt::UserRole + 3, c.done);
        item->setToolTip(0, jp::plainToolTip(c.text));
        if (c.done) {
            QFont f = item->font(0);
            f.setItalic(true);
            item->setFont(0, f);
            item->setForeground(0, palette().color(QPalette::Disabled, QPalette::Text));
        }
        items.insert(c.id, item);
        return item;
    };
    int made = 0;
    for (const write::Comment &c : all)
        if (c.parent < 0 && made++ < kMostListed) make(c, nullptr);
    for (int round = 0; round < 64; ++round) {
        bool grew = false;
        for (const write::Comment &c : all)
            if (c.parent >= 0 && !items.contains(c.id) && items.contains(c.parent) && made++ < kMostListed * 2) {
                make(c, items.value(c.parent));
                grew = true;
            }
        if (!grew) break;
    }
    m_comments->expandAll();
    if (chosen >= 0 && items.contains(chosen)) {
        const QSignalBlocker block(m_comments);
        items.value(chosen)->setSelected(true);
    }
    m_building = false;
    updateButtons();
}

void ReviewPane::updateButtons()
{
    write::WriteDoc *doc = m_win->document();
    const QList<QTreeWidgetItem *> sel = m_comments->selectedItems();
    const bool one = !sel.isEmpty();
    m_post->setEnabled(doc && one);
    m_reply->setEnabled(doc && one);
    m_resolve->setEnabled(doc && one);
    m_delete->setEnabled(doc && one);
    m_resolve->setText(one && sel.first()->data(0, Qt::UserRole + 3).toBool() ? tr("Reopen") : tr("Resolve"));
    const bool change = !m_changes->selectedItems().isEmpty();
    m_accept->setEnabled(doc && change);
    m_reject->setEnabled(doc && change);
    m_acceptAll->setEnabled(doc && m_changes->topLevelItemCount() > 0);
    m_rejectAll->setEnabled(doc && m_changes->topLevelItemCount() > 0);
}

void ReviewPane::showChosenChange()
{
    write::WriteDoc *doc = m_win->document();
    const QList<QTreeWidgetItem *> sel = m_changes->selectedItems();
    if (!doc || sel.isEmpty() || m_building) return;
    m_win->view()->leaveHeaderFooter();
    m_win->view()->leaveNote();
    QTextCursor c(doc->story());
    const int from = sel.first()->data(0, Qt::UserRole).toInt(), to = sel.first()->data(0, Qt::UserRole + 1).toInt();
    c.setPosition(std::min(from, doc->story()->characterCount() - 1));
    c.setPosition(std::min(std::max(from, to), doc->story()->characterCount() - 1), QTextCursor::KeepAnchor);
    m_win->view()->showCursor(c);
}

void ReviewPane::showChosenComment()
{
    write::WriteDoc *doc = m_win->document();
    if (!doc || m_building) return;
    const int id = chosenComment();
    if (id < 0) return;
    const std::optional<write::Comment> c = write::commentById(*doc, id);
    if (!c || c->from < 0) return;
    m_win->view()->leaveHeaderFooter();
    m_win->view()->leaveNote();
    QTextCursor t(doc->story());
    const int end = doc->story()->characterCount() - 1;
    t.setPosition(std::min(c->from, end));
    t.setPosition(std::min(std::max(c->from, c->to + 1), end), QTextCursor::KeepAnchor);
    m_win->view()->showCursor(t);
}

bool ReviewPane::postReply()
{
    write::WriteDoc *doc = m_win->document();
    const int id = chosenComment();
    const QString text = m_reply->text().trimmed();
    if (!doc || id < 0 || text.isEmpty()) return false;
    // A reply goes to the comment the thread begins with.
    const std::optional<write::Comment> c = write::commentById(*doc, id);
    const int root = c && c->parent >= 0 ? c->parent : id;
    const int made = write::replyToComment(*doc, root, text);
    if (made < 0) return false;
    m_reply->clear();
    refresh();
    return true;
}

void ReviewPane::toggleResolved()
{
    write::WriteDoc *doc = m_win->document();
    const int id = chosenComment();
    if (!doc || id < 0) return;
    const std::optional<write::Comment> c = write::commentById(*doc, id);
    if (!c) return;
    const int root = c->parent >= 0 ? c->parent : id;
    write::setCommentDone(*doc, root, !c->done);
    refresh();
}

void ReviewPane::deleteChosen()
{
    write::WriteDoc *doc = m_win->document();
    const int id = chosenComment();
    if (!doc || id < 0) return;
    write::deleteComment(*doc, id);
    refresh();
}

void ReviewPane::editChosen()
{
    write::WriteDoc *doc = m_win->document();
    const int id = chosenComment();
    if (!doc || id < 0) return;
    const std::optional<write::Comment> c = write::commentById(*doc, id);
    if (!c) return;
    bool ok = false;
    const QString text = QInputDialog::getMultiLineText(this, tr("Edit Comment"), tr("Comment:"), c->text, &ok);
    if (!ok || text == c->text) return;
    write::setCommentText(*doc, id, text);
    refresh();
}

void ReviewPane::resolveChange(bool accept)
{
    write::WriteDoc *doc = m_win->document();
    const QList<QTreeWidgetItem *> sel = m_changes->selectedItems();
    if (!doc || sel.isEmpty()) return;
    const int at = sel.first()->data(0, Qt::UserRole).toInt();
    QTextCursor c(doc->story());
    c.setPosition(std::min(at + 1, doc->story()->characterCount() - 1));
    if (const std::optional<write::Revision> r = write::revisionAt(doc->story(), c.position()))
        write::resolveRevisions(*doc, doc->story(), {*r}, accept);
    refresh();
}

} // namespace jp
