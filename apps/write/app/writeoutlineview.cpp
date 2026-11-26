#include "app/writeoutlineview.h"

#include "app/stylearea.h"
#include "core/outlineops.h"
#include "core/styleinspect.h"
#include "core/writedoc.h"

#include <QApplication>
#include <QFrame>
#include <QHeaderView>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QKeyEvent>
#include <QStack>
#include <QTextBlock>
#include <algorithm>

namespace jp {

namespace {
constexpr int kPositionRole = Qt::UserRole, kLevelRole = Qt::UserRole + 1;

constexpr int kSymbolSize = 10;

// The small square at the left of a heading's words: the thing to hold to move the heading with all that is under it.
QIcon headingSymbol(const QColor &color)
{
    QPixmap pm(kSymbolSize * 2, kSymbolSize * 2);
    pm.setDevicePixelRatio(2);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRoundedRect(QRectF(1, 1, kSymbolSize - 2, kSymbolSize - 2), 1.5, 1.5);
    return QIcon(pm);
}

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
    setIconSize(QSize(kSymbolSize, kSymbolSize));
    connect(this, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem *item, int) {
        if (!m_building) Q_EMIT textTyped(positionOf(item), item->text(0));
    });
}

bool WriteOutlineView::event(QEvent *e)
{
    // (Tab is the outline's own key, not the one that moves on to the next control.)
    if (e->type() == QEvent::KeyPress && state() != EditingState) {
        const int key = static_cast<QKeyEvent *>(e)->key();
        if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
            Q_EMIT levelKeyPressed(key == Qt::Key_Tab ? 1 : -1);
            return true;
        }
    }
    return QTreeWidget::event(e);
}

void WriteOutlineView::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && state() != EditingState && !selectedItems().isEmpty()) {
        Q_EMIT newParagraphWanted();
        return;
    }
    QTreeWidget::keyPressEvent(e);
}

// Dragging a heading's symbol moves its section: a line shows where it will go, and letting go says so (the window moves it). It is done here and not with the tree's own drag and drop,
// which would move the rows and not the text.
void WriteOutlineView::mousePressEvent(QMouseEvent *e)
{
    endDrag();
    if (e->button() == Qt::LeftButton && m_doc && state() != EditingState) {
        const QModelIndex index = indexAt(e->pos());
        const QTreeWidgetItem *item = index.isValid() ? itemFromIndex(index) : nullptr;
        if (item && levelOf(item) < write::kBodyLevel) {
            const QRect r = visualRect(index);
            if (e->pos().x() >= r.left() && e->pos().x() < r.left() + kSymbolSize + 8) {
                m_dragPosition = positionOf(item);
                m_pressedAt = e->pos();
            }
        }
    }
    QTreeWidget::mousePressEvent(e);
}

void WriteOutlineView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragPosition < 0 || !(e->buttons() & Qt::LeftButton)) {
        QTreeWidget::mouseMoveEvent(e);
        return;
    }
    if (!m_dragging && (e->pos() - m_pressedAt).manhattanLength() < QApplication::startDragDistance()) return;
    m_dragging = true;
    viewport()->setCursor(Qt::ClosedHandCursor);
    int lineY = 0;
    m_dropBefore = dropBeforeAt(e->pos().y(), &lineY);
    if (!m_dropLine) {
        m_dropLine = new QFrame(viewport());
        m_dropLine->setObjectName(QStringLiteral("outlineDropLine"));
        m_dropLine->setAutoFillBackground(true);
        m_dropLine->setAttribute(Qt::WA_TransparentForMouseEvents);
        QPalette pal = m_dropLine->palette();
        pal.setColor(QPalette::Window, palette().color(QPalette::Highlight));
        m_dropLine->setPalette(pal);
    }
    m_dropLine->setGeometry(0, std::max(0, lineY - 1), viewport()->width(), 2);
    m_dropLine->show();
    m_dropLine->raise();
}

void WriteOutlineView::mouseReleaseEvent(QMouseEvent *e)
{
    if (!m_dragging) {
        endDrag();
        QTreeWidget::mouseReleaseEvent(e);
        return;
    }
    const int position = m_dragPosition, before = m_dropBefore;
    endDrag();
    Q_EMIT sectionDragged(position, before);
}

void WriteOutlineView::endDrag()
{
    if (m_dragging) viewport()->unsetCursor();
    m_dragging = false;
    m_dragPosition = -1;
    if (m_dropLine) m_dropLine->hide();
}

int WriteOutlineView::dropBeforeAt(int y, int *lineY) const
{
    const QTreeWidgetItem *last = nullptr;
    for (const QTreeWidgetItem *item = itemAt(1, 0); item; item = itemBelow(item)) {
        const QRect r = visualItemRect(item);
        if (y < r.center().y()) {
            *lineY = r.top();
            return positionOf(item);
        }
        if (y < r.bottom() + 1) {
            const QTreeWidgetItem *next = itemBelow(item);
            *lineY = r.bottom() + 1;
            return next ? positionOf(next) : -1;
        }
        last = item;
    }
    *lineY = last ? visualItemRect(last).bottom() + 1 : 0;
    return -1;
}

void WriteOutlineView::setDocument(write::WriteDoc *doc)
{
    m_doc = doc;
    rebuild();
}

void WriteOutlineView::updateGeometries()
{
    QTreeWidget::updateGeometries();
    if (m_styleMargin == 0 || m_marginBusy) return;
    m_marginBusy = true;
    isRightToLeft() ? setViewportMargins(0, 0, m_styleMargin, 0) : setViewportMargins(m_styleMargin, 0, 0, 0);
    m_marginBusy = false;
}

void WriteOutlineView::setStyleAreaWidth(int width)
{
    m_styleWidth = std::max(0, width);
    if (!m_styleArea && m_styleWidth == 0) return;
    if (!m_styleArea) {
        m_styleArea = new write::StyleArea(this, [this] { return styleRows(); }, [this](int w) {
            m_styleMargin = w;
            isRightToLeft() ? setViewportMargins(0, 0, w, 0) : setViewportMargins(w, 0, 0, 0);
        });
        connect(m_styleArea, &write::StyleArea::rowClicked, this, [this](int position) { selectParagraphAt(position); });
        connect(m_styleArea, &write::StyleArea::rowDoubleClicked, this, [this](int position) {
            selectParagraphAt(position);
            Q_EMIT styleNameActivated(position);
        });
        connect(m_styleArea, &write::StyleArea::widthDragged, this, [this](int width) {
            m_styleWidth = width;
            Q_EMIT styleAreaWidthDragged(width);
        });
        QPointer<write::StyleArea> area = m_styleArea;   // (the model resets from ~QTreeWidget, when this view is no longer a WriteOutlineView)
        auto later = [area] { if (area) area->scheduleRefresh(); };
        connect(this, &QTreeWidget::itemExpanded, m_styleArea, later);
        connect(this, &QTreeWidget::itemCollapsed, m_styleArea, later);
        connect(model(), &QAbstractItemModel::rowsInserted, m_styleArea, later);
        connect(model(), &QAbstractItemModel::rowsRemoved, m_styleArea, later);
        connect(model(), &QAbstractItemModel::modelReset, m_styleArea, later);
        connect(model(), &QAbstractItemModel::layoutChanged, m_styleArea, later);
        connect(model(), &QAbstractItemModel::dataChanged, m_styleArea, later);
    }
    m_styleArea->setAreaWidth(m_styleWidth);
}

// The rows on show: from the one at the top of the viewport down, as far as the viewport goes (the rows hidden by a collapsed heading are not walked).
QVector<write::StyleRow> WriteOutlineView::styleRows() const
{
    QVector<write::StyleRow> out;
    if (!m_doc) return out;
    QHash<QString, QString> names;
    const int bottom = viewport()->height();
    for (QTreeWidgetItem *item = itemAt(1, 0); item && out.size() < 2000; item = itemBelow(item)) {
        const QRect r = visualItemRect(item);
        if (r.top() >= bottom) break;
        const int position = positionOf(item);
        if (position < 0) continue;
        const QTextBlock b = m_doc->story()->findBlock(position);
        if (!b.isValid()) continue;
        write::StyleRow row;
        row.top = r.top();
        row.height = r.height();
        row.position = position;
        row.name = write::paragraphStyleNameOf(*m_doc, b, &names);
        out << row;
    }
    return out;
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
        if (r.heading()) item->setIcon(0, headingSymbol(palette().color(QPalette::Mid)));
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
