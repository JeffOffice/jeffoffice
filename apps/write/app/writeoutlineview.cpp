#include "app/writeoutlineview.h"

#include "core/outlineops.h"
#include "core/writedoc.h"

#include <QHeaderView>
#include <QStack>

namespace jp {

namespace {
constexpr int kPositionRole = Qt::UserRole, kLevelRole = Qt::UserRole + 1;

QString keyOf(int level, const QString &text) { return QString::number(level) + QLatin1Char(':') + text; }
} // namespace

WriteOutlineView::WriteOutlineView(QWidget *parent) : QTreeWidget(parent)
{
    setObjectName(QStringLiteral("outlineView"));
    setAccessibleName(tr("Outline"));
    setAccessibleDescription(tr("The headings and text of the document. Use the Outlining tab to promote, demote, and move them."));
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setEditTriggers(QAbstractItemView::EditKeyPressed);   // (F2: a click selects, and a double click opens the text on the pages)
    setUniformRowHeights(false);
    setWordWrap(true);
    setIndentation(22);
    setRootIsDecorated(true);
    setFrameShape(QFrame::NoFrame);
    header()->setStretchLastSection(true);
    connect(this, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem *item, int) {
        if (!m_building) Q_EMIT textTyped(positionOf(item), item->text(0));
    });
}

void WriteOutlineView::setDocument(write::WriteDoc *doc)
{
    m_doc = doc;
    rebuild();
}

int WriteOutlineView::positionOf(const QTreeWidgetItem *item) { return item ? item->data(0, kPositionRole).toInt() : -1; }
int WriteOutlineView::levelOf(const QTreeWidgetItem *item) { return item ? item->data(0, kLevelRole).toInt() : write::kBodyLevel; }

void WriteOutlineView::applyLook(QTreeWidgetItem *item)
{
    QFont f = font();
    const int level = levelOf(item);
    if (m_formatting && level < write::kBodyLevel) {
        f.setBold(true);
        f.setPointSizeF(f.pointSizeF() * (level == 0 ? 1.35 : level == 1 ? 1.2 : level == 2 ? 1.1 : 1.0));
    }
    item->setFont(0, f);
}

void WriteOutlineView::rebuild()
{
    // What was collapsed, and what was selected, is found again after the rows are made.
    QSet<int> selectedPlaces;
    for (const QTreeWidgetItem *it : selectedItems()) selectedPlaces.insert(positionOf(it));
    {
        QTreeWidgetItemIterator at(this);
        while (*at) {
            if (!(*at)->isExpanded() && (*at)->childCount() > 0 && levelOf(*at) < write::kBodyLevel) m_collapsed.insert(keyOf(levelOf(*at), (*at)->text(0)));
            else if ((*at)->isExpanded()) m_collapsed.remove(keyOf(levelOf(*at), (*at)->text(0)));
            ++at;
        }
    }
    m_building = true;
    clear();
    if (!m_doc) {
        m_building = false;
        return;
    }
    const QVector<write::OutlineRow> rows = write::outlineRows(*m_doc);
    QStack<QTreeWidgetItem *> open;   // the headings above the next row
    for (const write::OutlineRow &r : rows) {
        auto *item = new QTreeWidgetItem;
        item->setText(0, r.text);
        item->setData(0, kPositionRole, r.position);
        item->setData(0, kLevelRole, r.level);
        if (r.editable) item->setFlags(item->flags() | Qt::ItemIsEditable);
        if (r.heading())
            while (!open.isEmpty() && levelOf(open.top()) >= r.level) open.pop();
        if (open.isEmpty()) addTopLevelItem(item);
        else open.top()->addChild(item);
        if (r.heading()) open.push(item);
        applyLook(item);
    }
    m_building = false;
    expandAll();
    for (QTreeWidgetItemIterator at(this); *at; ++at)
        if (m_collapsed.contains(keyOf(levelOf(*at), (*at)->text(0)))) (*at)->setExpanded(false);
    applyLevel();
    for (QTreeWidgetItemIterator at(this); *at; ++at)
        if (selectedPlaces.contains(positionOf(*at))) (*at)->setSelected(true);
}

void WriteOutlineView::applyLevel()
{
    for (QTreeWidgetItemIterator at(this); *at; ++at) {
        QTreeWidgetItem *item = *at;
        const int level = levelOf(item);
        if (level == write::kBodyLevel) {
            item->setHidden(m_level != 0);
        } else if (m_level != 0) {
            item->setHidden(level >= m_level);
            item->setExpanded(level + 1 < m_level);
        } else {
            item->setHidden(false);
        }
    }
}

void WriteOutlineView::setShowLevel(int level)
{
    m_level = qBound(0, level, 9);
    if (m_level == 0) {
        m_collapsed.clear();
        expandAll();
    }
    applyLevel();
}

void WriteOutlineView::setShowFormatting(bool on)
{
    m_formatting = on;
    for (QTreeWidgetItemIterator at(this); *at; ++at) applyLook(*at);
}

void WriteOutlineView::setFirstLineOnly(bool on)
{
    m_firstLine = on;
    setWordWrap(!on);
    setTextElideMode(Qt::ElideRight);
    doItemsLayout();
}

QPair<int, int> WriteOutlineView::selectedRange() const
{
    int first = -1, last = -1;
    for (const QTreeWidgetItem *it : selectedItems()) {
        const int p = positionOf(it);
        if (first < 0 || p < first) first = p;
        if (p > last) last = p;
    }
    return {first, last};
}

void WriteOutlineView::selectParagraphAt(int position)
{
    QTreeWidgetItem *best = nullptr;
    for (QTreeWidgetItemIterator at(this); *at; ++at)
        if (positionOf(*at) <= position) best = *at;
    if (!best) return;
    clearSelection();
    for (QTreeWidgetItem *p = best->parent(); p; p = p->parent()) p->setExpanded(true);
    best->setHidden(false);
    best->setSelected(true);
    setCurrentItem(best);
    scrollToItem(best);
}

void WriteOutlineView::expandSelected(bool expand)
{
    const QList<QTreeWidgetItem *> chosen = selectedItems();
    if (chosen.isEmpty()) {
        if (expand) expandAll();
        else collapseAll();
        return;
    }
    for (QTreeWidgetItem *it : chosen) {
        if (it->childCount() == 0) continue;
        it->setExpanded(expand);
        if (expand)
            for (int i = 0; i < it->childCount(); ++i) it->child(i)->setHidden(false);
    }
}

QStringList WriteOutlineView::shownRows() const
{
    QStringList out;
    for (QTreeWidgetItemIterator at(const_cast<WriteOutlineView *>(this)); *at; ++at) {
        bool shown = !(*at)->isHidden();
        for (const QTreeWidgetItem *p = (*at)->parent(); p && shown; p = p->parent()) shown = p->isExpanded() && !p->isHidden();
        if (shown) out << (*at)->text(0);
    }
    return out;
}

} // namespace jp
