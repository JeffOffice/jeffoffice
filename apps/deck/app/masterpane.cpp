#include "app/masterpane.h"

#include "app/deckwidgets.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/theme.h"
#include "core/deck.h"
#include "text/storyio.h"

#include <QAction>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {
constexpr int kThumbWidth = 132;
constexpr int kIndent = 22;   // a layout stands this far in
constexpr int kPad = 6;
constexpr int kMaxThumbnails = 400;   // pages past these show their name only

// What a page looks like, as a hash: its background, whether it shows the master under it, its objects, and their text.
QByteArray lookOf(const Document &d, const MasterPage &page)
{
    QCryptographicHash h(QCryptographicHash::Sha1);
    auto add = [&h](const QJsonObject &o) { h.addData(QJsonDocument(o).toJson(QJsonDocument::Compact)); };
    add(page.background.toJson());
    h.addData(page.showMasterItems ? QByteArrayView("m1") : QByteArrayView("m0"));
    walkItems(page.items, [&](const ItemPtr &it) {
        add(it->toJson());
        QStringList stories;
        if (it->type() == ItemType::Text) stories << static_cast<const TextItem &>(*it).storyId;
        else if (it->type() == ItemType::Shape) stories << static_cast<const ShapeItem &>(*it).storyId;
        for (const QString &id : stories)
            if (const QTextDocument *story = id.isEmpty() ? nullptr : d.storyDoc(id)) add(storyToJson(story));
    });
    h.addData(d.pageSize().width() > 0 ? QByteArray::number(d.pageSize().width()) + "x" + QByteArray::number(d.pageSize().height()) : QByteArray());
    return h.result();
}

// A row: the page's thumbnail with a thin gray border, or for the page being edited an accent outline around it, and its name beside it.
class PageDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const override
    {
        p->save();
        const bool current = opt.state & QStyle::State_Selected, hover = opt.state & QStyle::State_MouseOver;
        const int depth = idx.data(MasterPane::DepthRole).toInt();
        const QRect thumb(opt.rect.left() + kPad + depth * kIndent, opt.rect.top() + (opt.rect.height() - opt.decorationSize.height()) / 2, opt.decorationSize.width(),
                          opt.decorationSize.height());
        idx.data(Qt::DecorationRole).value<QIcon>().paint(p, thumb, Qt::AlignCenter, QIcon::Normal);
        p->setBrush(Qt::NoBrush);
        const QColor edge = uiDark() ? QColor(hover ? 0x78 : 0x4A, hover ? 0x80 : 0x52, hover ? 0x8C : 0x5E) : QColor(hover ? 0x9A : 0xC4, hover ? 0xA0 : 0xC8, hover ? 0xAA : 0xD0);
        p->setPen(QPen(edge, 1));
        p->drawRect(QRectF(thumb).adjusted(0.5, 0.5, -0.5, -0.5));
        if (current) {
            p->setPen(QPen(uiAccent(), 3));
            p->drawRect(QRectF(thumb).adjusted(-1.5, -1.5, 1.5, 1.5));
        }
        QFont f = opt.font;
        f.setWeight(depth == 0 || current ? QFont::DemiBold : QFont::Normal);
        p->setFont(f);
        p->setPen(uiText());
        const QRect text(thumb.right() + 10, opt.rect.top(), opt.rect.right() - thumb.right() - 14, opt.rect.height());
        p->drawText(text, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, QFontMetrics(f).elidedText(idx.data(Qt::DisplayRole).toString(), Qt::ElideRight, text.width() * 2));
        p->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const override
    {
        return QSize(kPad * 2 + idx.data(MasterPane::DepthRole).toInt() * kIndent + opt.decorationSize.width() + 80, opt.decorationSize.height() + 14);
    }
};
} // namespace

MasterPane::MasterPane(Editor *ed, QWidget *parent) : QListWidget(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("masterPane"));
    setAccessibleName(tr("Slide masters and layouts"));
    setAccessibleDescription(tr("The slide masters, each with its layouts under it. The Up and Down arrows move between them."));
    setViewMode(QListView::ListMode);
    setIconSize(QSize(kThumbWidth, kThumbWidth * 9 / 16));
    setSpacing(2);
    setFrameShape(QFrame::NoFrame);
    setItemDelegate(new PageDelegate(this));
    setMouseTracking(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QAbstractItemView::SingleSelection);
    auto restyle = [this] {
        setStyleSheet(QStringLiteral("QListWidget{background:%1; border:none; border-right:1px solid %2; outline:0; padding-top:6px;}").arg(uiPanel().name(), uiLine().name()));
    };
    restyle();
    connect(UiTheme::instance(), &UiTheme::changed, this, restyle);
    setMinimumWidth(kThumbWidth + kIndent + 120);
    m_timer.setSingleShot(true);
    m_timer.setInterval(120);
    connect(&m_timer, &QTimer::timeout, this, &MasterPane::rebuild);
    connect(this, &QListWidget::currentRowChanged, this, [this](int row) {
        const QString id = row >= 0 ? item(row)->data(IdRole).toString() : QString();
        if (!m_updating && !id.isEmpty() && id != m_ed->masterView()) m_ed->setMasterView(id);
    });
    rebuild();
}

void MasterPane::refresh(bool soon)
{
    if (soon) m_timer.start();
    else rebuild();
}

void MasterPane::rebuild()
{
    const Document *d = m_ed->doc();
    if (!d) return;
    m_timer.stop();
    m_updating = true;
    // Each slide master with its layouts; then the layouts that name no slide master, as the first's.
    QVector<const MasterPage *> order;
    QSet<QString> masters;
    for (const auto &m : d->masters)
        if (deck::isSlideMaster(*m)) masters.insert(m->id);
    for (const auto &m : d->masters) {
        if (!deck::isSlideMaster(*m)) continue;
        order << m.get();
        for (const auto &l : d->masters)
            if (deck::isLayout(*l) && l->baseId == m->id) order << l.get();
    }
    for (const auto &l : d->masters)
        if (!deck::isSlideMaster(*l) && !(deck::isLayout(*l) && masters.contains(l->baseId))) order << l.get();
    while (count() > order.size()) delete takeItem(count() - 1);
    while (count() < order.size()) addItem(new QListWidgetItem());
    setIconSize(QSize(kThumbWidth, std::max(1, int(std::lround(kThumbWidth * d->pageSize().height() / std::max(1.0, d->pageSize().width()))))));
    const qreal dpr = devicePixelRatioF();
    QHash<QString, QByteArray> seen;
    for (int row = 0; row < order.size(); ++row) {
        const MasterPage &page = *order[row];
        QListWidgetItem *it = item(row);
        it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        const QByteArray look = lookOf(*d, page) + QByteArray::number(row < kMaxThumbnails) + QByteArray::number(dpr);
        if (it->data(IdRole).toString() != page.id || m_seen.value(page.id) != look) {
            if (row < kMaxThumbnails) {
                QImage img = layoutPicture(*d, page, int(std::lround(kThumbWidth * dpr)));
                img.setDevicePixelRatio(dpr);
                it->setIcon(QIcon(QPixmap::fromImage(img)));
            } else {
                it->setIcon(QIcon());
            }
        }
        seen.insert(page.id, look);
        const int depth = deck::isSlideMaster(page) ? 0 : 1;
        it->setText(page.name);
        it->setData(IdRole, page.id);
        it->setData(DepthRole, depth);
        it->setData(Qt::AccessibleTextRole, depth == 0 ? tr("Slide master %1").arg(page.name) : tr("Layout %1").arg(page.name));
    }
    m_seen = seen;
    m_updating = false;
    followCurrent();
}

void MasterPane::followCurrent()
{
    const int row = rowOf(m_ed->masterView());
    if (row == currentRow()) return;
    m_updating = true;
    setCurrentRow(row);
    m_updating = false;
}

int MasterPane::rowOf(const QString &id) const
{
    for (int row = 0; row < count(); ++row)
        if (item(row)->data(IdRole).toString() == id) return row;
    return -1;
}

QStringList MasterPane::ids() const
{
    QStringList out;
    for (int row = 0; row < count(); ++row) out << item(row)->data(IdRole).toString();
    return out;
}

int MasterPane::depthOf(const QString &id) const
{
    const int row = rowOf(id);
    return row < 0 ? -1 : item(row)->data(DepthRole).toInt();
}

QString MasterPane::nameOf(const QString &id) const
{
    const int row = rowOf(id);
    return row < 0 ? QString() : item(row)->text();
}

QString MasterPane::currentId() const
{
    const int row = currentRow();
    return row < 0 ? QString() : item(row)->data(IdRole).toString();
}

void MasterPane::selectPage(const QString &id)
{
    if (const int row = rowOf(id); row >= 0) setCurrentRow(row);
}

QRect MasterPane::thumbnailRect(const QString &id) const
{
    const int row = rowOf(id);
    if (row < 0) return QRect();
    const QRect r = visualItemRect(item(row));
    return QRect(r.left() + kPad + item(row)->data(DepthRole).toInt() * kIndent, r.top() + (r.height() - iconSize().height()) / 2, iconSize().width(), iconSize().height());
}

void MasterPane::setMenuActions(const QList<QAction *> &actions)
{
    for (QAction *a : actions) {
        if (!a) {
            auto *sep = new QAction(this);
            sep->setSeparator(true);
            addAction(sep);
        } else {
            addAction(a);
        }
    }
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

} // namespace jp
