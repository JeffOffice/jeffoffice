#include "app/reviewpane_deck.h"

#include "app/icons.h"
#include "app/plaintext.h"
#include "review/textdiff.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
#include <QStackedWidget>
#include <QTabBar>
#include <QTextBrowser>
#include <QToolButton>
#include <QVBoxLayout>

namespace jp {

namespace {

QString stateWord(review::State s)
{
    switch (s) {
    case review::State::Accepted: return QCoreApplication::translate("DeckReviewPane", "Accepted");
    case review::State::Rejected: return QCoreApplication::translate("DeckReviewPane", "Rejected");
    default: return QCoreApplication::translate("DeckReviewPane", "Pending");
    }
}

// The most characters of a text that the word-level view sets out (the rest is left off, and the view says so).
constexpr int kMostDetailText = 20000;

QString escaped(const QString &s) { return s.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>")); }

// The words of `before` that go and the words of `after` that come, set out in one run of text.
QString wordDiffHtml(const QString &before, const QString &after)
{
    const QString del = uiDanger().name(), ins = uiAccent().name();
    const QString a = before.left(kMostDetailText), b = after.left(kMostDetailText);
    const QVector<textdiff::TextEdit> edits = textdiff::diffText(a, b, textdiff::DiffOptions());
    QString html;
    int at = 0;
    for (const textdiff::TextEdit &e : edits) {
        const int from = std::clamp(e.deleteFrom, 0, int(a.size())), to = std::clamp(e.deleteTo, from, int(a.size()));
        if (from > at) html += escaped(a.mid(at, from - at));
        if (to > from) html += QStringLiteral("<span style=\"color:%1; text-decoration: line-through\">%2</span>").arg(del, escaped(a.mid(from, to - from)));
        if (!e.insertText.isEmpty()) html += QStringLiteral("<span style=\"color:%1; text-decoration: underline\">%2</span>").arg(ins, escaped(e.insertText));
        at = to;
    }
    if (at < a.size()) html += escaped(a.mid(at));
    if (before.size() > kMostDetailText || after.size() > kMostDetailText) html += QStringLiteral("<br>…");
    return html;
}

} // namespace

// ---- the rows ----

void DeckReviewModel::setList(const review::ChangeList *list)
{
    m_list = list;
    reload();
}

void DeckReviewModel::setFilter(const QString &container, const QString &author, int state)
{
    m_container = container;
    m_author = author;
    m_state = state;
    reload();
}

void DeckReviewModel::reload()
{
    beginResetModel();
    m_rows.clear();
    if (m_list) {
        const int n = std::min<int>(m_list->size(), review::kMaxChanges);
        m_rows.reserve(n);
        for (int i = 0; i < n; ++i) {
            const review::Change &c = m_list->changes[i];
            if ((m_container.isEmpty() || c.target.container == m_container) && (m_author.isEmpty() || c.author == m_author) && (m_state < 0 || int(c.state) == m_state)) m_rows << i;
        }
    }
    endResetModel();
}

QVariant DeckReviewModel::data(const QModelIndex &index, int role) const
{
    if (!m_list || !index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) return {};
    const int i = m_rows[index.row()];
    if (i < 0 || i >= m_list->size()) return {};
    const review::Change &c = m_list->changes[i];
    switch (role) {
    case Qt::DisplayRole:
        return c.state == review::State::Pending ? c.summary : QCoreApplication::translate("DeckReviewPane", "%1: %2").arg(stateWord(c.state), c.summary);
    case Qt::ToolTipRole: return plainToolTip(c.summary);
    case Qt::ForegroundRole:
        if (c.state != review::State::Pending) return uiMuted();
        return {};
    case ChangeIndexRole: return i;
    case StateRole: return int(c.state);
    default: return {};
    }
}

int DeckReviewModel::rowOfChange(int change) const { return int(m_rows.indexOf(change)); }

// ---- the pane ----

DeckReviewPane::DeckReviewPane(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("deckReviewPane"));
    setAccessibleName(tr("Reviewing Pane"));
    auto *column = new QVBoxLayout(this);
    column->setContentsMargins(8, 8, 8, 8);
    column->setSpacing(8);

    m_tabs = new QTabBar(this);
    m_tabs->setObjectName(QStringLiteral("reviewTabs"));
    m_tabs->addTab(tr("Changes"));
    m_tabs->addTab(tr("Comments"));
    m_tabs->setExpanding(false);
    column->addWidget(m_tabs);
    m_pages = new QStackedWidget(this);
    column->addWidget(m_pages, 1);

    // Page 1: the changes.
    auto *changes = new QWidget(m_pages);
    auto *cl = new QVBoxLayout(changes);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(8);
    m_status = plainLabel(new QLabel(changes));
    m_status->setObjectName(QStringLiteral("reviewStatus"));
    m_status->setWordWrap(true);
    cl->addWidget(m_status);
    auto *filters = new QHBoxLayout();
    filters->setSpacing(8);
    m_slide = new QComboBox(changes);
    m_slide->setObjectName(QStringLiteral("reviewSlideFilter"));
    m_slide->setAccessibleName(tr("Show changes on slide"));
    m_author = new QComboBox(changes);
    m_author->setObjectName(QStringLiteral("reviewAuthorFilter"));
    m_author->setAccessibleName(tr("Show changes by"));
    m_state = new QComboBox(changes);
    m_state->setObjectName(QStringLiteral("reviewStateFilter"));
    m_state->setAccessibleName(tr("Show changes that are"));
    m_state->addItem(tr("Pending"), int(review::State::Pending));
    m_state->addItem(tr("All"), -1);
    m_state->addItem(tr("Accepted"), int(review::State::Accepted));
    m_state->addItem(tr("Rejected"), int(review::State::Rejected));
    for (QComboBox *b : {m_slide, m_author, m_state}) {
        b->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        b->setMinimumContentsLength(8);
        connect(b, &QComboBox::currentIndexChanged, this, [this] { filtersChanged(); });
    }
    cl->addWidget(m_slide);
    filters->addWidget(m_author, 1);
    filters->addWidget(m_state, 1);
    cl->addLayout(filters);
    m_model = new DeckReviewModel(this);
    m_list = new QListView(changes);
    m_list->setObjectName(QStringLiteral("reviewList"));
    m_list->setAccessibleName(tr("Changes"));
    m_list->setModel(m_model);
    m_list->setUniformItemSizes(true);   // (rows are made as the view asks: ten thousand cost no more than the ones on screen)
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_list->setTextElideMode(Qt::ElideRight);
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    cl->addWidget(m_list, 3);
    m_detail = new QTextBrowser(changes);
    m_detail->setObjectName(QStringLiteral("reviewDetail"));
    m_detail->setAccessibleName(tr("The change"));
    m_detail->setMaximumHeight(140);
    cl->addWidget(m_detail, 1);

    auto button = [&](const QString &name, const QString &text, const QString &tip) {
        auto *b = new QPushButton(text, changes);
        b->setObjectName(name);
        b->setToolTip(plainToolTip(tip));
        return b;
    };
    m_accept = button(QStringLiteral("reviewAccept"), tr("Accept"), tr("Take this change into the presentation"));
    m_reject = button(QStringLiteral("reviewReject"), tr("Reject"), tr("Keep this presentation as it is here"));
    m_prev = button(QStringLiteral("reviewPrev"), tr("Previous"), tr("Go to the change before this one"));
    m_next = button(QStringLiteral("reviewNext"), tr("Next"), tr("Go to the change after this one"));
    m_acceptAll = button(QStringLiteral("reviewAcceptAll"), tr("Accept All"), tr("Take every change that is still pending"));
    m_rejectAll = button(QStringLiteral("reviewRejectAll"), tr("Reject All"), tr("Keep this presentation as it is everywhere"));
    auto *row1 = new QHBoxLayout();
    row1->setSpacing(8);
    row1->addWidget(m_prev);
    row1->addWidget(m_next);
    row1->addStretch(1);
    row1->addWidget(m_reject);
    row1->addWidget(m_accept);
    auto *row2 = new QHBoxLayout();
    row2->setSpacing(8);
    row2->addStretch(1);
    row2->addWidget(m_rejectAll);
    row2->addWidget(m_acceptAll);
    cl->addLayout(row1);
    cl->addLayout(row2);
    m_pages->addWidget(changes);

    // Page 2: the comments.
    m_comments = new QListWidget(m_pages);
    m_comments->setObjectName(QStringLiteral("reviewComments"));
    m_comments->setAccessibleName(tr("Comments"));
    m_comments->setWordWrap(true);
    m_pages->addWidget(m_comments);

    connect(m_tabs, &QTabBar::currentChanged, m_pages, &QStackedWidget::setCurrentIndex);
    connect(m_tabs, &QTabBar::currentChanged, this, &DeckReviewPane::pageChanged);
    connect(m_list->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &) {
        showDetail();
        updateButtons();
        Q_EMIT changeChosen(currentChange());
    });
    connect(m_accept, &QPushButton::clicked, this, [this] {
        if (currentChange() >= 0) Q_EMIT acceptRequested(currentChange());
    });
    connect(m_reject, &QPushButton::clicked, this, [this] {
        if (currentChange() >= 0) Q_EMIT rejectRequested(currentChange());
    });
    connect(m_prev, &QPushButton::clicked, this, [this] { step(false); });
    connect(m_next, &QPushButton::clicked, this, [this] { step(true); });
    connect(m_acceptAll, &QPushButton::clicked, this, &DeckReviewPane::acceptAllRequested);
    connect(m_rejectAll, &QPushButton::clicked, this, &DeckReviewPane::rejectAllRequested);
    connect(m_comments, &QListWidget::currentRowChanged, this, [this](int row) {
        QListWidgetItem *it = m_comments->item(row);
        if (it) Q_EMIT commentChosen(it->data(Qt::UserRole).toString(), it->data(Qt::UserRole + 1).toInt());
    });
    setMinimumWidth(280);
    refresh();
}

void DeckReviewPane::setSource(const Source &source)
{
    m_src = source;
    m_model->setList(m_src.list);
    refresh();
}

int DeckReviewPane::page() const { return m_tabs->currentIndex(); }
void DeckReviewPane::setPage(int page) { m_tabs->setCurrentIndex(std::clamp(page, 0, 1)); }

void DeckReviewPane::setComments(const QVector<CommentRow> &rows)
{
    m_comments->clear();
    const int n = std::min<int>(rows.size(), 2000);
    for (int i = 0; i < n; ++i) {
        const CommentRow &r = rows[i];
        auto *it = new QListWidgetItem(tr("Slide %1, %2: %3").arg(r.slide + 1).arg(r.author, r.text.left(200)), m_comments);
        it->setData(Qt::UserRole, r.id);
        it->setData(Qt::UserRole + 1, r.slide);
    }
}

void DeckReviewPane::rebuildFilters()
{
    m_rebuilding = true;
    const QString keepSlide = m_slide->currentData().toString(), keepAuthor = m_author->currentData().toString();
    m_slide->clear();
    m_author->clear();
    m_slide->addItem(tr("All slides"), QString());
    m_author->addItem(tr("All authors"), QString());
    if (m_src.list) {
        QSet<QString> slides, authors;
        const int n = std::min<int>(m_src.list->size(), review::kMaxChanges);
        for (int i = 0; i < n; ++i) {
            const review::Change &c = m_src.list->changes[i];
            if (!c.target.container.isEmpty() && !slides.contains(c.target.container) && slides.size() < 2000) {
                slides.insert(c.target.container);
                m_slide->addItem(m_src.slideLabel ? m_src.slideLabel(c.target.container) : c.target.container, c.target.container);
            }
            if (!c.author.isEmpty() && !authors.contains(c.author) && authors.size() < 500) {
                authors.insert(c.author);
                m_author->addItem(c.author, c.author);
            }
        }
    }
    const int s = m_slide->findData(keepSlide), a = m_author->findData(keepAuthor);
    m_slide->setCurrentIndex(s >= 0 ? s : 0);
    m_author->setCurrentIndex(a >= 0 ? a : 0);
    m_rebuilding = false;
}

void DeckReviewPane::filtersChanged()
{
    if (m_rebuilding) return;
    const int keep = currentChange();
    m_model->setFilter(m_slide->currentData().toString(), m_author->currentData().toString(), m_state->currentData().toInt());
    const int row = m_model->rowOfChange(keep);
    if (row >= 0) m_list->setCurrentIndex(m_model->index(row));
    showDetail();
    updateButtons();
}

void DeckReviewPane::refresh()
{
    const int keep = currentChange();
    const int oldRow = m_list->currentIndex().row();
    rebuildFilters();
    m_model->setFilter(m_slide->currentData().toString(), m_author->currentData().toString(), m_state->currentData().toInt());
    int row = m_model->rowOfChange(keep);
    if (row < 0 && m_model->rowCount() > 0 && keep >= 0) row = std::clamp(oldRow, 0, m_model->rowCount() - 1);   // the chosen one is not shown now (it was decided): the one that took its place
    {
        const QSignalBlocker block(m_list->selectionModel());
        m_list->setCurrentIndex(row >= 0 ? m_model->index(row) : QModelIndex());
    }
    const review::ChangeList *l = m_src.list;
    if (!l) m_status->setText(tr("No presentation is being compared."));
    else if (l->isEmpty()) m_status->setText(tr("The two presentations are the same."));
    else m_status->setText(tr("%1 of %2 changes are pending.").arg(l->pending()).arg(l->size()) + (l->tooMany ? QLatin1Char(' ') + tr("There are more changes than are listed here.") : QString()));
    showDetail();
    updateButtons();
}

int DeckReviewPane::currentChange() const
{
    const QModelIndex i = m_list->currentIndex();
    return i.isValid() ? m_model->changeOfRow(i.row()) : -1;
}

void DeckReviewPane::setCurrentChange(int change)
{
    const int row = m_model->rowOfChange(change);
    if (row < 0) return;
    m_list->setCurrentIndex(m_model->index(row));
    m_list->scrollTo(m_model->index(row));
}

int DeckReviewPane::visibleCount() const { return m_model->rowCount(); }

void DeckReviewPane::step(bool forward)
{
    const int n = m_model->rowCount();
    if (n == 0) return;
    const int at = m_list->currentIndex().isValid() ? m_list->currentIndex().row() : (forward ? -1 : n);
    int pick = -1;
    for (int k = 1; k <= n && pick < 0; ++k) {
        const int row = forward ? ((at + k) % n + n) % n : ((at - k) % n + n) % n;
        const int c = m_model->changeOfRow(row);
        if (m_src.list && c >= 0 && m_src.list->changes[c].state == review::State::Pending) pick = row;
    }
    if (pick < 0) pick = forward ? (at + 1) % n : ((at - 1) % n + n) % n;
    m_list->setCurrentIndex(m_model->index(pick));
    m_list->scrollTo(m_model->index(pick));
}

void DeckReviewPane::showDetail()
{
    const int c = currentChange();
    if (!m_src.list || c < 0 || c >= m_src.list->size()) {
        m_detail->clear();
        return;
    }
    const review::Change &ch = m_src.list->changes[c];
    QString html = QStringLiteral("<b>%1</b>").arg(escaped(ch.summary));
    QString before, after;
    if (m_src.texts && m_src.texts(c, &before, &after)) html += QStringLiteral("<p>") + wordDiffHtml(before, after) + QStringLiteral("</p>");
    if (!ch.author.isEmpty()) html += QStringLiteral("<p>%1</p>").arg(escaped(tr("By %1").arg(ch.author)));
    html += QStringLiteral("<p>%1</p>").arg(escaped(stateWord(ch.state)));
    m_detail->setHtml(html);
}

void DeckReviewPane::updateButtons()
{
    const int c = currentChange();
    const bool one = c >= 0 && m_src.list && c < m_src.list->size();
    m_accept->setEnabled(one && m_src.list->changes[c].state != review::State::Accepted);
    m_reject->setEnabled(one && m_src.list->changes[c].state != review::State::Rejected);
    m_prev->setEnabled(m_model->rowCount() > 0);
    m_next->setEnabled(m_model->rowCount() > 0);
    const bool any = m_src.list && m_src.list->pending() > 0;
    m_acceptAll->setEnabled(any);
    m_rejectAll->setEnabled(any);
}

} // namespace jp
