#include "app/slidespane.h"

#include "app/editor.h"
#include "app/icons.h"
#include "app/sectionui.h"
#include "app/theme.h"
#include "core/sections.h"
#include "render/renderer.h"

#include <QContextMenuEvent>
#include <QDropEvent>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyledItemDelegate>

namespace jp {

namespace {
constexpr int kThumbWidth = 168;
constexpr int kNumberWidth = 32;   // the slide's number, at the left of its thumbnail

QRect thumbIn(const QRect &row, const QSize &icon)
{
    return QRect(row.left() + kNumberWidth, row.top() + (row.height() - icon.height()) / 2, icon.width(), icon.height());
}

// A row: the slide's number, then its thumbnail with a thin gray border, or
// for the current slide an accent outline around it; no fill over either. A
// section's heading is a row of its own.
class SlideDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const override
    {
        if (idx.data(SlidesPane::SlideRole).toInt() < 0) {
            p->save();
            p->setFont(opt.font);
            paintSectionHeader(p, opt.rect, idx.data(Qt::DisplayRole).toString(), idx.data(SlidesPane::CountRole).toInt(), idx.data(SlidesPane::CollapsedRole).toBool(),
                               opt.state & QStyle::State_MouseOver);
            p->restore();
            return;
        }
        p->save();
        const bool current = opt.state & QStyle::State_Selected, hover = opt.state & QStyle::State_MouseOver;
        const bool hidden = idx.data(SlidesPane::HiddenRole).toBool();
        const QRect thumb = thumbIn(opt.rect, opt.decorationSize);
        p->setOpacity(hidden ? 0.4 : 1.0);
        idx.data(Qt::DecorationRole).value<QIcon>().paint(p, thumb, Qt::AlignCenter, QIcon::Normal);
        p->setOpacity(1.0);
        p->setBrush(Qt::NoBrush);
        const QColor edge = uiDark() ? QColor(hover ? 0x78 : 0x4A, hover ? 0x80 : 0x52, hover ? 0x8C : 0x5E) : QColor(hover ? 0x9A : 0xC4, hover ? 0xA0 : 0xC8, hover ? 0xAA : 0xD0);
        p->setPen(QPen(edge, 1));
        p->drawRect(QRectF(thumb).adjusted(0.5, 0.5, -0.5, -0.5));
        if (current) {
            p->setPen(QPen(uiAccent(), 3));
            p->drawRect(QRectF(thumb).adjusted(-1.5, -1.5, 1.5, 1.5));
        }
        QFont f = opt.font;
        f.setStrikeOut(hidden);
        if (current) f.setWeight(QFont::DemiBold);
        p->setFont(f);
        QColor text = uiText();
        if (!current) text.setAlphaF(0.65f);
        p->setPen(text);
        p->drawText(QRect(opt.rect.left(), thumb.top(), kNumberWidth - 8, QFontMetrics(f).height() + 2), Qt::AlignRight | Qt::AlignTop,
                    idx.data(Qt::DisplayRole).toString());
        p->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const override
    {
        if (idx.data(SlidesPane::SlideRole).toInt() < 0) return QSize(kNumberWidth + opt.decorationSize.width() + 16, kSectionHeaderHeight);
        return QSize(kNumberWidth + opt.decorationSize.width() + 16, opt.decorationSize.height() + 14);
    }
};
}

SlidesPane::SlidesPane(Editor *ed, QWidget *parent) : QListWidget(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("slidesPane"));
    setAccessibleName(tr("Slides"));
    setViewMode(QListView::ListMode);
    setIconSize(QSize(kThumbWidth, kThumbWidth * 9 / 16));
    setSpacing(2);
    setFrameShape(QFrame::NoFrame);
    setItemDelegate(new SlideDelegate(this));
    setMouseTracking(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto restyle = [this] {
        setStyleSheet(QStringLiteral("QListWidget{background:%1; border:none; border-right:1px solid %2; outline:0; padding-top:6px;}").arg(uiPanel().name(), uiLine().name()));
    };
    restyle();
    connect(UiTheme::instance(), &UiTheme::changed, this, restyle);   // the look switched in Options
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setMinimumWidth(kNumberWidth + kThumbWidth + 28);
    m_timer.setSingleShot(true);
    m_timer.setInterval(120);
    connect(&m_timer, &QTimer::timeout, this, &SlidesPane::rebuild);
    connect(this, &QListWidget::currentRowChanged, this, [this](int row) {
        const int slide = slideOfRow(row);
        if (!m_updating && slide >= 0 && slide != m_ed->currentPage()) m_ed->setCurrentPage(slide);
    });
    rebuild();
}

void SlidesPane::refresh(bool soon)
{
    if (soon) m_timer.start();
    else rebuild();
}

void SlidesPane::followCurrent()
{
    const Document *d = m_ed->doc();
    if (!d) return;
    if (count() != d->pages.size() + (deck::hasSections(*d) ? deck::sectionsOf(*d).size() : 0) || m_shownSections != d->appData.value(QStringLiteral("sections")).toArray()) {
        rebuild();
        return;
    }
    const int slide = std::clamp(m_ed->currentPage(), 0, int(d->pages.size()) - 1);
    if (slideHidden(slide)) {
        setCollapsed(deck::sectionOfSlide(*d, slide), false);
        return;
    }
    m_updating = true;
    setCurrentRow(rowOfSlide(slide));
    m_updating = false;
}

bool SlidesPane::slideHidden(int slide) const
{
    const Document *d = m_ed->doc();
    if (!d || m_collapsed.isEmpty()) return false;
    const int section = deck::sectionOfSlide(*d, slide);
    return section >= 0 && isCollapsed(section);
}

void SlidesPane::rebuild()
{
    const Document *d = m_ed->doc();
    if (!d) return;
    m_updating = true;
    const int n = int(d->pages.size());
    const QVector<deck::Section> sections = deck::sectionsOf(*d);
    m_shownSections = d->appData.value(QStringLiteral("sections")).toArray();
    const int rows = n + int(sections.size());
    while (count() > rows) delete takeItem(count() - 1);
    while (count() < rows) addItem(new QListWidgetItem());
    m_slideOfRow.fill(-1, rows);
    m_rowOfSlide.fill(-1, n);
    m_headerRows.clear();
    PaintContext ctx;
    ctx.doc = d;
    ctx.cache = &m_ed->cache();
    const double scale = kThumbWidth / std::max(1.0, d->pageSize().width());
    // The thumbnails keep the slides' shape (4:3 or 16:9).
    setIconSize(QSize(kThumbWidth, std::max(1, int(std::lround(kThumbWidth * d->pageSize().height() / std::max(1.0, d->pageSize().width()))))));
    int row = 0, slide = 0;
    auto showSlide = [&](bool folded) {
        QListWidgetItem *it = item(row);
        it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
        const bool hidden = d->pages[slide]->hidden;
        // A slide in a section folded away is drawn when the section opens.
        if (folded) {
            it->setIcon(QIcon());
        } else {
            QImage img = Renderer::renderToImage(ctx, slide, scale * devicePixelRatioF());
            img.setDevicePixelRatio(devicePixelRatioF());
            it->setIcon(QIcon(QPixmap::fromImage(img)));
        }
        it->setText(QString::number(slide + 1));
        it->setData(HiddenRole, hidden);
        it->setData(SlideRole, slide);
        it->setData(SectionRole, QVariant());
        it->setData(CountRole, QVariant());
        it->setData(CollapsedRole, QVariant());
        it->setData(Qt::AccessibleTextRole, hidden ? tr("Slide %1, hidden").arg(slide + 1) : tr("Slide %1").arg(slide + 1));
        setRowHidden(row, folded);
        m_slideOfRow[row] = slide;
        m_rowOfSlide[slide] = row;
        ++row;
        ++slide;
    };
    if (sections.isEmpty()) {
        while (slide < n) showSlide(false);
    } else {
        for (int s = 0; s < sections.size(); ++s) {
            const bool folded = m_collapsed.contains(sections[s].id);
            QListWidgetItem *it = item(row);
            it->setFlags(Qt::ItemIsEnabled);
            it->setIcon(QIcon());
            it->setText(sections[s].name);
            it->setData(HiddenRole, false);
            it->setData(SlideRole, -1);
            it->setData(SectionRole, s);
            it->setData(CountRole, sections[s].count);
            it->setData(CollapsedRole, folded);
            it->setData(Qt::AccessibleTextRole, tr("Section %1, %2 slides, %3").arg(sections[s].name).arg(sections[s].count).arg(folded ? tr("collapsed") : tr("expanded")));
            setRowHidden(row, false);
            m_headerRows << row;
            ++row;
            for (int k = 0; k < sections[s].count; ++k) showSlide(folded);
        }
    }
    setCurrentRow(rowOfSlide(std::clamp(m_ed->currentPage(), 0, n - 1)));
    m_updating = false;
}

bool SlidesPane::isHeaderRow(int row) const { return row >= 0 && row < m_slideOfRow.size() && m_slideOfRow[row] < 0; }
int SlidesPane::slideOfRow(int row) const { return row >= 0 && row < m_slideOfRow.size() ? m_slideOfRow[row] : -1; }
int SlidesPane::rowOfSlide(int slide) const { return slide >= 0 && slide < m_rowOfSlide.size() ? m_rowOfSlide[slide] : -1; }
int SlidesPane::headerRow(int section) const { return section >= 0 && section < m_headerRows.size() ? m_headerRows[section] : -1; }

QRect SlidesPane::headerRect(int section) const
{
    const int row = headerRow(section);
    return row >= 0 ? visualItemRect(item(row)) : QRect();
}

QRect SlidesPane::thumbnailRect(int slide) const
{
    const int row = rowOfSlide(slide);
    return row >= 0 && !isRowHidden(row) ? thumbIn(visualItemRect(item(row)), iconSize()) : QRect();
}

bool SlidesPane::isCollapsed(int section) const
{
    const QVector<deck::Section> sections = m_ed->doc() ? deck::sectionsOf(*m_ed->doc()) : QVector<deck::Section>();
    return section >= 0 && section < sections.size() && m_collapsed.contains(sections[section].id);
}

void SlidesPane::setCollapsed(int section, bool collapsed)
{
    const QVector<deck::Section> sections = m_ed->doc() ? deck::sectionsOf(*m_ed->doc()) : QVector<deck::Section>();
    if (section < 0 || section >= sections.size() || isCollapsed(section) == collapsed) return;
    if (collapsed) m_collapsed.insert(sections[section].id);
    else m_collapsed.remove(sections[section].id);
    rebuild();
    Q_EMIT collapsedChanged();
}

void SlidesPane::setAllCollapsed(bool collapsed)
{
    const QVector<deck::Section> sections = m_ed->doc() ? deck::sectionsOf(*m_ed->doc()) : QVector<deck::Section>();
    QSet<QString> now;
    if (collapsed)
        for (const deck::Section &s : sections) now.insert(s.id);
    if (now == m_collapsed) return;
    m_collapsed = now;
    rebuild();
    Q_EMIT collapsedChanged();
}

void SlidesPane::setCollapsedSections(const QSet<QString> &ids)
{
    if (ids == m_collapsed) return;
    m_collapsed = ids;
    rebuild();
}

void SlidesPane::moveSlide(int from, int to, int into)
{
    Document *d = m_ed->doc();
    const int n = int(d->pages.size());
    if (from < 0 || from >= n || to < 0 || to >= n) return;
    const QList<int> order = deck::orderAfterMoving(n, {from}, to > from ? to + 1 : to);
    if (from == to && (into < 0 || into == deck::sectionOfSlide(*d, from))) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Move Slide"));
    deck::slidesReordered(*d, order, {from}, into);
    if (from != to) m_ed->movePage(from, to);
    m_ed->setCurrentPage(to);
    m_ed->endChange();
}

void SlidesPane::contextMenuEvent(QContextMenuEvent *e)
{
    const QModelIndex at = indexAt(e->pos());
    QMenu menu(this);
    if (at.isValid() && at.data(SlideRole).toInt() < 0) {
        const int section = at.data(SectionRole).toInt();
        SectionMenu commands;
        commands.rename = [this, section] { Q_EMIT renameSectionWanted(section); };
        commands.remove = [this, section] { Q_EMIT removeSectionWanted(section); };
        commands.removeWithSlides = [this, section] { Q_EMIT removeSectionAndSlidesWanted(section); };
        commands.moveUp = [this, section] { Q_EMIT moveSectionWanted(section, -1); };
        commands.moveDown = [this, section] { Q_EMIT moveSectionWanted(section, 1); };
        commands.collapseAll = [this] { setAllCollapsed(true); };
        commands.expandAll = [this] { setAllCollapsed(false); };
        commands.canMoveUp = section > 0;
        commands.canMoveDown = section < m_headerRows.size() - 1;
        fillSectionMenu(menu, commands);
        menu.exec(e->globalPos());
        return;
    }
    const int slide = at.isValid() ? at.data(SlideRole).toInt() : m_ed->currentPage();
    menu.addAction(tr("New Slide"), this, &SlidesPane::newSlideWanted);
    menu.addAction(tr("Duplicate Slide"), this, &SlidesPane::duplicateWanted);
    menu.addAction(tr("Delete Slide"), this, &SlidesPane::deleteWanted);
    menu.addSeparator();
    menu.addAction(tr("Hide Slide"), this, &SlidesPane::hideWanted);
    menu.addSeparator();
    menu.addAction(tr("Add Section"), this, [this, slide] { Q_EMIT addSectionWanted(slide); });
    menu.exec(e->globalPos());
}

void SlidesPane::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) Q_EMIT deleteWanted();
    else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) Q_EMIT newSlideWanted();
    else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Home || e->key() == Qt::Key_End) {
        // From slide to slide, whatever the list has laid out (the slides of a section folded away are passed).
        const int last = int(m_rowOfSlide.size()) - 1, slide = std::clamp(slideOfRow(currentRow()), 0, std::max(0, last));
        const int step = e->key() == Qt::Key_Up || e->key() == Qt::Key_Home ? -1 : 1;
        int target = e->key() == Qt::Key_Home ? 0 : e->key() == Qt::Key_End ? last : slide + step;
        while (target >= 0 && target <= last && slideHidden(target)) target += step;
        if (target >= 0 && target <= last) setCurrentRow(rowOfSlide(target));
    } else QListWidget::keyPressEvent(e);
}

// A heading takes the mouse itself: it is no slide, so it is not selected, and a click on it folds or opens its section.
void SlidesPane::mousePressEvent(QMouseEvent *e)
{
    const QModelIndex at = indexAt(e->pos());
    if (at.isValid() && at.data(SlideRole).toInt() < 0) {
        m_pressedHeader = e->button() == Qt::LeftButton ? at.data(SectionRole).toInt() : -1;
        e->accept();
        return;
    }
    m_pressedHeader = -1;
    QListWidget::mousePressEvent(e);
}

void SlidesPane::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressedHeader >= 0) {
        const QModelIndex at = indexAt(e->pos());
        const int section = m_pressedHeader;
        m_pressedHeader = -1;
        if (e->button() == Qt::LeftButton && at.isValid() && at.data(SlideRole).toInt() < 0 && at.data(SectionRole).toInt() == section) setCollapsed(section, !isCollapsed(section));
        e->accept();
        return;
    }
    QListWidget::mouseReleaseEvent(e);
}

void SlidesPane::mouseDoubleClickEvent(QMouseEvent *e)
{
    const QModelIndex at = indexAt(e->pos());
    if (at.isValid() && at.data(SlideRole).toInt() < 0) {
        e->accept();
        return;
    }
    QListWidget::mouseDoubleClickEvent(e);
}

void SlidesPane::dropEvent(QDropEvent *e)
{
    const int from = slideOfRow(currentRow());
    QListWidget::dropEvent(e);
    // Where the list put the row: its place among the slides, under the heading above it.
    const int row = currentRow();
    int to = 0, into = m_headerRows.isEmpty() ? -1 : 0;
    for (int r = 0; r < row; ++r) {
        if (item(r)->data(SlideRole).toInt() >= 0) ++to;
        else into = item(r)->data(SectionRole).toInt();
    }
    if (from >= 0 && row >= 0) moveSlide(from, to, into);
    rebuild();
}

} // namespace jp
