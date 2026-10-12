#include "app/slidespane.h"

#include "app/editor.h"
#include "render/renderer.h"

#include <QContextMenuEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>

namespace jp {

namespace {
constexpr int kThumbWidth = 168;
}

SlidesPane::SlidesPane(Editor *ed, QWidget *parent) : QListWidget(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("slidesPane"));
    setAccessibleName(tr("Slides"));
    setViewMode(QListView::ListMode);
    setIconSize(QSize(kThumbWidth, kThumbWidth * 9 / 16));
    setSpacing(4);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setMinimumWidth(kThumbWidth + 60);
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
    for (int i = 0; i < n; ++i) {
        QImage img = Renderer::renderToImage(ctx, i, scale * devicePixelRatioF());
        img.setDevicePixelRatio(devicePixelRatioF());
        const bool hidden = d->pages[i]->hidden;
        if (hidden) {
            QPainter p(&img);
            p.fillRect(QRectF(QPointF(0, 0), img.deviceIndependentSize()), QColor(255, 255, 255, 140));
        }
        QListWidgetItem *it = item(i);
        it->setIcon(QIcon(QPixmap::fromImage(img)));
        it->setText(hidden ? tr("%1 (hidden)").arg(i + 1) : QString::number(i + 1));
        it->setData(Qt::AccessibleTextRole, hidden ? tr("Slide %1, hidden").arg(i + 1) : tr("Slide %1").arg(i + 1));
    }
    setCurrentRow(std::clamp(m_ed->currentPage(), 0, n - 1));
    m_updating = false;
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
    else QListWidget::keyPressEvent(e);
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
