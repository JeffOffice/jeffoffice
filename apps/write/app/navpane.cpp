#include "app/navpane.h"

#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/editops.h"
#include "core/gotoops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"

#include <QCoreApplication>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPixmap>
#include <QTabWidget>
#include <QTextBlock>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

namespace {

constexpr int kPositionRole = Qt::UserRole, kLevelRole = Qt::UserRole + 1, kLengthRole = Qt::UserRole + 2;
constexpr int kMostThumbnails = 60, kThumbnailWidth = 96;


} // namespace

NavigationPane::NavigationPane(WriteWindow *win, QWidget *parent) : QWidget(parent), m_win(win)
{
    setObjectName(QStringLiteral("navigationPane"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName(QStringLiteral("navigationTabs"));
    v->addWidget(m_tabs);

    m_headings = new QTreeWidget(this);
    m_headings->setObjectName(QStringLiteral("navigationHeadings"));
    m_headings->setHeaderHidden(true);
    m_headings->setAccessibleName(tr("Headings"));
    m_headings->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tabs->addTab(m_headings, tr("Headings"));
    connect(m_headings, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item) { goToHeading(item); });
    connect(m_headings, &QWidget::customContextMenuRequested, this, [this](const QPoint &at) {
        QTreeWidgetItem *item = m_headings->itemAt(at);
        QMenu menu(this);
        if (item) {
            connect(menu.addAction(tr("Promote")), &QAction::triggered, this, [this, item] { moveHeadingLevel(item, -1); });
            connect(menu.addAction(tr("Demote")), &QAction::triggered, this, [this, item] { moveHeadingLevel(item, 1); });
            menu.addSeparator();
        }
        connect(menu.addAction(tr("Expand All")), &QAction::triggered, m_headings, &QTreeWidget::expandAll);
        connect(menu.addAction(tr("Collapse All")), &QAction::triggered, m_headings, &QTreeWidget::collapseAll);
        menu.exec(m_headings->viewport()->mapToGlobal(at));
    });

    m_pages = new QListWidget(this);
    m_pages->setObjectName(QStringLiteral("navigationPages"));
    m_pages->setAccessibleName(tr("Pages"));
    m_pages->setViewMode(QListView::IconMode);
    m_pages->setResizeMode(QListView::Adjust);
    m_pages->setMovement(QListView::Static);
    m_pages->setIconSize(QSize(kThumbnailWidth, kThumbnailWidth * 4 / 3));
    m_tabs->addTab(m_pages, tr("Pages"));
    connect(m_pages, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) { m_win->view()->goToPage(item->data(kPositionRole).toInt()); });

    auto *resultsPage = new QWidget(this);
    auto *rv = new QVBoxLayout(resultsPage);
    m_search = new QLineEdit(resultsPage);
    m_search->setObjectName(QStringLiteral("navigationSearch"));
    m_search->setPlaceholderText(tr("Search document"));
    m_search->setClearButtonEnabled(true);
    m_results = new QListWidget(resultsPage);
    m_results->setObjectName(QStringLiteral("navigationResults"));
    m_results->setAccessibleName(tr("Results"));
    rv->addWidget(m_search);
    rv->addWidget(m_results, 1);
    m_tabs->addTab(resultsPage, tr("Results"));
    connect(m_search, &QLineEdit::returnPressed, this, &NavigationPane::search);
    connect(m_results, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (!m_win->document()) return;
        QTextCursor c(m_win->document()->story());
        const int at = item->data(kPositionRole).toInt(), length = item->data(kLengthRole).toInt();
        c.setPosition(std::clamp(at, 0, write::editableEnd(m_win->document()->story())));
        c.setPosition(std::clamp(at + length, 0, write::editableEnd(m_win->document()->story())), QTextCursor::KeepAnchor);
        m_win->view()->showCursor(c);
        m_win->view()->setFocus(Qt::OtherFocusReason);
    });
    connect(m_tabs, &QTabWidget::currentChanged, this, [this] { refresh(); });
}

NavigationPane::~NavigationPane()
{
    m_dying = true;   // (the window has let go of its document by the time the panes it owns are taken down)
}

void NavigationPane::refresh()
{
    if (m_dying) return;
    if (!m_win->document()) {
        m_headings->clear();
        m_pages->clear();
        m_results->clear();
        m_headingsOf = m_pagesOf = 0;
        return;
    }
    const int index = m_tabs->currentIndex();
    if (index == 0) {
        rebuildHeadings();
        markCaretHeading();
    } else if (index == 1) {
        rebuildPages();
    }
}

void NavigationPane::rebuildHeadings()
{
    write::WriteDoc *doc = m_win->document();
    if (m_headingsOf == doc->id() && m_revision == doc->story()->revision()) return;
    m_headingsOf = doc->id();
    m_revision = doc->story()->revision();
    // (What was collapsed stays collapsed: the headings are found again by their text and level.)
    QSet<QString> collapsed;
    for (QTreeWidgetItemIterator it(m_headings); *it; ++it)
        if (!(*it)->isExpanded() && (*it)->childCount() > 0) collapsed.insert((*it)->text(0) + QLatin1Char('\t') + QString::number((*it)->data(0, kLevelRole).toInt()));
    m_headings->setUpdatesEnabled(false);
    m_headings->clear();
    QVector<QTreeWidgetItem *> open;   // the last heading of each level
    for (const write::GoToItem &h : write::goToItems(*doc, write::GoToKind::Heading)) {
        const int level = std::clamp(doc->story()->findBlock(h.position).blockFormat().intProperty(write::prop::Outline), 0, 8);
        while (!open.isEmpty() && open.last()->data(0, kLevelRole).toInt() >= level) open.removeLast();
        auto *item = open.isEmpty() ? new QTreeWidgetItem(m_headings) : new QTreeWidgetItem(open.last());
        item->setText(0, h.label);
        item->setData(0, kPositionRole, h.position);
        item->setData(0, kLevelRole, level);
        open.push_back(item);
    }
    m_headings->expandAll();
    for (QTreeWidgetItemIterator it(m_headings); *it; ++it)
        if (collapsed.contains((*it)->text(0) + QLatin1Char('\t') + QString::number((*it)->data(0, kLevelRole).toInt()))) (*it)->setExpanded(false);
    m_headings->setUpdatesEnabled(true);
}

void NavigationPane::markCaretHeading()
{
    // The heading the caret is under: the last one at or before it.
    const int caret = m_win->view()->cursor().position();
    QTreeWidgetItem *current = nullptr;
    for (QTreeWidgetItemIterator it(m_headings); *it; ++it)
        if ((*it)->data(0, kPositionRole).toInt() <= caret) current = *it;
    QSignalBlocker block(m_headings);
    m_headings->setCurrentItem(current);
}

void NavigationPane::goToHeading(QTreeWidgetItem *item)
{
    if (!item || !m_win->document()) return;
    QTextCursor c(m_win->document()->story());
    c.setPosition(std::clamp(item->data(0, kPositionRole).toInt(), 0, write::editableEnd(m_win->document()->story())));
    m_win->view()->showCursor(c);
    m_win->view()->setFocus(Qt::OtherFocusReason);
}

bool NavigationPane::moveHeadingLevel(QTreeWidgetItem *item, int by)
{
    write::WriteDoc *doc = m_win->document();
    if (!item || !doc || m_win->isReadOnly()) return false;
    // The heading and those under it (to the next of the same or a higher level): found in the text, each by its block.
    QVector<int> blocks;
    int most = 0, least = 8;
    const int level = item->data(0, kLevelRole).toInt();
    for (QTextBlock b = doc->story()->findBlock(item->data(0, kPositionRole).toInt()); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (!bf.hasProperty(write::prop::Outline) || bf.intProperty(write::prop::Outline) >= 9) continue;
        const int l = bf.intProperty(write::prop::Outline);
        if (!blocks.isEmpty() && l <= level) break;
        blocks << b.position();
        most = std::max(most, l);
        least = std::min(least, l);
    }
    if (blocks.isEmpty() || least + by < 0 || most + by > 8) return false;
    const write::WriteDoc::StepGroup group(*doc);
    for (const int position : blocks) {
        QTextCursor c(doc->story());
        c.setPosition(position);
        const int l = c.block().blockFormat().intProperty(write::prop::Outline);
        write::applyParagraphStyle(*doc, c, QStringLiteral("Heading%1").arg(l + by + 1));
    }
    rebuildHeadings();
    return true;
}

void NavigationPane::rebuildPages()
{
    write::WriteDoc *doc = m_win->document();
    write::PageView *view = m_win->view();
    const int pages = std::min(view->pageCount(), kMostThumbnails);
    if (m_pagesOf == doc->id() && m_pageRevision == doc->story()->revision() && m_pages->count() == pages) return;
    m_pagesOf = doc->id();
    m_pageRevision = doc->story()->revision();
    m_pages->clear();
    for (int i = 0; i < pages; ++i) {
        auto *item = new QListWidgetItem(QIcon(QPixmap::fromImage(view->pageImage(i, kThumbnailWidth * 2))), QString::number(i + 1), m_pages);
        item->setData(kPositionRole, i);
        item->setTextAlignment(Qt::AlignHCenter);
    }
}

void NavigationPane::search()
{
    m_results->clear();
    write::WriteDoc *doc = m_win->document();
    const QString what = m_search->text().trimmed();
    if (!doc || what.isEmpty()) return;
    const QTextDocument *story = doc->story();
    const int end = write::editableEnd(story);
    QTextCursor found = story->find(what, 0);
    while (!found.isNull() && found.selectionEnd() <= end && m_results->count() < kMostResults) {
        const QTextBlock b = found.block();
        const int from = std::max(0, found.selectionStart() - b.position() - 24), to = std::min(b.length(), found.selectionEnd() - b.position() + 24);
        QString context = b.text().mid(from, to - from).simplified();
        if (from > 0) context.prepend(QChar(0x2026));
        if (to < b.length() - 1) context.append(QChar(0x2026));
        auto *item = new QListWidgetItem(context, m_results);
        item->setData(kPositionRole, found.selectionStart());
        item->setData(kLengthRole, found.selectionEnd() - found.selectionStart());
        found = story->find(what, found.selectionEnd());
    }
    if (m_results->count() == 0) m_results->addItem(tr("No results"));
}

} // namespace jp
