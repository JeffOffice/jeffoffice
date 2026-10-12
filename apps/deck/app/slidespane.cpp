#include "app/slidespane.h"

#include "app/editor.h"
#include "app/icons.h"
#include "app/theme.h"
#include "render/renderer.h"

#include <QContextMenuEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMenu>
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
// for the current slide an accent outline around it; no fill over either.
class SlideDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const override
    {
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
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &) const override
    {
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
        if (!m_updating && row >= 0 && row != m_ed->currentPage()) m_ed->setCurrentPage(row);
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
    if (count() != d->pages.size()) {
        rebuild();
        return;
    }
    m_updating = true;
    setCurrentRow(std::clamp(m_ed->currentPage(), 0, int(d->pages.size()) - 1));
    m_updating = false;
}

void SlidesPane::rebuild()
{
    const Document *d = m_ed->doc();
    if (!d) return;
    m_updating = true;
    const int n = int(d->pages.size());
    while (count() > n) delete takeItem(count() - 1);
    while (count() < n) addItem(new QListWidgetItem());
    PaintContext ctx;
    ctx.doc = d;
    ctx.cache = &m_ed->cache();
    const double scale = kThumbWidth / std::max(1.0, d->pageSize().width());
    // The thumbnails keep the slides' shape (4:3 or 16:9).
    setIconSize(QSize(kThumbWidth, std::max(1, int(std::lround(kThumbWidth * d->pageSize().height() / std::max(1.0, d->pageSize().width()))))));
    for (int i = 0; i < n; ++i) {
        QImage img = Renderer::renderToImage(ctx, i, scale * devicePixelRatioF());
        img.setDevicePixelRatio(devicePixelRatioF());
        const bool hidden = d->pages[i]->hidden;
        QListWidgetItem *it = item(i);
        it->setIcon(QIcon(QPixmap::fromImage(img)));
        it->setText(QString::number(i + 1));
        it->setData(HiddenRole, hidden);
        it->setData(Qt::AccessibleTextRole, hidden ? tr("Slide %1, hidden").arg(i + 1) : tr("Slide %1").arg(i + 1));
    }
    setCurrentRow(std::clamp(m_ed->currentPage(), 0, n - 1));
    m_updating = false;
}

QRect SlidesPane::thumbnailRect(int row) const
{
    return row >= 0 && row < count() ? thumbIn(visualItemRect(item(row)), iconSize()) : QRect();
}

void SlidesPane::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    menu.addAction(tr("New Slide"), this, &SlidesPane::newSlideWanted);
    menu.addAction(tr("Duplicate Slide"), this, &SlidesPane::duplicateWanted);
    menu.addAction(tr("Delete Slide"), this, &SlidesPane::deleteWanted);
    menu.addSeparator();
    menu.addAction(tr("Hide Slide"), this, &SlidesPane::hideWanted);
    menu.exec(e->globalPos());
}

void SlidesPane::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) Q_EMIT deleteWanted();
    else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) Q_EMIT newSlideWanted();
    else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Home || e->key() == Qt::Key_End) {
        // From slide to slide, whatever the list has laid out.
        const int last = count() - 1, row = std::max(0, currentRow());
        setCurrentRow(std::clamp(e->key() == Qt::Key_Up ? row - 1 : e->key() == Qt::Key_Down ? row + 1 : e->key() == Qt::Key_Home ? 0 : last, 0, std::max(0, last)));
    } else QListWidget::keyPressEvent(e);
}

void SlidesPane::dropEvent(QDropEvent *e)
{
    const int from = currentRow();
    QListWidget::dropEvent(e);
    const int to = currentRow();
    if (from >= 0 && to >= 0 && from != to) {
        m_ed->movePage(from, to);
        m_ed->setCurrentPage(to);
    }
    rebuild();
}

} // namespace jp
